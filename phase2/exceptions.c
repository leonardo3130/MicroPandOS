#include "include/exceptions.h"
//#include "../klog.c"

void saveState(state_t* dest, state_t* to_copy){
  dest->entry_hi = to_copy->entry_hi;
  dest->cause = to_copy->cause;
  dest->status = to_copy->status;
  dest->pc_epc = to_copy->pc_epc;
  for(int i = 0; i < STATE_GPR_LEN; i++) //29, ma registri definiti sono 31 --> ????
    dest->gpr[i] = to_copy->gpr[i];
  dest->hi = to_copy->hi;
  dest->lo = to_copy->lo;
}

static void passUpOrDie(int i, state_t *exception_state) {
  if(current_process) {
    if(current_process->p_supportStruct != NULL) {
      saveState(&(current_process->p_supportStruct->sup_exceptState[i]), exception_state);
      LDCXT(current_process->p_supportStruct->sup_exceptContext[i].stackPtr,
            current_process->p_supportStruct->sup_exceptContext[i].status,
            current_process->p_supportStruct->sup_exceptContext[i].pc
      );
    }
    else {
      ssi_terminate_process(current_process);
      scheduler();
    }
  }
}

static void addrToDevice(int *line, int *n_dev, int *term, memaddr *command_address){
    for (int i = 0; i < 5; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        if(i == IL_TERMINAL) {
          termreg_t *base_address = (termreg_t *)DEV_REG_ADDR(i + 3, j);
          if(&(base_address->recv_command) == command_address){
            *line = i + 3;
            *n_dev = j;
            *term = 0;
            break;
          }
          else if(&(base_address->transm_command) == command_address){
            *line = i + 3;
            *n_dev = j;
            *term = 1;
            break;
          }
        }
        else {
            dtpreg_t *base_address = (dtpreg_t *)DEV_REG_ADDR(i + 3, j);
            if(&(base_address->command) == command_address){
              *line = i + 3;
              *n_dev = j;
              *term = -1;
              break;
            }
        }
      }
    }
  }

static pcb_t *unblockProcessByService(pcb_t *dest, int service, struct list_head *list) {
  pcb_t* tmp;
  list_for_each_entry(tmp, list, p_list) {
    if(dest == tmp && tmp->service == service) {
      return outProcQ(list, tmp);
    }
  }
  return NULL;
}

