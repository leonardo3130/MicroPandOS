#include "../headers/const.h"
#include "../headers/types.h"
#include "../phase1/headers/pcb.h"

#include <umps/libumps.h>


extern struct list_head Ready_Queue;
extern pcb_t *Current_Process;
extern unsigned int process_count;
extern unsigned int soft_blocked_count;


void scheduler(){
    if(!emptyProcQ(Ready_Queue)){
        //  1. Remove the PCB from the head of the Ready Queue and store the pointer to the PCB
        //     in the Current Process field.
        Current_Process = removeProcQ(Ready_Queue);

        //  2. Load 5 milliseconds on the PLT [Section 4.1.4-pops].
        setPLT(TIMESLICE);

        //  3. Perform a Load Processor State (LDST) on the processor state stored in PCB of the
        //     Current Process (p_s).
        start = getTOD(); //da includere (timers e initial) quando farai i file .h 
        LDST(&(Current_Process->p_s));
    }
    else{


        if(process_count > 0 && soft_blocked_count > 0 ){
            /* DA FARE

                Important: Before executing the WAIT instruction, the Scheduler must first set the Status register
                to enable interrupts and either disable the PLT (also through the Status register), or load it with
                a very large value. The first interrupt that occurs after entering a Wait State should not be for the PLT.

            */

            WAIT();
        }else if(process_count > 0 && soft_blocked_count == 0){
            PANIC();
        }else if(process_count == 1){
            HALT();
        }
    }
}


