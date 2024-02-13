#include "include/exceptions.h"

static void passUpOrDie(int i, state_t *exceptionState) {
  if(Current_Process) {
    if(Current_Process->p_supportStruct) {
      Current_Process->p_supportStruct->sup_exceptState[i] = *exceptionState;
      LDCXT(
        Current_Process->p_supportStruct->sup_exceptContext[i].c_stackPtr,
        Current_Process->p_supportStruct->sup_exceptContext[i].c_status,
        Current_Process->p_supportStruct->sup_exceptContext[i].c_pc,
      );
    }
    else {
      //terminate process 
      //scheduler();
    }
  }
}

static void syscallExceptionHandler(state_t* exceptionState){
  if((exceptionState->p_s.staus << 30) >> 31) { //not in kernel mode // <<28 ?
    exceptionState->cause = (exceptionState->cause & CLEAREXECCODE) | (RI << CAUSESHIFT);
    passUpOrDie(GENERALEXCEPT, exceptionState);
  }
  else {
    //qualcosa con SSI ?
  }
}

void exceptionHandler() {
	state_t *exceptionState = (state_t *)BIOSDATAPAGE;
	int cause = getCAUSE();

	switch ((cause & GETEXECCODE) >> CAUSESHIFT) {
		case IOINTERRUPTS:
			interruptHandler(cause, exceptionState);
			break;
		case 1 ... 3:
			//TLB exceptions --> passo controllo al rispettivo gestore
      passUpOrDie(PGFAULTEXCEPT, exceptionState);
			break;
		case SYSEXCEPTION:
			//anche qui potrebbe sevire passUpOrDie ma la situa vs gestita
      //diversamente
      syscallExceptionHandler(exceptionState);
			break;
		default: //4-7, 9-12
			//Program traps --> passo controllo al rispettivo gestore
      passUpOrDie(GENERALEXCEPT, exceptionState);
			break;
 }
}



void uTLB_RefillHandler() {
  setENTRYHI(0x80000000);
  setENTRYLO(0x00000000);
  TLBWR();
  LDST((state_t*) 0x0FFFF000);
}



