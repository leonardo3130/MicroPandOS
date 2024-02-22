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
    //devo capire se la roba qu sotto viene fatta dalla routine SYSCALL o se devo farla io
    if(exceptionState->reg_a0 == SENDMESSAGE) {
      //SEND is async
      pcb_t *dest = (pcb_t *)(exceptionState->reg_a1);
      
      if(dest = outProcQ(&Locked_Message, dest)) { //Locked_Message not defined yet --> processes waiting for a Locked_Message
        insertProcQ(&Ready_Queue, dest);
      } 
      else{
        int found = 0;
        pcb_t *tmp;
        list_for_each_entry(tmp, Ready_Queue, p_list) {
          if(tmp == dest) {
            found = 1;
            break;
          }
        }
        if(!found)
          dest = NULL;
        else {
          msg_t *msg = allocMsg();
          msg->payload = exceptionState->reg_a2;
          msg->sender = Current_Process; //non sono sicuro
          insertMessage(&dest->msg_list, msg);
        }
      }
      if(!dest) 
        exceptionState->reg_v0 = DEST_NOT_EXISTS;
      else 
        exceptionState->reg_v0 = 0;
      exceptionState->pc_epc += WORDLEN;
      LDST(exceptionState);
    }
    else if(exceptionState->reg_a0 == RECEIVEMESSAGE) {
      list_head *msg_inbox = &(((pcb_t *)(exceptionState->reg_v0))->msg_list);
      if(list_empty(msg_inbox) || !popMessage(msg_inbox, (pcb_t *)(exceptionState->reg_v0))) { //se non ci sono messaggi in attesa di essere ricevuti --> bloccante
        insertProcQ(&Locked_Message, Current_Process); //blocco del processo, non sono sicuro di Current_Process
        Current_Process->p_s = *exceptionState;
        updateCPUtime(Current_Process, &start); 
        scheduler();
      }
      else {
        exceptionState->pc_epc += WORDLEN;
        LDST(exceptionState);
      }
    }
    
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



