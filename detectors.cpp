/* Arduino Library Includes */

#include "TaskAction.h"

/* Application Includes */

#include "application.h"
#include "detectors.h"
#include "settings.h"

/* Defines, typedefs, constants */

static const uint8_t NUMBER_OF_DETECTORS = 5;

static const char NO_DETECT = -1;

static const uint8_t DETECTOR_PINS[NUMBER_OF_DETECTORS] = {
	A0,A1,A2,A3,A4
};

static const uint8_t DETECTOR_POLARITY_PIN = A5;

static const int DEBOUNCE_COUNT = 5;

static char s_sequence[SEQUENCE_LENGTH];

/* Private Variables */

static DETECTOR s_detectors[NUMBER_OF_DETECTORS];
static int s_tripped_count = 0;
static bool * sp_detector_update_flag;
static uint8_t s_detector_trip_condition = LOW;

/* Private Functions */

static char detector_in_sequence(char detector_id)
{
	for (uint8_t i=0; i<SEQUENCE_LENGTH; i++)
	{
		if (detector_id == UNLOCK_SEQUENCE[i])
		{
			return true;
		}
	}
	return false;
}

static char get_tripped(DETECTOR * s_detectors)
{
	uint8_t i;
	for (i = 0; i < NUMBER_OF_DETECTORS; i++)
	{
		if (s_detectors[i].tripped)
		{
			return i;
		}
	}
	return NO_DETECT;
}

static void log_tripped(char pressed, char * log)
{
	for (uint8_t i=0; i < (SEQUENCE_LENGTH-1); i++)
	{
		log[i] = log[i+1];	
	}
	log[SEQUENCE_LENGTH-1] = pressed;
}

static void debounce_detector(DETECTOR& detector, bool state)
{
	detector.just_tripped = false;
	detector.just_untripped = false;

	if (state)
	{
		detector.debounce = min(detector.debounce+1, DEBOUNCE_COUNT);
	}
	else
	{
		detector.debounce = max(detector.debounce-1, 0);	
	}

	if (detector.debounce == DEBOUNCE_COUNT)
	{
		detector.just_tripped = !detector.tripped;
		detector.tripped = true;
	}
	else if (detector.debounce == 0)
	{
		detector.just_untripped = detector.tripped;
		detector.tripped = false;
	}
}

static void debounce_task_fn(TaskAction* this_task)
{
	(void)this_task;

	int i;
	bool at_least_one_just_tripped = false;

	s_tripped_count = 0;

	for (i=0; i<NUMBER_OF_DETECTORS; i++)
	{
		if (detector_in_sequence(i))
		{
			debounce_detector(s_detectors[i], digitalRead(DETECTOR_PINS[i])==s_detector_trip_condition);
			if (s_detectors[i].tripped) { s_tripped_count++; }
			at_least_one_just_tripped |= s_detectors[i].just_tripped;
		}
	}

	if (at_least_one_just_tripped && (s_tripped_count == 1))
	{
		log_tripped( get_tripped(s_detectors), s_sequence );
		*sp_detector_update_flag = true;		
	}
}
static TaskAction s_debounce_task(debounce_task_fn, 10, INFINITE_TICKS);

/*
 * Public Functions
 */

void detectors_setup(bool& detector_update_flag)
{

	pinMode(DETECTOR_POLARITY_PIN, INPUT_PULLUP);
	delay(50);
	s_detector_trip_condition = digitalRead(DETECTOR_POLARITY_PIN) == LOW ? HIGH : LOW;

	Serial.print("Detector: Tripping on pin ");
	Serial.println(s_detector_trip_condition == HIGH ? "high" : "low");

	sp_detector_update_flag = &detector_update_flag;

	for (uint8_t i=0; i<SEQUENCE_LENGTH; i++)
	{
		s_sequence[i] = NO_DETECT;
	}

	for (uint8_t i=0; i<NUMBER_OF_DETECTORS; i++)
	{
		pinMode(DETECTOR_PINS[i], INPUT_PULLUP);
		s_detectors[i].just_tripped = false;
		s_detectors[i].tripped = false;
		s_detectors[i].just_untripped = false;
		s_detectors[i].debounce = 0;
	}
}

void detectors_tick()
{
	s_debounce_task.tick();
}

void detectors_get(char * values)
{
	if (!values) { return; }
	
	for (uint8_t i=0; i<SEQUENCE_LENGTH; i++)
	{
		values[i] = s_sequence[i];
	}
}

bool detectors_match_sequence(char const * to_match)
{
	if (!to_match) { return false; }

	for (uint8_t i=0; i<SEQUENCE_LENGTH; i++)
	{
		if (to_match[i] != s_sequence[i]) { return false; }
	}
	return true;
}
