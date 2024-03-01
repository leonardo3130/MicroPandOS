#include "include/interrupts.h"

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

  devreg_t *device_register = DEV_REG_ADDR(line, device_number);
  pcb_t* unblocked_pcb;

  if(line == IL_TERMINAL){
    //gestione interrupt terminale --> 2 sub-devices
    if(device_register->transm_status == 5) {
      device_status = device_register->transm_status;
      device_register->transm_command = ACK;
      unblocked_pcb = removeProcQ(Locked_terminal_in);
    }
    else {
      device_status = device_register->recv_status;
      device_register->recv_command = ACK;
      unblocked_pcb = removeProcQ(&Locked_terminal_out);
    }
  } 
  else{
    //gestione interrupt di tutti gli altri dispositivi I/O
    device_status = device_register->status;
    device_register->command = ACK;
    switch (line) {
      case IL_DISK:
        unblocked_pcb = removeProcQ(&Locked_disk);
        break;
      case IL_FLASH:
        unblocked_pcb = removeProcQ(&Locked_flash);
        break;
      case IL_ETHERNET:
        unblocked_pcb = removeProcQ(&Locked_ethernet);
        break;
      case IL_PRINTER:
        unblocked_pcb = removeProcQ(&Locked_printer);
        break;
      default:
        unblocked_pcb = NULL;
        break;
    }
  }

  if(unblocked_pcb) {
    unblocked_pcb->p_s.s_v0 = device_status;
    insertProcQ(&Ready_Queue, unblocked_pcb);
    soft_blocked_count--;
  }

  if(current_process) {
    LDST(exception_state);
  }
  else {
    scheduler();
    //Scheduler farà WAIT()
  }
}
static void localTimerInterruptHandler(state_t *exception_state) {
  setPLT(TIMESLICE);
  current_process->p_s = *exception_state;
  updateCPUtime(current_process, &start);
  insertProcQ(&Ready_Queue, current_process);
  soft_blocked_count--;
  scheduler();
}
static void pseudoClockInterruptHandler(state_t* exception_state) {
  setIntervalTimer(PSECOND);
  pcb_t *unblocked_pcb = removeProcQ(&Locked_pseudo_clock);
  while (unblocked_pcb) {
    insertProcQ(&Ready_Queue, unblocked_pcb);
    soft_blocked_count--;
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

