#ifndef VMSUPPORT_H
#define VMSUPPORT_H

#include "../../phase2/include/initial.h"
#include "./initProc.h"

extern pcb_t* currentProcess;
void uTLB_refillHandler();
void pager();

#endif
