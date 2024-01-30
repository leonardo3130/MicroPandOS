#include "include/interrupts.h"

static void deviceInterruptHandler(int line, int cause, state_t *exceptionState) {
		
}
static void localTimerInterruptHandler(state_t *exceptionState) {}
static void pseudoClockInterruptHandler(state_t* exceptionState) {}

void interruptHandler(int cause, state_t *exceptionState) {
	//riconoscimento interrupt --> in ordine di priorità
	if (CAUSE_IP_GET(cause, IL_CPUTIMER))
		localTimerInterruptHandler(exceptionState);
	else if (CAUSE_IP_GET(cause, IL_TIMER))
		pseudoClockInterruptHandler(exceptionState);
	else if (CAUSE_IP_GET(cause, IL_DISK))
		deviceInterruptHandler(IL_DISK, cause, exceptionState);
	else if (CAUSE_IP_GET(cause, IL_FLASH))
		deviceInterruptHandler(IL_FLASH, cause, exceptionState);
	else if (CAUSE_IP_GET(cause, IL_ETHERNET))
		deviceInterruptHandler(IL_ETHERNET, cause, exceptionState);
	else if (CAUSE_IP_GET(cause, IL_PRINTER))
		deviceInterruptHandler(IL_PRINTER, cause, exceptionState);
	else if (CAUSE_IP_GET(cause, IL_TERMINAL))
		deviceInterruptHandler(IL_TERMINAL, cause, exceptionState);
}

