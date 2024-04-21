#include "./include/vmSupport.h"


// Since all valid ASID values are positive numbers, one can indicate that a frame is unoccupied with 
// an entry of -1 in that frame’s ASID entry in the Swap Pool table [pagina 6/16]
void init_swapPT(){
    for(int i= 0;i < POOLSIZE; i++)
        swap_pool_table[i].sw_asid = -1; 
}


void uTLB_refillHandler(){
    // prendo l'exception_state dalla BIOSDATAPAGE al fine di trovare 
    state_t* exception_state = (state_t *)BIOSDATAPAGE;
    int p; // trovare il modo di prendere la page da exception_state->entry_hi

    setENTRYHI(currentProcess->p_supportStruct->sup_privatePgTbl[p].pte_entryHI);
    setENTRYLO(currentProcess->p_supportStruct->sup_privatePgTbl[p].pte_entryLO);
    
    // scrivo nel TLB
    TLBWR();        

    //Return control to the Current Process to retry the instruction that caused the TLB-Refill event:
    //LDST on the saved exception state located at the start of the BIOS Data Page.                                                                                                                                                                                                                                                                                                                                                                                             
    LDST(exception_state);
}