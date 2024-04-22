#include "include/sysSupport.h"

void programTrapExceptionHandler(state_t *exception_state) {
  //se ha lo swap mutex rilascialo --> send a swap mutex process
  ssi_payload_t term_process_payload = {
      .service_code = TERMPROCESS,
      .arg = NULL,
  };
  SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&term_process_payload), 0);
  SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, 0, 0);
}

void syscallExceptionHandler(state_t *exception_state) { 
	//trova modo di riciclare codice syscal invece di fare solo copia incolla
  if(exception_state->reg_a0 == SENDMSG) {
    int nogood;     //valore di ritorno da inserire in reg_v0
    int ready;      //se uguale a 1 il destinatario è nella ready queue, 0 altrimenti
    int not_exists; //se uguale a 1 il destinatario è nella lista pcbFree_h
    pcb_t *dest = NULL;
    if(exception_state->reg_a1 == SST)
			dest = current_process->p_parent; 
		else
    	dest = (pcb_t *)(exception_state->reg_a1);
    ready = findPCB(dest, &Ready_Queue);
    not_exists = findPCB(dest, &pcbFree_h);

    if(not_exists) 
      exception_state->reg_v0 = DEST_NOT_EXIST;  
      else if(ready || dest == current_process) {
      nogood = send(current_process, dest, exception_state->reg_a2);
      exception_state->reg_v0 = nogood;
    }
    else {
      nogood = send(current_process, dest, exception_state->reg_a2);
      insertProcQ(&Ready_Queue, dest);   //il processo era bloccato quindi lo inserisco sulla ready queue
      exception_state->reg_v0 = nogood;
    }

    exception_state->pc_epc += WORDLEN; //non bloccante
    LDST(exception_state);
  } 
  else if(exception_state->reg_a0 == RECEIVEMSG) {
    struct list_head *msg_inbox = &(current_process->msg_inbox);                        //inbox del ricevente
    unsigned int from = exception_state->reg_a1;                                        //da chi voglio ricevere
    msg_t *msg = popMessage(msg_inbox, (from == ANYMESSAGE ? NULL : (pcb_t *)(from)));  //rimozione messaggio dalla inbox del ricevente
           
    if(!msg) { 
      //receive bloccante
      saveState(&(current_process->p_s), exception_state);
      updateCPUtime(current_process);
      scheduler();       
    }
    else {
      //receive non bloccante
      exception_state->reg_v0 = (memaddr)(msg->m_sender);
      if(msg->m_payload != (unsigned int)NULL) {
        //accedo all'area di memoria in cui andare a caricare il payload del messaggio
        unsigned int *a2 = (unsigned int *)exception_state->reg_a2;
        *a2 = msg->m_payload;
      }
      freeMsg(msg); //il messaggio non serve più, lo libero
      exception_state->pc_epc += WORDLEN; //non bloccante
      LDST(exception_state);
    }
  }
}

void generalExceptionHandler(){
  support_t *sup_struct_ptr;
  ssi_payload_t getsup_payload = {
      .service_code = GETSUPPORTPTR,
      .arg = NULL,
  };
  SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&getsup_payload), 0);
  SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&sup_struct_ptr), 0);
	state_t *exception_state = &sup_struct_ptr->sup_exceptState[GENERALEXCEPT];  
	int cause = exception_state.cause;

  switch((cause & GETEXECCODE) >> CAUSESHIFT){
			case SYSEXCEPTION:
          syscallExceptionHandler(exception_state);
					break;
			default:
          programTrapExceptionHandler(exception_state);
  				break;
  }
}
