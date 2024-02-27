#include "include/initial.h"

/*
    This module implements initNucleus() and the Nucleus’s global variables (e.g.
    process count, soft blocked count, blocked PCBs lists/pointers, etc.).
*/


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

    pid_counter = 2;    //pid 0 is SSI, pid 1 is test

    //  Tail pointer to a queue of PCBs that are in the “ready” state
    mkEmptyProcQ(Ready_Queue);

    //  list of blocked PCBs for each external (sub)device
    mkEmptyProcQ(Locked_disk);
    mkEmptyProcQ(Locked_flash);
    mkEmptyProcQ(Locked_terminal_in);
    mkEmptyProcQ(Locked_terminal_out);
    mkEmptyProcQ(Locked_ethernet);
    mkEmptyProcQ(Locked_printer);

    mkEmptyProcQ(Locked_Message)


    //  5. Load the system-wide Interval Timer with 100 milliseconds
    setIntervalTimer(PSECOND);

    //  6. Instantiate a first process, place its PCB in the Ready Queue, and increment Process Count.
    ssi_pcb = allocPcb();
    ssi_pcb->p_pid = 0;

    // IEc: The “current” global interrupt enable bit. When 0, regardless
    // of the settings in Status.IM all interrupts are disabled. When 1, interrupt
    // acceptance is controlled by Status.IM.
    ssi_pcb->p_s.status |= (1<<1);

    //  KUc: bit 1 - The “current” kernel-mode user-mode control bit. When Status.KUc=0 the processor is in kernel-mode.
    ssi_pcb->p_s.status &= (2<<1);   // Vado in Kernel mode

    // Abilito gli interrupt [IM: bits 8-15 - The Interrupt Mask]
    ssi_pcb->p_s.status |= (255<<8);

    /*
        the SP set to RAMTOP (i.e. use the last RAM frame for its stack), and its PC set to the
        address of SSI_function_entry_point
    */
    RAMTOP(ssi_pcb->p_s.reg_sp);
    ssi_pcb->p_s.s_pc = (memaddr) SSI_function_entry_point;

    // general purpose register t9
    ssi_pcb-> p_s.gpr[24] = ssi_pcb-> p_s.pc_epc;
    insertProcQ(Ready_Queue, ssi_pcb);
    ++process_count;


    //  7.  Instantiate a second process, place its PCB in the Ready Queue, and increment Process Count
    pcb_t *toTest = allocPcb();
    toTest->p_pid = 1;

    toTest->p_s.status |= (1<<1);
    toTest->p_s.status &= (2<<1);   // Vado in Kernel mode
    toTest->p_s.status |= (255<<8);

    RAMTOP(toTest->p_s.reg_sp);
    toTest->p_s.reg_sp -= (2 * PAGESIZE);
    toTest->p_s.s_pc = (memaddr) test;

    toTest-> p_s.gpr[24] = toTest-> p_s.pc_epc;
    insertProcQ(Ready_Queue, toTest);
    ++process_count;

    //  8. Call the Scheduler.
    scheduler();
}
