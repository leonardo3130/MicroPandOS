#ifndef EXCEPTIONS
#define EXCEPTIONS
#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../headers/listx.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/msg.h"
#include "interrupts.h"
#include "initial.h"
#include <umps/const.h>
#include <umps/libumps.h>
#include <umps/arch.h>

extern void exceptionHandler();
extern void uTLB_RefillHandler();

#endif
