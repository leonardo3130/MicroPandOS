#include "../headers/const.h"
#include "../headers/types.h"
#include "../phase1/headers/pcb.h"
#include "../phase1/headers/msg.h"

#include <umps/libumps.h>

/*
This module implements main() and exports the Nucleus’s global variables (e.g.
process count, soft blocked count, blocked PCBs lists/pointers, etc.).
*/

//  1. Declare the Level 3 global variables.
export unsigned int process_count;
export unsigned int soft_blocked_count;

export list_head Ready_Queue;
export list_head Locked_PCBs;

export list_head Locked_disk;
export list_head Locked_flash;
export list_head Locked_terminal_in;
export list_head Locked_terminal_out;
export list_head Locked_PCBs;
export list_head Locked_PCBs;
export list_head Locked_PCBs;
export list_head Locked_PCBs;


export pcb_t Current_Process;

// extern void test();

void init(){
    //  integer indicating the number of started, but not yet terminated processes
    process_count = 0;

    //  This integer is the number of started, but not terminated
    //  processes that in are the “blocked” state due to an I/O or timer request.
    soft_blocked_count = 0;

    //  Tail pointer to a queue of PCBs that are in the “ready” state
    INIT_LIST_HEAD(Ready_Queue)

    //  list of blocked PCBs for each external (sub)device
    INIT_LIST_HEAD(Locked_PCBs)

    //  2. Passup Vector
    passupvector_t passupvector;
    passupvector->tlb_refll_handler = (memaddr) uTLB_RefillHandler


    //  3. Initialize the Level 2 (Phase 1) data structures:
    initPcbs();
    initMsgs();



    //  5. Load the system-wide Interval Timer with 100 milliseconds
    LDIT(PSECOND)


}
