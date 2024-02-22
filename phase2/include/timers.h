#ifndef TIMERS
#define TIMER
#include <umps/const.h>
#include <umps/arch.h>
#include <umps/libumps.h>

extern unsigned int getTOD();
extern void updateCPUtime(pcb_t *p, unsigned int *start); 
extern void setIntervalTimer(unsigned int t);
extern void setPLT(unsigned int t);
extern unsigned int getPLT();

#endif
