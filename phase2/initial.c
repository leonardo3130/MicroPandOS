#include "../headers/const.h"
#include "../headers/types.h"
#include "../headers/listx.h"
#include "../phase1/headers/pcb.h"

#include <umps/libumps.h>

/*
This module implements main() and the Nucleus’s global variables (e.g.
process count, soft blocked count, blocked PCBs lists/pointers, etc.).
*/

//  1. Declare the Level 3 global variables.
unsigned int process_count;
unsigned int soft_blocked_count;

LIST_HEAD(Ready_Queue);
LIST_HEAD(locked_pcbs);

LIST_HEAD(Locked_disk);
LIST_HEAD(Locked_flash);
LIST_HEAD(Locked_terminal_in);
LIST_HEAD(Locked_terminal_out);
LIST_HEAD(Locked_ethernet);
LIST_HEAD(Locked_printer);

LIST_HEAD(Locked_Message)


pcb_t *Current_Process;

extern void test();
extern void scheduler();

void initNucleus(){
    //  2. Passup Vector
    passupvector_t *passupv;
    passupv = (passupvector_t *) PASSUPVECTOR;

    passupv-> tlb_refll_handler  = (memaddr) uTLB_RefillHandler
    passupv.tlb_refill_stackPtr  = (unsigned int *) KERNELSTACK;
    passupv-> exception_handler  = (memaddr) exceptionHandler;
    passupv.exception_stackPtr   = (unsigned int *) KERNELSTACK;


    //  3. Initialize the Level 2 (Phase 1) data structures:
    initPcbs();
    initMsgs();

    // 4
    //  integer indicating the number of started, but not yet terminated processes
    process_count = 0;

    //  This integer is the number of started, but not terminated
    //  processes that in are the “blocked” state due to an I/O or timer request.
    soft_blocked_count = 0;

    //  Tail pointer to a queue of PCBs that are in the “ready” state
    mkEmptyProcQ(Ready_Queue);

    //  list of blocked PCBs for each external (sub)device
    mkEmptyProcQ(locked_pcbs);


    //  5. Load the system-wide Interval Timer with 100 milliseconds
    LDIT(PSECOND);

    //  6. Instantiate a first process, place its PCB in the Ready Queue, and increment Process Count.
    pcb_t *toInsert = allocPcb();

    toInsert-> p_sib = NULL;
    toInsert-> p_supportStruct = NULL;
    toInsert-> p_time = 0;

    toInsert-> p_s

    // IEo & KUo: bits 4-5 - the “previous” settings of the Status.IEp and Status.KUp - denoted the “old” bit settings
    toInsert-> p_s.gpr[4] = toInsert-> p_s.gpr[2];
    toInsert-> p_s.gpr[5] = toInsert-> p_s.gpr[3];

    // IEp & KUp: bits 2-3 - the “previous” settings of the Status.IEc and Status.KUc.
    toInsert-> p_s.gpr[2] = toInsert-> p_s.gpr[0];
    toInsert-> p_s.gpr[3] = toInsert-> p_s.gpr[1];

    // IEc: The “current” global interrupt enable bit. When 0, regardless
    // of the settings in Status.IM all interrupts are disabled. When 1, interrupt
    // acceptance is controlled by Status.IM.
    toInsert-> p_s.gpr[0] = 1;

    //  KUc: bit 1 - The “current” kernel-mode user-mode control bit. When Status.KUc=0 the processor is in kernel-mode.
    toInsert-> p_s.gpr[1] = 0;

    // ENABLE INTERRUPT [IM: bits 8-15 - The Interrupt Mask]
    for(int i=8; i<16; i++)
        toInsert-> p_s.gpr[i] = 1;

    /*
    MANCANTE

    the SP set to RAMTOP (i.e. use the last RAM frame for its stack), and its PC set to the
    address of SSI_function_entry_point

    */

    //  toInsert-> p_s.gpr[24] = toInsert-> p_s.pc_epc;
    insertProcQ(Ready_Queue, toInsert);
    ++process_count;

    //  7.  Instantiate a second process, place its PCB in the Ready Queue, and increment Process Count
    *toInsert = allocPcb();


    //  DA FINIRE


    insertProcQ(Ready_Queue, toInsert);
    ++process_count;

    //  8. Call the Scheduler.
    scheduler();
}
