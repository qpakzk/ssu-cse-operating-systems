#ifndef		__SCHED_H__
#define		__SCHED_H__

#include <list.h>
#include <proc/proc.h>

void schedule(void);

#define QUE_LV_MAX 3
#define LV1_TIMER 40
#define LV2_TIMER 80

#endif
