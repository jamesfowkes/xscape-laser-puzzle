/* Arduino Library Includes */

#include "TaskAction.h"
#include "MFRC522.h"
#include "EEPROMex.h"

/* Application Includes */

#include "rfid.h"

/* Private Variables */

static uint8_t SS_PIN = 10;
static uint8_t RST_PIN = 9;

static bool * sp_rfid_update_flag;
static MFRC522 s_rfid = MFRC522(SS_PIN, RST_PIN);

static UID s_current_uid;

static const int EEPROM_ADDRESS_UID = EEPROM.getAddress(sizeof(UID));

/*
 * Private Functions
 */

static void get_eeprom_uid(UID& uid)
{
	EEPROM.readBlock(EEPROM_ADDRESS_UID, uid);
}

static void set_eeprom_uid(UID& uid)
{
	EEPROM.writeBlock(EEPROM_ADDRESS_UID, uid);
}

static bool get_uid(MFRC522& mfrc522, UID& uid)
{
	if (!mfrc522.PICC_IsNewCardPresent()) { return false; }
	if (!mfrc522.PICC_ReadCardSerial()) { return false; }
	if (mfrc522.uid.size == 0) { return false; }

	memcpy(uid.bytes, mfrc522.uid.uidByte, mfrc522.uid.size);
	uid.size = mfrc522.uid.size;

	return true;
}

static void rfid_task_fn(TaskAction* this_task)
{
	(void)this_task;
	*sp_rfid_update_flag = get_uid(s_rfid, s_current_uid);
}
static TaskAction s_rfid_task(rfid_task_fn, 50, INFINITE_TICKS);

static void print_uid(UID& uid)
{
	if ((uid.size == 4) || (uid.size == 7) || (uid.size == 10))
	{
		for(uint8_t i=0; i<uid.size; i++)
		{
			Serial.print(uid.bytes[i], 16);
		}
	}
	else
	{
		Serial.print(" Invalid UID? Size=");
		Serial.print(uid.size);
	}
}
/*
 * Public Functions
 */

void rfid_setup(bool& rfid_update_flag)
{
	sp_rfid_update_flag = &rfid_update_flag;

	SPI.begin();
	
	bool got_reader = false;
	while(!got_reader)
	{
	    Serial.print("Checking for reader in slot ");
	    s_rfid.PCD_Init();
	    got_reader = s_rfid.PCD_DumpVersionToSerial();
	    delay(50);
    }
}

void rfid_tick()
{
	s_rfid_task.tick();
}

void rfid_print_current_uid()
{
	print_uid(s_current_uid);
}

void rfid_print_saved_uid()
{
	UID to_print;
	get_eeprom_uid(to_print);
	print_uid(to_print);
}

void rfid_save_current_uid()
{
	set_eeprom_uid(s_current_uid);
}

bool rfid_match_saved()
{
	UID eeprom_uid;
	get_eeprom_uid(eeprom_uid);

	if (eeprom_uid.size != s_current_uid.size) { return false; }

	return memcmp(eeprom_uid.bytes, s_current_uid.bytes, eeprom_uid.size) == 0;
}