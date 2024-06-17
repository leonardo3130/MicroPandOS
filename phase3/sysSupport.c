#include "include/sysSupport.h"

void programTrapExceptionHandler(state_t *exception_state) {
  //SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex_process, 0, 0);
  ssi_payload_t term_process_payload = {
      .service_code = TERMPROCESS,
      .arg = NULL,
  };
  SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&term_process_payload), 0);
  SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, 0, 0);
}

void bp() {}
void supSyscallExceptionHandler(state_t *exception_state) { 
  if(exception_state->reg_a0 == SENDMSG) {
    if(exception_state->reg_a1 == PARENT)
      SYSCALL(SENDMESSAGE, (unsigned int)current_process->p_parent, exception_state->reg_a2, 0);
    else
      SYSCALL(SENDMESSAGE, exception_state->reg_a1, exception_state->reg_a2, 0);
  } 
  else if(exception_state->reg_a0 == RECEIVEMSG) {
    bp();
    SYSCALL(RECEIVEMESSAGE, exception_state->reg_a1, exception_state->reg_a2, 0);
    bp();
  }
}

void general_bp(){}
void generalExceptionHandler(){
  general_bp();
  support_t *sup_struct_ptr;
  ssi_payload_t getsup_payload = {
    .service_code = GETSUPPORTPTR,
    .arg = NULL,
  };
  SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&getsup_payload), 0);
  SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&sup_struct_ptr), 0);

	state_t *exception_state = &(sup_struct_ptr->sup_exceptState[GENERALEXCEPT]);

	exception_state->pc_epc += WORDLEN;
  int val = (exception_state->cause & GETEXECCODE) >> CAUSESHIFT;

  switch(val){
    case SYSEXCEPTION:
      supSyscallExceptionHandler(exception_state);
      break;
    default:
      // klog_print_dec(current_process->p_pid);
      // klog_print("\n");

      // viene killato il processo con PID == 7
      programTrapExceptionHandler(exception_state);
      break;
  }
  LDST(exception_state);
}
