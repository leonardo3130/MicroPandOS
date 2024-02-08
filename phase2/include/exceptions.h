#ifndef EXCEPTIONS
#define EXCEPTIONS
#include "../../headers/const.h"
#include "../../headers/types.h"
#include "interrupts.h"
#include "initial.h"
#include <umps/libumps.h>
#include <umps/arch.h>

extern void exceptionHandler();
extern void uTLB_RefillHandler();

#endif
