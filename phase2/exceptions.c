#include "include/exceptions.h"

void exceptionHandler() {
	state_t *exceptionState = (state_t *)BIOSDATAPAGE;
	int cause = getCAUSE();

	switch ((cause & GETEXECCODE) >> CAUSESHIFT) {
		case IOINTERRUPTS:
			interruptHandler(cause, exceptionState);
			break;
		case 1 ... 3:
			//TLB exceptions --> passo controllo al rispettivo gestore
			break;
		case SYSEXCEPTION:
			//
			break;
		default: //4-7, 9-12
			//Program traps --> passo controllo al rispettivo gestore
			break;
 }
}



