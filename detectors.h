#ifndef _DETECTORS_H_
#define _DETECTORS_H_

struct detector
{
	bool tripped;
	bool just_tripped;
	bool just_untripped;
	int debounce;
};
typedef struct detector DETECTOR;

void detectors_setup(bool& detector_update_flag);
void detectors_tick();
void detectors_get(char * values);
bool detectors_match_sequence(char const * to_match);

#endif