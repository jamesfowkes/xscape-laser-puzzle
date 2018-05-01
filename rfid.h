#ifndef _RFID_H_
#define _RFID_H_

typedef struct {
	byte		size;
	byte		bytes[10];
} UID;

void rfid_setup(bool& rfid_update_flag);
void rfid_tick();

void rfid_print_current_uid();
void rfid_print_saved_uid();
void rfid_save_current_uid();

bool rfid_match_saved();

#endif
