#include "include/interrupts.h"

static pcb_t *unblockProcessByDeviceNumber(int device_number, struct list_head *list) {
  pcb_t* tmp;
  list_for_each_entry(tmp, list, p_list) {
    if(tmp->dev_no == device_number)
      return outProcQ(list, tmp);
  }
  return NULL;
}

static void deviceInterruptHandler(int line, int cause, state_t *exception_state) {
	devregarea_t *device_register_area = (devregarea_t *)BUS_REG_RAM_BASE;
	unsigned int interrupting_devices_bitmap = device_register_area->interrupt_dev[line - 3];
	unsigned int device_status;
  unsigned int device_number;

  //ordered by priority
  if(interrupting_devices_bitmap & DEV0ON)
    device_number = 0;
  else if(interrupting_devices_bitmap & DEV1ON)
    device_number = 1;
  else if(interrupting_devices_bitmap & DEV2ON)
    device_number = 2;
  else if(interrupting_devices_bitmap & DEV3ON)
    device_number = 3;
  else if(interrupting_devices_bitmap & DEV4ON)
    device_number = 4;
  else if(interrupting_devices_bitmap & DEV5ON)
    device_number = 5;
  else if(interrupting_devices_bitmap & DEV6ON)
    device_number = 6;
  else if(interrupting_devices_bitmap & DEV7ON)
    device_number = 7;

  pcb_t* unblocked_pcb;

  if(line == IL_TERMINAL){
    termreg_t *device_register = (termreg_t *)DEV_REG_ADDR(line, device_number);
    //gestione interrupt terminale --> 2 sub-devices
    if(device_register->transm_status == 5) { 
      //input terminale
      device_status = device_register->transm_status;
      device_register->transm_command = ACK;
      unblocked_pcb = unblockProcessByDeviceNumber(device_number, &Locked_terminal_in);
    }
    else {
      /*klog_print("\ninterrupt output\n");
      klog_print_dec(line);
      klog_print("\n");
      klog_print_dec(device_number);
      klog_print("\n");*/
      //output terminale
      device_status = device_register->recv_status;
      device_register->recv_command = ACK;
      unblocked_pcb = unblockProcessByDeviceNumber(device_number, &Locked_terminal_out);
    }
  } 
  else{
    dtpreg_t *device_register = (dtpreg_t *)DEV_REG_ADDR(line, device_number);
    //gestione interrupt di tutti gli altri dispositivi I/O
    device_status = device_register->status;
    device_register->command = ACK;
    switch (line) {
      case IL_DISK:
        unblocked_pcb = unblockProcessByDeviceNumber(device_number, &Locked_disk);
        break;
      case IL_FLASH:
        unblocked_pcb = unblockProcessByDeviceNumber(device_number, &Locked_flash);
        break;
      case IL_ETHERNET:
        unblocked_pcb = unblockProcessByDeviceNumber(device_number, &Locked_ethernet);
        break;
      case IL_PRINTER:
        unblocked_pcb = unblockProcessByDeviceNumber(device_number, &Locked_printer);
        break;
      default:
        unblocked_pcb = NULL;
        break;
    }
  }

  if(unblocked_pcb) {
    msg_t *msg = allocMsg();
    msg->m_sender = ssi_pcb;
    msg->m_payload = (memaddr)(&device_status);
    insertMessage(&(unblocked_pcb->msg_inbox), msg);
    insertProcQ(&Ready_Queue, unblocked_pcb);
    soft_blocked_count--;
  }

  if(current_process) {
    LDST(exception_state);
  }
  else {
    scheduler();
  }
}

static void localTimerInterruptHandler(state_t *exception_state) {
  setPLT(ACK);
  updateCPUtime(current_process);
  saveState(&(current_process->p_s), exception_state);
  insertProcQ(&Ready_Queue, current_process);
  soft_blocked_count--;
  scheduler();
}

static void pseudoClockInterruptHandler(state_t* exception_state) {
  setIntervalTimer(PSECOND);
  pcb_t *unblocked_pcb = removeProcQ(&Locked_pseudo_clock);
  while (unblocked_pcb) {
    soft_blocked_count--;
    msg_t *msg = allocMsg();
    msg->m_sender = ssi_pcb;
    msg->m_payload = (memaddr)NULL;
    insertMessage(&(unblocked_pcb->msg_inbox), msg);
    insertProcQ(&Ready_Queue, unblocked_pcb);
    unblocked_pcb = removeProcQ(&Locked_pseudo_clock);
  }
  LDST(exception_state);
}

void interruptHandler(int cause, state_t *exception_state) {
	//riconoscimento interrupt --> in ordine di priorità
  if (CAUSE_IP_GET(cause, IL_CPUTIMER))
		localTimerInterruptHandler(exception_state);
	else if (CAUSE_IP_GET(cause, IL_TIMER))
		pseudoClockInterruptHandler(exception_state);
	else if (CAUSE_IP_GET(cause, IL_DISK))
		deviceInterruptHandler(IL_DISK, cause, exception_state);
	else if (CAUSE_IP_GET(cause, IL_FLASH))
		deviceInterruptHandler(IL_FLASH, cause, exception_state);
	else if (CAUSE_IP_GET(cause, IL_ETHERNET))
		deviceInterruptHandler(IL_ETHERNET, cause, exception_state);
	else if (CAUSE_IP_GET(cause, IL_PRINTER))
		deviceInterruptHandler(IL_PRINTER, cause, exception_state);
	else if (CAUSE_IP_GET(cause, IL_TERMINAL))
    deviceInterruptHandler(IL_TERMINAL, cause, exception_state);
}
/*Device register type for disks, flash and printers
typedef struct dtpreg {
unsigned int status;
unsigned int command;
unsigned int data0;
unsigned int data1;
} dtpreg_t;
 Device register type for terminals
typedef struct termreg {
unsigned int recv_status;
unsigned int recv_command;
unsigned int transm_status;
unsigned int transm_command;
} termreg_t;
typedef union devreg {
dtpreg_t dtp;
termreg_t term;
} devreg_t;
 Bus register area
typedef struct devregarea {
unsigned int rambase;
unsigned int ramsize;
unsigned int execbase;
unsigned int execsize;
unsigned int bootbase;
unsigned int bootsize;
unsigned int todhi;
unsigned int todlo;
unsigned int intervaltimer;
unsigned int timescale;
unsigned int TLBFloorAddr;
unsigned int inst_dev[5];
unsigned int interrupt_dev[5];
devreg_t devreg[5][8];
} devregarea_t;
*/

