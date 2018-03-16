/* Arduino Library Includes */

#include "TaskAction.h"

/* Application Includes */

#include "rfid.h"

/* Private Variables */

static bool * sp_rfid_update_flag;

/*
 * Private Functions
 */

static void rfid_task_fn(TaskAction* this_task)
{
	(void)this_task;
}
static TaskAction s_rfid_task(rfid_task_fn, 10, INFINITE_TICKS);

/*
 * Public Functions
 */

void rfid_setup(bool& rfid_update_flag)
{
	sp_rfid_update_flag = &rfid_update_flag;
}

void rfid_tick()
{
	s_rfid_task.tick();
}
