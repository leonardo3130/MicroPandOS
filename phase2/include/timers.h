#ifndef TIMERS
#define TIMER
#include <umps/const.h>
#include <umps/arch.h>
#include "../../headers/types.h"
#include "initial.h"
#include <umps/libumps.h>

unsigned int getTOD();
void updateCPUtime(pcb_t *p); 
void setIntervalTimer(unsigned int t);
void setPLT(unsigned int t);
unsigned int getPLT();

#endif
