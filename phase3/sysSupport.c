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
    if(exception_state->reg_a1 == PARENT)
      SYSCALL(SENDMESSAGE, current_process->p_parent, exception_state->reg_a2, 0);
    else
      SYSCALL(SENDMESSAGE, exception_state->reg_a1, exception_state->reg_a2, 0);
  } 
  else if(exception_state->reg_a0 == RECEIVEMSG) {
    SYSCALL(RECEIVEMESSAGE, exception_state->reg_a1, exception_state->reg_a2, 0);
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
