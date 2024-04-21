#ifndef VMSUPPORT_H
#define VMSUPPORT_H

#include "../../phase2/include/initial.h"

extern pcb_t* currentProcess;
swap_t swap_pool_table[POOLSIZE];

void init_swapPT();
void uTLB_refillHandler();


#endif