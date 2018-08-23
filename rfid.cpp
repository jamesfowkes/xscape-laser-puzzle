/* Arduino Library Includes */

#include "EEPROMex.h"
#include "AltSoftSerial.h"

/* Application Includes */

#include "rfid.h"

/* Private Variables */

static AltSoftSerial s_alt_serial;
static bool * sp_rfid_update_flag;
static bool s_serial_rx_flag = false;
static UID s_uid;
static uint8_t s_uid_index;
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

static void print_uid(UID& uid)
{
    Serial.println(uid.bytes);
}

/*
 * Public Functions
 */

void rfid_setup(bool& rfid_update_flag)
{
    sp_rfid_update_flag = &rfid_update_flag;
    s_alt_serial.begin(9600);
}

void rfid_tick()
{
    char c;
    if (s_alt_serial.available())
    {
        c = s_alt_serial.read();
        switch(c)
        {
        case 2:
            *sp_rfid_update_flag = false;
            s_uid_index = 0;
            s_uid.bytes[s_uid_index] = '\0';
            break;
        case 3:
            *sp_rfid_update_flag = true;
            break;
        default:
            s_uid.bytes[s_uid_index++] = c;
            s_uid.bytes[s_uid_index] = '\0';
            break;
        }
    }
}

void rfid_print_current_uid()
{
    print_uid(s_uid);
}

void rfid_print_saved_uid()
{
    UID to_print;
    get_eeprom_uid(to_print);
    print_uid(to_print);
}

void rfid_save_current_uid()
{
    set_eeprom_uid(s_uid);
}

bool rfid_match_saved()
{
    UID eeprom_uid;
    get_eeprom_uid(eeprom_uid);

    return strcmp(eeprom_uid.bytes, s_uid.bytes) == 0;
}
