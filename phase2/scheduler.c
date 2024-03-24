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
            /* DA FARE

                Important: Before executing the WAIT instruction, the Scheduler must first set the Status register
                to enable interrupts and either disable the PLT (also through the Status register), or load it with
                a very large value. The first interrupt that occurs after entering a Wait State should not be for the PLT.

            */
            current_process = NULL;
            unsigned int current_status = getSTATUS();
            // bit or con 0x0000FF00 accende i bit dell'interrupt mask --> interrupt attivi (bit da 15 a 8)
            // bit and con 0xF7FFFFFF spegne il bit di attivazione del PLT (bit 27)
            current_status = (current_status | 0x0000FF00 ) & 0xF7FFFFFF;
            setSTATUS(current_status);
            WAIT();
        }else if(process_count > 0 && soft_blocked_count == 0){
            PANIC();
        }else if(process_count == 1){
            HALT();
        }
    }
}


