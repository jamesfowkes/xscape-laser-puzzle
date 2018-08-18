/* Arduino Library Includes */

#include "TaskAction.h"
#include "EEPROMex.h"

/* Application Includes */

#include "application.h"
#include "detectors.h"
#include "rfid.h"

/* Defines, typedefs, constants */

static const char UNLOCK_SEQUENCE[] = {
	/*
	Lasers are numbered 0 to 4 from top to bottom.
	Change the unlock code here!
	*/ 
	0,1,2,3,4
};

static const bool INVERT_MAGLOCK = false;
static const int LASER_CONTROL_PIN = 4;
static const int FIRST_MAGLOCK_CONTROL_PIN = 2;
static const int SECOND_MAGLOCK_CONTROL_PIN = 3;

enum game_mode
{
	MODE_LASERS,
	MODE_RFID,
	MODE_PUZZLE_COMPLETE
};
typedef enum game_mode GAME_MODE;

/* Private Variables */

static bool s_detector_flag = false;
static bool s_rfid_flag = false;

static GAME_MODE s_game_mode = MODE_LASERS;

/* Private Functions */

static void laser_control(bool on)
{
	digitalWrite(LASER_CONTROL_PIN, on ? HIGH : LOW);
}

static void first_maglock_control(bool on)
{
	digitalWrite(FIRST_MAGLOCK_CONTROL_PIN, (on ^ INVERT_MAGLOCK) ? HIGH : LOW);
}

static void second_maglock_control(bool on)
{
	digitalWrite(SECOND_MAGLOCK_CONTROL_PIN, (on ^ INVERT_MAGLOCK) ? HIGH : LOW);	
}

static bool check_and_clear(bool& b)
{
	bool value = b;
	b = false;
	return value;
}

static void debug_task_fn(TaskAction* this_task)
{
	(void)this_task;

	char detectors[5];

	switch(s_game_mode)
	{
	case MODE_LASERS:
		Serial.print("Lasers: ");
		detectors_get(detectors);
		for (uint8_t i = 0; i < NUMBER_OF_DETECTORS; i++)
		{
			Serial.print((int)detectors[i]);
			Serial.print(",");
		}
		Serial.println("");
		break;
	case MODE_RFID:
		Serial.println("RFID: ");
		if (s_rfid_flag)
		{
			Serial.print("Got UID: ");
			rfid_print_current_uid();
			Serial.println("");	
		}
		break;
	case MODE_PUZZLE_COMPLETE:
		Serial.println("Complete");
		break;
	}
}
static TaskAction s_debug_task(debug_task_fn, 1000, INFINITE_TICKS);

static void register_rfid(unsigned long for_seconds)
{
	while(millis() < (for_seconds * 1000UL))
	{
		rfid_tick();
		if (check_and_clear(s_rfid_flag))
		{
			Serial.print("Saving UID: ");
			rfid_print_current_uid();
			Serial.println("");

			rfid_save_current_uid();

			return;
		}
	}
	Serial.print("No RFID seen, using ");
	rfid_print_saved_uid();
	Serial.println("");
}

/* Public Functions */

void setup()
{
	Serial.begin(115200);

	detectors_setup(s_detector_flag);
	rfid_setup(s_rfid_flag);

	pinMode(LASER_CONTROL_PIN, OUTPUT);
	pinMode(FIRST_MAGLOCK_CONTROL_PIN, OUTPUT);
	pinMode(SECOND_MAGLOCK_CONTROL_PIN, OUTPUT);

	laser_control(true);
	first_maglock_control(false);
	second_maglock_control(false);
	register_rfid(5);
}

void loop()
{
	switch(s_game_mode)
	{
	case MODE_LASERS:
		detectors_tick();

		if (check_and_clear(s_detector_flag))
		{
			if (detectors_match_sequence(UNLOCK_SEQUENCE))
			{
				laser_control(false);
				first_maglock_control(true);
				s_game_mode = MODE_RFID;
			}
		}
		break;
	case MODE_RFID:
		rfid_tick();

		if (check_and_clear(s_rfid_flag) && rfid_match_saved())
		{
			second_maglock_control(true);
			s_game_mode = MODE_PUZZLE_COMPLETE;
		}
		break;
	case MODE_PUZZLE_COMPLETE:
		break;
	}
	s_debug_task.tick();
}

