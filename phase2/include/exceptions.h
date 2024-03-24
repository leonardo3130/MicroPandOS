#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../headers/listx.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/msg.h"
#include "interrupts.h"
#include "initial.h"
#include "scheduler.h"
#include "ssi.h"
#include <umps/const.h>
#include <umps/libumps.h>
#include <umps/arch.h>
#include <umps/cp0.h>

void exceptionHandler();
void uTLB_RefillHandler();
void saveState(state_t* new_state, state_t* old_state);
int send(pcb_t *sender, pcb_t *dest, unsigned int payload); 

#endif