static void syscallExceptionHandler(state_t* exception_state){
  if((exception_state->status << 30) >> 31) { //not in kernel mode // <<28 ?
    exception_state->cause = (exception_state->cause & CLEAREXECCODE) | (EXC_RI << CAUSESHIFT);
    passUpOrDie(GENERALEXCEPT, exception_state);
  }
  else {
    if(exception_state->reg_a0 == SENDMESSAGE) {
      //SEND is async
      pcb_t *dest = (pcb_t *)(exception_state->reg_a1);
      if(dest == ssi_pcb) {
        ssi_payload_t *payload = (ssi_payload_t *)exception_state->reg_a2;
        current_process->service = payload->service_code;
        if(current_process->service == DOIO) {
          int device, device_number, term;
          addrToDevice(&device, &device_number, &term, ((ssi_do_io_t *)(payload->arg))->commandAddr);
          current_process->device = device;
          current_process->dev_no = device_number;
          current_process->term = term;
          //klog_print_dec(device);
          //klog_print("\n");
          //klog_print_dec(device_number);
          //klog_print("\n");
          //klog_print_dec(term);
        }
        //klog_print_dec(current_process->service);
      }

      int nogood = 0;
      pcb_t* dest_tmp = unblockProcessByService(dest, dest->service, &Locked_Message);
      if(dest_tmp != NULL) {
        /*if(dest_tmp == ssi_pcb)
          klog_print("\nunblock ssi");
        if(dest_tmp == dest)
          klog_print("\nok");*/
        dest->p_s.reg_v0 = (memaddr)current_process;
        //klog_print_hex((unsigned int)current_process);
        dest->p_s.reg_a2 = exception_state->reg_a2;
        //klog_print_dec(((ssi_payload_t *)(dest->p_s.reg_a2))->service_code);
        insertProcQ(&Ready_Queue, dest);
        soft_blocked_count--;
      }
      else{
        //destinatario già sulla ready queue --> non in attesa
        int found = 0;
        pcb_t *tmp;
        list_for_each_entry(tmp, &Ready_Queue, p_list) {
          if(tmp == dest) {
            found = 1;
            break;
          }
        }
        if(!found && dest != current_process) {
          dest = NULL;
        }
        else {
          msg_t *msg;
          if(msg = allocMsg()) {
            msg->m_payload = exception_state->reg_a2;
            msg->m_sender = current_process;
            //klog_print("\ncreated addres\n");
            //klog_print_hex((memaddr)(msg));
            insertMessage(&(dest->msg_inbox), msg);
          }
          else
            nogood = 1;
        }
      }
      if(nogood)
        exception_state->reg_v0 = MSGNOGOOD;
      else if(!dest)
        exception_state->reg_v0 = DEST_NOT_EXIST;
      else
        exception_state->reg_v0 = 0;
      exception_state->pc_epc += WORDLEN;
      LDST(exception_state);
    }
    else if(exception_state->reg_a0 == RECEIVEMESSAGE) {
      struct list_head *msg_inbox = &(current_process->msg_inbox);
      int isEmpty = list_empty(msg_inbox);
      msg_t *msg;
      if(exception_state->reg_a1 == ANYMESSAGE && isEmpty == 0) {
        msg = popMessage(msg_inbox, (pcb_t *)NULL);
        //klog_print("here1\n");
      }
      else if(isEmpty == 0) {
        msg = popMessage(msg_inbox, (pcb_t *)(exception_state->reg_a1));
        //klog_print("here2\n");
      }
      if(isEmpty || msg == NULL) { //bloccante !!!!
        if(current_process->service == DOIO) {
          switch (current_process->device) {
            case IL_DISK:
              insertProcQ(&Locked_disk, current_process);
              break;
            case IL_FLASH:
              insertProcQ(&Locked_flash, current_process);
              break;
            case IL_ETHERNET:
              insertProcQ(&Locked_ethernet, current_process);
              break;
            case IL_PRINTER:
              insertProcQ(&Locked_printer, current_process);
              break;
            case IL_TERMINAL:
              if(current_process->term)
                insertProcQ(&Locked_terminal_in, current_process);
              else
                insertProcQ(&Locked_terminal_out, current_process);
              break;
            default:
              break;
          }
          soft_blocked_count++;
          saveState(&(current_process->p_s), exception_state);
          updateCPUtime(current_process);
          scheduler();
        }
        else if (current_process->service == CLOCKWAIT) {
          //inserimento già fatto da ssi
          insertProcQ(&Locked_pseudo_clock, current_process); //blocco del processo --> da capire se va fatto qua o in ssi
          soft_blocked_count++;
          saveState(&(current_process->p_s), exception_state);
          updateCPUtime(current_process);
          scheduler();
        }
        else {
          insertProcQ(&Locked_Message, current_process); //blocco del processo
          //if(current_process == ssi_pcb)
          //  klog_print("\nok block");
          soft_blocked_count++;
          saveState(&(current_process->p_s), exception_state);
          updateCPUtime(current_process);
          scheduler();
        }
      }
      else {
        //klog_print("\nreceived address\n");
        //klog_print_hex((memaddr)(msg));
        exception_state->pc_epc += WORDLEN;
        exception_state->reg_v0 = (memaddr)(msg->m_sender);
        exception_state->reg_a2 = msg->m_payload;
        freeMsg(msg);
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
    case 4 ... 7:
    case 9 ... 12:
			//Program traps --> passo controllo al rispettivo gestore
      passUpOrDie(GENERALEXCEPT, exception_state);
			break;
		default: //4-7, 9-12
      PANIC();
 }
}

void uTLB_RefillHandler() {
  setENTRYHI(0x80000000);
  setENTRYLO(0x00000000);
  TLBWR();
  LDST((state_t*) 0x0FFFF000);
}


