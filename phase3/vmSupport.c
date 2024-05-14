#include "./include/vmSupport.h"


static int getPage(){
    
    for (int i = 0; i < POOLSIZE; i++)
        if (swap_pool_table[i].sw_asid == -1)   // -1 significa che la pagina è libera
            return i;

    // se non c'è una pagina libera, use un algoritmo FIFO per trovare una pagina da rimpiazzare
    static int i = -1;
    return i = (i + 1) % POOLSIZE;
}

static void update(int index){
    
    // Imposta il registro ENTRYHI con il valore entryHI dalla swap_pool_table all'indice dato
    setENTRYHI(swap_pool_table[index].sw_pte.pte_entryHI);
    
    // Prova il TLB per trovare una voce corrispondente
    TLBP();
    
    // Se la voce non è presente nel TLB, aggiorna il registro ENTRYLO e scrivilo nel TLB
    if((getINDEX() & PRESENTFLAG) == 0){
        setENTRYLO(swap_pool_table[index].sw_pte.pte_entryLO);
        TLBWI();
    } 
}

static void cleanDirtyPage(int sp_index){
    setSTATUS(getSTATUS() & (~IECON)); // disabilito interrupt per avere mutua esclusione

    swap_pool_table[sp_index].pte_entry_lo &= !VALIDON; // invalido la pagina
    update(sp_index);

    setSTATUS(getSTATUS() | IECON); // riabilito interrupt per rilasciare la mutua esclusione
}

void pager(){
    //prendo la support struct
    support_t * sup_st = (support_t *) SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    
    // TLB-Modification exception
    // If the Cause is a TLB-Modification exception, treat this exception as a program trap
    if(sup_st->sup_exceptState[PGFAULTEXCEPT].s_cause == 1){
       
        /*
            Importante: se il processo da terminare è attualmente in attesa di mutua esclusione su una struttura del livello di 
            supporto (ad esempio la tabella Swap Pool), la mutua esclusione deve essere prima rilasciata (inviare un messaggio) 
            prima di richiedere la terminazione del processo.
        */
        kill_proc();

    } else {
        // INIZIO MUTUA ESCLUSIONE
        // Gain mutual exclusion over the Swap Pool table sending a message to the swap_table PCB and waiting for a response.
        
        // Prendo la pagina dalla entry_hi supp_p->sup_exceptState[PGFAULTEXCEPT].entry_hi
        int p = GETVPN(sup_st->sup_exceptState[PGFAULTEXCEPT].entry_hi);
        
        // Uso il mio algoritmo di rimpiazzamento per trovare la pagina da sostituire
        int i = getPage();

        // è necessario aggiornare la pagina se questa era occupata da un altro frame appartenente ad un altro processo
        // e in caso serve "pulirna" poichè ormai obsoleta (ovviamente tutto ciò in modo atomico per evitare inconsistenza)
        if(swap_pool_table[i].sw_asid != -1){
            cleanDirtyPage(i);
        }




        //FINE MUTUA ESCLUSIONE
        LDST(&(sup_st->sup_exceptState[PGFAULTEXCEPT]));
    }
}

void uTLB_refillHandler(){
    // prendo l'exception_state dalla BIOSDATAPAGE al fine di trovare 
    state_t* exception_state = (state_t *) BIOSDATAPAGE;
    int p = GETVPN(&(exception_state->entry_hi));

    setENTRYHI(currentProcess->p_supportStruct->sup_privatePgTbl[p].pte_entryHI);
    setENTRYLO(currentProcess->p_supportStruct->sup_privatePgTbl[p].pte_entryLO);
    
    // scrivo nel TLB
    TLBWR();        

    //Return control to the Current Process to retry the instruction that caused the TLB-Refill event:
    //LDST on the saved exception state located at the start of the BIOS Data Page.                                                                                                                                                                                                                                                                                                                                                                                             
    LDST(exception_state);
}