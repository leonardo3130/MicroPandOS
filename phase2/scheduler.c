#include "./include/scheduler.h"

void scheduler(){
    if(emptyProcQ(&Ready_Queue) == 0){
        //  1. Remove the PCB from the head of the Ready Queue and store the pointer to the PCB
        //     in the Current Process field.
        current_process = removeProcQ(&Ready_Queue);

        //  2. Load 5 milliseconds on the PLT [Section 4.1.4-pops].
        setPLT(TIMESLICE);

        //  3. Perform a Load Processor State (LDST) on the processor state stored in PCB of the
        //     Current Process (p_s).
        start = getTOD(); //da includere (timers e initial) quando farai i file .h
        LDST(&(current_process->p_s));
    }
    else{
        if(process_count > 0 && soft_blocked_count > 0 ){
            current_process = NULL;
            setSTATUS(ALLOFF | IECON | IMON);
            WAIT();
        }else if(process_count > 0 && soft_blocked_count == 0){
            PANIC();
        }else if(process_count == 1){
            HALT();
        }
    }
}


