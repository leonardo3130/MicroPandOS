#include "../headers/const.h"
#include "../headers/types.h"
#include "include/exceptions.h"
#include <umps/libumps.h>

void exceptionHandler() {
	//vedi pagina 5 spec.
	//getCAUSE() --> ritorna il registro CAUSE di CP0
	//CAUSESHIFT = 2 --> nel registro ExcCode inizia dal secondo bit
	//GETEXECODE = 0x0000007C
	//getCAUSE eseguibile solo in kernel mode
	switch (((getCAUSE() >> CAUSESHIFT) << 27) >> 27) {
		case IOINTERRUPTS:
			//passo controlo a gestore device interrupt	
			break;
		case 1:
		case 2:
		case 3:
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



