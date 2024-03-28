#include "include/initial.h"

/*
    This module implements initNucleus() and the Nucleus’s global variables (e.g.
    process count, soft blocked count, blocked PCBs lists/pointers, etc.).
*/

LIST_HEAD(Ready_Queue);
LIST_HEAD(Locked_disk);
LIST_HEAD(Locked_flash);
LIST_HEAD(Locked_terminal_recv);
LIST_HEAD(Locked_terminal_transm);
LIST_HEAD(Locked_ethernet);
LIST_HEAD(Locked_printer);
LIST_HEAD(Locked_pseudo_clock);


int process_count;
int soft_blocked_count;
int start;
int pid_counter;

pcb_t *current_process;
pcb_t *ssi_pcb;

void initPassupVector(){
    passupvector_t *passupv;
    passupv = (passupvector_t *) PASSUPVECTOR;

    passupv-> tlb_refill_handler  = (memaddr) uTLB_RefillHandler;
    passupv->tlb_refill_stackPtr  = (memaddr) KERNELSTACK;
    passupv-> exception_handler  = (memaddr) exceptionHandler;
    passupv->exception_stackPtr   = (memaddr) KERNELSTACK;
}

void initFirstProcesses(){
    //  6. Instantiate a first process, place its PCB in the Ready Queue, and increment Process Count.
    ssi_pcb = allocPcb();
    ssi_pcb->p_pid = 1;
    ssi_pcb->p_supportStruct = NULL;

    // IEc: The “current” global interrupt enable bit. When 0, regardless
    // of the settings in Status.IM all interrupts are disabled. When 1, interrupt
    // acceptance is controlled by Status.IM.
    ssi_pcb->p_s.status = ALLOFF | IEPON | IMON | TEBITON;
    /*
        the SP set to RAMTOP (i.e. use the last RAM frame for its stack), and its PC set to the
        address of SSI_function_entry_point
    */
    RAMTOP(ssi_pcb->p_s.reg_sp);
    ssi_pcb->p_s.pc_epc = (memaddr) SSILoop;

    // general purpose register t9
    ssi_pcb-> p_s.gpr[24] = ssi_pcb-> p_s.pc_epc;
    insertProcQ(&Ready_Queue, ssi_pcb);
    ++process_count;


    //  7.  Instantiate a second process, place its PCB in the Ready Queue, and increment Process Count
    pcb_t *toTest = allocPcb();
    toTest->p_pid = 2;
    toTest->p_supportStruct = NULL;


    toTest->p_s.status = ALLOFF | IEPON | IMON | TEBITON;
    RAMTOP(toTest->p_s.reg_sp);
    toTest->p_s.reg_sp -= (2 * PAGESIZE);
    toTest->p_s.pc_epc = (memaddr) test;

    toTest-> p_s.gpr[24] = toTest-> p_s.pc_epc;
    insertProcQ(&Ready_Queue, toTest);
    ++process_count;

}



int main(int argc, char const *argv[])
{
    //  2. Passup Vector
    initPassupVector();

    //  3. Initialize the Level 2 (Phase 1) data structures:
    initPcbs();
    initMsgs();

    //  4
    //  integer indicating the number of started, but not yet terminated processes
    process_count = 0;

    //  This integer is the number of started, but not terminated
    //  processes that in are the "blocked" state due to an I/O or timer request.
    soft_blocked_count = 0;

    pid_counter = 3;    //pid 1 is SSI, pid 2 is test

    //  Tail pointer to a queue of PCBs that are in the “ready” state
    mkEmptyProcQ(&Ready_Queue);

    //  list of blocked PCBs for each external (sub)device
    mkEmptyProcQ(&Locked_disk);
    mkEmptyProcQ(&Locked_flash);
    mkEmptyProcQ(&Locked_terminal_recv);
    mkEmptyProcQ(&Locked_terminal_transm);
    mkEmptyProcQ(&Locked_ethernet);
    mkEmptyProcQ(&Locked_printer);
    mkEmptyProcQ(&Locked_pseudo_clock);


    //  5. Load the system-wide Interval Timer with 100 milliseconds
    setIntervalTimer(PSECOND);

    initFirstProcesses();

    //  8. Call the Scheduler.
    scheduler();
    return EXIT_SUCCESS;
}

