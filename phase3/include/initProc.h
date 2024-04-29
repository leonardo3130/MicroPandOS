#ifndef INITPROC

#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../headers/listx.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/msg.h"
#include "../../phase2/include/timers.h"
#include "../../phase2/include/exceptions.h"
#include "../../phase3/include/sst.h"
#include "./sysSupport.h"
#include "./vmSupport.h"
#include <umps/const.h>
#include <umps/libumps.h>
#include <umps/arch.h>
#include <umps/cp0.h>


extern support_t ss_array[UPROCMAX]; //support struct array
extern state_t UProc_state[UPROCMAX];
extern pcb_t *swap_mutex_pcb;
extern swap_t swap_pool_table[POOLSIZE];
extern pcb_t *sst_array[UPROCMAX];
//extern pcb_t *terminal_pcbs_recv[UPROCMAX];
extern pcb_t *terminal_pcbs[UPROCMAX];
extern pcb_t *printer_pcbs[UPROCMAX];

void test();

#endif 
