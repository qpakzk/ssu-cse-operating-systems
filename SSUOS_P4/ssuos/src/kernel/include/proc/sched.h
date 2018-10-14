#ifndef		__SCHED_H__
#define		__SCHED_H__

#include <list.h>
#include <proc/proc.h>

void schedule(void);

#define QUE_LV_MAX 3
//레벨 1 타임 컨텀을 40 tick으로 선언
#define LV1_TIMER 40
//레벨 2 타임 컨텀을 80 tick으로 선언
#define LV2_TIMER 80

#endif
