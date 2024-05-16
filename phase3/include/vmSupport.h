#ifndef VMSUPPORT_H
#define VMSUPPORT_H

#include "../../phase2/include/initial.h"
#include "./initProc.h"

extern pcb_t* currentProcess;
void uTLB_refillHandler();

/*

#define KUSEG 0x80000000
#define VPNSHIFT 12
#define GETPAGENO 0x3FFFF000

*/

#define GETVPN(T) ((T >= KUSEG && T < 0xBFFFF000) ? ((T & GETPAGENO) >> VPNSHIFT) : 31)

#endif
