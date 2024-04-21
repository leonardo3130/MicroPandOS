#include "./include/vmSupport.h"

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