#include "include/exceptions.h"

static void passUpOrDie(int i, state_t *exception_state) {
  if(current_process) {
    if(current_process->p_supportStruct) {
      current_process->p_supportStruct->sup_exceptState[i] = *exception_state;
      LDCXT(
        current_process->p_supportStruct->sup_exceptContext[i].c_stackPtr,
        current_process->p_supportStruct->sup_exceptContext[i].c_status,
        current_process->p_supportStruct->sup_exceptContext[i].c_pc,
      );
    }
    else {
      //terminate process 
      scheduler();
    }
  }
}

static void syscallExceptionHandler(state_t* exception_state){
  if((exception_state->p_s.staus << 30) >> 31) { //not in kernel mode // <<28 ?
    exception_state->cause = (exception_state->cause & CLEAREXECCODE) | (RI << CAUSESHIFT);
    passUpOrDie(GENERALEXCEPT, exception_state);
  }
  else {
    if(exception_state->reg_a0 == SENDMESSAGE) {
      //SEND is async
      pcb_t *dest = (pcb_t *)(exception_state->reg_a1);
      
      if(dest = outProcQ(&Locked_Message, dest)) { //Locked_Message not defined yet --> processes waiting for a Locked_Message
        msg_t *msg = allocMsg();
        msg->payload = exception_state->reg_a2;
        msg->sender = current_process; //non sono sicuro
        insertMessage(&dest->msg_list, msg);
        insertProcQ(&Ready_Queue, dest);
        soft_blocked_count--;
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
          msg->payload = exception_state->reg_a2;
          msg->sender = current_process; 
          insertMessage(&dest->msg_list, msg);
        }
      }
      if(!dest) 
        exception_state->reg_v0 = DEST_NOT_EXISTS;
      else 
        exception_state->reg_v0 = 0;
      exception_state->pc_epc += WORDLEN;
      LDST(exception_state);
    }
    else if(exception_state->reg_a0 == RECEIVEMESSAGE) {
      list_head *msg_inbox = &(((pcb_t *)(exception_state->reg_a1))->msg_list);
      msg_t *msg = NULL;
      if(list_empty(msg_inbox) || !(msg = popMessage(msg_inbox, (pcb_t *)(exception_state->reg_a1)))) { //bloccante
        insertProcQ(&Locked_Message, current_process); //blocco del processo
        soft_blocked_count++;
        current_process->p_s = *exception_state;
        updateCPUtime(current_process, &start); 
        scheduler();
      }
      else {
        exception_state->pc_epc += WORDLEN;
        exception_state->reg_a2 = msg->payload;
        LDST(exception_state);
      }
    }
    
  }
}

void exceptionHandler() {
	state_t *exception_state = (state_t *)BIOSDATAPAGE;
	int cause = getCAUSE();

	switch ((cause & GETEXECCODE) >> CAUSESHIFT) {
		case IOINTERRUPTS:
			interruptHandler(cause, exception_state);
			break;
		case 1 ... 3:
			//TLB exceptions --> passo controllo al rispettivo gestore
      passUpOrDie(PGFAULTEXCEPT, exception_state);
			break;
		case SYSEXCEPTION:
      syscallExceptionHandler(exception_state);
			break;
		default: //4-7, 9-12
			//Program traps --> passo controllo al rispettivo gestore
      passUpOrDie(GENERALEXCEPT, exception_state);
			break;
 }
}

void uTLB_RefillHandler() {
  setENTRYHI(0x80000000);
  setENTRYLO(0x00000000);
  TLBWR();
  LDST((state_t*) 0x0FFFF000);
}



