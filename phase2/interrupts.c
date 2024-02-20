#include "include/interrupts.h"

static void deviceInterruptHandler(int line, int cause, state_t *exceptionState) {
	devregarea_t *devicesRegisterArea = (devregarea_t *)BUS_REG_RAM_BASE;
	unsigned int interruptingDevicesBitmap = devicesRegisterArea->interrupt_dev[line - 3];
	unsigned int deviceStatus;
  unsigned int deviceNumber;

  //ordered by priority
  if(interruptingDevicesBitmap & DEV0ON)
    deviceNumber = 0;
  else if(interruptingDevicesBitmap & DEV1ON)
    deviceNumber = 1;
  else if(interruptingDevicesBitmap & DEV2ON)
    deviceNumber = 2;
  else if(interruptingDevicesBitmap & DEV3ON)
    deviceNumber = 3;
  else if(interruptingDevicesBitmap & DEV4ON)
    deviceNumber = 4;
  else if(interruptingDevicesBitmap & DEV5ON)
    deviceNumber = 5;
  else if(interruptingDevicesBitmap & DEV6ON)
    deviceNumber = 6;
  else if(interruptingDevicesBitmap & DEV7ON)
    deviceNumber = 7;

  devreg_t *deviceRegister = DEV_REG_ADDR(line, deviceNumber);
  pcb_t* unblockedPCB;

  if(line == IL_TERMINAL){
    //gestione interrupt terminale --> 2 sub-devices
    if(deviceRegister->transm_status == 5) {
      deviceStatus = deviceRegister->transm_status;
      deviceRegister->transm_command = ACK;
      unblockedPCB = removeProcQ(Locked_terminal_in);
    }
    else {
      deviceStatus = deviceRegister->recv_status;
      deviceRegister->recv_command = ACK;
      unblockedPCB = removeProcQ(&Locked_terminal_out);
    }
  } 
  else{
    //gestione interrupt di tutti gli altri dispositivi I/O
    deviceStatus = deviceRegister->status;
    deviceRegister->command = ACK;
    switch (line) {
      case IL_DISK:
        unblockedPCB = removeProcQ(&Locked_disk);
        break;
      case IL_FLASH:
        unblockedPCB = removeProcQ(&Locked_flash);
        break;
      case IL_ETHERNET:
        unblockedPCB = removeProcQ(&Locked_ethernet);
        break;
      case IL_PRINTER:
        unblockedPCB = removeProcQ(&Locked_printer);
        break;
      default:
        unblockedPCB = NULL;
        break;
    }
  }

  if(unblockedPCB) {
    unblockedPCB->p_s.s_v0 = deviceStatus;
    insertProcQ(&Ready_Queue, unblockedPCB);
  }

  if(Current_Process) {
    LDST(exceptionState);
  }
  else {
    scheduler();
    //Scheduler farà WAIT()
  }
}
static void localTimerInterruptHandler(state_t *exceptionState) {
  setTIMER(TIMESLICE);
  Current_Process->p_s = *exceptionState;
  insertProcQ(&Ready_Queue, Current_Process);
  scheduler();
}
static void pseudoClockInterruptHandler(state_t* exceptionState) {
  LDIT(PSECOND);
  pcb_t *unblockedPCB = removeProcQ(&Locked_pseudo_clock);
  while (unblockedPCB) {
    insertProcQ(&Ready_Queue, unblockedPCB);
    unblockedPCB = removeProcQ(&Locked_pseudo_clock);
  }
  LDST(exceptionState);
}

void interruptHandler(int cause, state_t *exceptionState) {
	//riconoscimento interrupt --> in ordine di priorità
  if (CAUSE_IP_GET(cause, IL_CPUTIMER))
		localTimerInterruptHandler(exceptionState);
	else if (CAUSE_IP_GET(cause, IL_TIMER))
		pseudoClockInterruptHandler(exceptionState);
	else if (CAUSE_IP_GET(cause, IL_DISK))
		deviceInterruptHandler(IL_DISK, cause, exceptionState);
	else if (CAUSE_IP_GET(cause, IL_FLASH))
		deviceInterruptHandler(IL_FLASH, cause, exceptionState);
	else if (CAUSE_IP_GET(cause, IL_ETHERNET))
		deviceInterruptHandler(IL_ETHERNET, cause, exceptionState);
	else if (CAUSE_IP_GET(cause, IL_PRINTER))
		deviceInterruptHandler(IL_PRINTER, cause, exceptionState);
	else if (CAUSE_IP_GET(cause, IL_TERMINAL))
    deviceInterruptHandler(IL_TERMINAL, cause, exceptionState);
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

