#ifndef SCHEDULER
#define SCHEDULER

#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../phase1/headers/pcb.h"
#include "timers.h"
#include "initial.h"
#include <umps/libumps.h>



//extern struct list_head Ready_Queue;
//extern pcb_t *Current_Process;
//extern unsigned int process_count;
//extern unsigned int soft_blocked_count;
void scheduler();

#endif
