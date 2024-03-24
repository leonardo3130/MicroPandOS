#ifndef INTERRUPTS
#define INTERRUPTS
#include "../../headers/const.h"
#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "initial.h" 
#include <umps/const.h>
#include <umps/libumps.h>
#include <umps/arch.h>

void interruptHandler(int cause, state_t *exception_state);

#endif
