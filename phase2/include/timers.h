#ifndef TIMERS
#define TIMER
#include <umps/const.h>
#include <umps/arch.h>
#include <umps/libumps.h>

unsigned int getTOD();
void updateCPUtime(pcb_t *p, unsigned int *start); 
void setIntervalTimer(unsigned int t);
void setPLT(unsigned int t);
unsigned int getPLT();

#endif
