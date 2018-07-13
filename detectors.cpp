/* Arduino Library Includes */

#include "TaskAction.h"

/* Application Includes */

#include "application.h"
#include "detectors.h"

/* Defines, typedefs, constants */

static const char NO_DETECT = -1;

static const uint8_t DETECTOR_PINS[NUMBER_OF_DETECTORS] = {
	A0,A1,A2,A3,A4
};

static const int DEBOUNCE_COUNT = 5;

static char s_last_tripped[5] = {NO_DETECT, NO_DETECT, NO_DETECT, NO_DETECT, NO_DETECT};

/* Private Variables */

static DETECTOR s_detectors[NUMBER_OF_DETECTORS];
static int s_tripped_count = 0;
static bool * sp_detector_update_flag;

/* Private Functions */

static char get_tripped(DETECTOR * s_detectors)
{
	uint8_t i;
	for (i = 0; i < NUMBER_OF_DETECTORS; i++)
	{
		if (s_detectors[i].tripped) { return i; }
	}
	return NO_DETECT;
}

static void log_tripped(char pressed, char * log)
{
	log[0] = log[1];
	log[1] = log[2];
	log[2] = log[3];
	log[3] = log[4];
	log[4] = pressed;
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
		debounce_detector(s_detectors[i], digitalRead(DETECTOR_PINS[i])==LOW);
		if (s_detectors[i].tripped) { s_tripped_count++; }
		at_least_one_just_tripped |= s_detectors[i].just_tripped;
	}

	if (at_least_one_just_tripped && (s_tripped_count == 1))
	{
		log_tripped( get_tripped(s_detectors), s_last_tripped );
		*sp_detector_update_flag = true;		
	}
}
static TaskAction s_debounce_task(debounce_task_fn, 10, INFINITE_TICKS);

/*
 * Public Functions
 */

void detectors_setup(bool& detector_update_flag)
{
	sp_detector_update_flag = &detector_update_flag;

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
	
	for (uint8_t i=0; i<NUMBER_OF_DETECTORS; i++)
	{
		values[i] = s_last_tripped[i];
	}
}

bool detectors_match_sequence(char const * to_match)
{
	if (!to_match) { return false; }

	for (uint8_t i=0; i<NUMBER_OF_DETECTORS; i++)
	{
		if (to_match[i] != s_last_tripped[i]) { return false; }
	}
	return true;
}
