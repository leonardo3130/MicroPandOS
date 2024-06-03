#include "./include/vmSupport.h"


static int getPage(){
    
    for (int i = 0; i < POOLSIZE; i++)
        if (swap_pool_table[i].sw_asid == -1)   // -1 significa che la pagina e' libera
            return i;

    // se non c'e' una pagina libera, use un algoritmo FIFO per trovare una pagina da rimpiazzare
    static int i = -1;
    return i = (i + 1) % POOLSIZE;
}

static void updateTLB(pteEntry_t p){
    
    // Imposta il registro ENTRYHI con il valore entryHI dalla swap_pool_table all'indice dato
    setENTRYHI(p.pte_entryHI);
    
    // Prova il TLB per trovare una voce corrispondente
    TLBP();
    
    // Se la voce non e' presente nel TLB, aggiorna il registro ENTRYLO e scrivilo nel TLB
    if((getINDEX() & PRESENTFLAG) == 0){
        setENTRYLO(p.pte_entryLO);
        TLBWI();
    } 
}

static void cleanDirtyPage(int sp_index){
    setSTATUS(getSTATUS() & (~IECON)); // disabilito interrupt per avere atomicità

    swap_pool_table[sp_index].sw_pte->pte_entryLO &= !VALIDON; // invalido la pagina
    //swap_pool_table[sp_index].pte_entry_lo &= !VALIDON; // invalido la pagina
    updateTLB(*(swap_pool_table[sp_index].sw_pte));

    setSTATUS(getSTATUS() | IECON); // riabilito interrupt per rilasciare l'atomicita'
}

static int RWBackingStore(int page_no, int asid, memaddr addr, int w) {
    setSTATUS(getSTATUS() & (~IECON)); // disabilito interrupt per avere mutua esclusione
    dtpreg_t *device_register = (dtpreg_t *)DEV_REG_ADDR(IL_FLASH, asid - 1);
    device_register->data0 = addr; 

    unsigned int value = w ? FLASHWRITE | (page_no << 8) : FLASHREAD | (page_no << 8);
    unsigned int status;

    ssi_do_io_t do_io = {
        .commandAddr = (unsigned int *)device_register->command,
        .commandValue = value,
    };
    ssi_payload_t payload = {
        .service_code = DOIO,
        .arg = &do_io,
    };
    
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&payload), 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&status), 0);
    setSTATUS(getSTATUS() | IECON); // riabilito interrupt per rilasciare l'atomicità
    return status;
}

static void kill_proc(){
    ssi_payload_t term_process_payload = {
        .service_code = TERMPROCESS,
        .arg = NULL,
    };

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&term_process_payload), 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, 0, 0);
}

void pager(){
    //prendo la support struct
    support_t *sup_st;
    ssi_payload_t getsup_payload = {
        .service_code = GETSUPPORTPTR,
        .arg = NULL,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&getsup_payload), 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&sup_st), 0);
    
    
    // TLB-Modification exception
    // If the Cause is a TLB-Modification exception, treat this exception as a program trap
    if(sup_st->sup_exceptState[PGFAULTEXCEPT].cause == 1){
       
        /*
            Importante: se il processo da terminare è attualmente in attesa di mutua esclusione su una struttura del livello di 
            supporto (ad esempio la tabella Swap Pool), la mutua esclusione deve essere prima rilasciata (inviare un messaggio) 
            prima di richiedere la terminazione del processo.
        */
        kill_proc();

    } else {
        // INIZIO MUTUA ESCLUSIONE
        // Gain mutual exclusion over the Swap Pool table sending a message to the swap_table PCB and waiting for a response.
        SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex_process, P, 0);
        SYSCALL(RECEIVEMESSAGE, (unsigned int)swap_mutex_process, 0, 0); //qui si blocca se value == 0
        
        // Prendo la pagina dalla entry_hi supp_p->sup_exceptState[PGFAULTEXCEPT].entry_hi
        int p = ENTRYHI_GET_VPN(sup_st->sup_exceptState[PGFAULTEXCEPT].entry_hi);
        
        // Uso il mio algoritmo di rimpiazzamento per trovare la pagina da sostituire
        int i = getPage(); //pagina vittima

        swap_t *swap_pool_entry = &swap_pool_table[i];
        memaddr victim_addr = SWAP_POOL_AREA + i * PAGESIZE;

        // è necessario aggiornare la pagina se questa era occupata da un altro frame appartenente ad un altro processo
        // e in caso serve "pulirna" poichè ormai obsoleta (ovviamente tutto ciò in modo atomico per evitare inconsistenza)
        int status;
        if(swap_pool_table[i].sw_asid != -1){
            cleanDirtyPage(i);
            //write backing store/flash
            status = RWBackingStore(swap_pool_entry->sw_pageNo ,swap_pool_entry->sw_asid, victim_addr, 1);

            if(status != 1) {
                SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex_process, V, 0);
                kill_proc(); //tratto gli errori come se fossere program trap
            }
        }
        

        //read backing store/flash
        status = RWBackingStore(p, sup_st->sup_asid, victim_addr, 0);
        if(status != 1) {
            SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex_process, V, 0);
            kill_proc(); //tratto gli errori come se fossere program trap
        }


        // 10
        //Update the Swap Pool table’s entry i to reflect frame i’s new contents: page p belonging to the
        //Current Process’s ASID, and a pointer to the Current Process’s Page Table entry for page p.
        swap_pool_entry->sw_asid = sup_st->sup_asid; 
        swap_pool_entry->sw_pageNo = p;  
        swap_pool_entry->sw_pte = &(sup_st->sup_privatePgTbl[p]);


        setSTATUS(getSTATUS() & (~IECON)); // disabilito interrupt per avere atomicita'
        // 11 Update the Current Process's Page Table entry for page p to indicate it is now present (V bit) and occupying frame i (PFN field).
        sup_st->sup_privatePgTbl[p].pte_entryLO |= VALIDON;
        sup_st->sup_privatePgTbl[p].pte_entryLO |= (victim_addr); //controllare !!!

        // 12 Update the TLB. The cached entry in the TLB for the Current Process's page p is clearly out of date; it was just updated in the previous step.
        updateTLB(sup_st->sup_privatePgTbl[p]);

        setSTATUS(getSTATUS() | IECON); // riabilito interrupt per rilasciare l'atomicita'
        
        // 13 RILASCIARE MUTUA ESCLUSIONE
        SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex_process, V, 0);
            
        //FINE MUTUA ESCLUSIONE
        LDST(&(sup_st->sup_exceptState[PGFAULTEXCEPT]));
    }
}

void uTLB_RefillHandler(){
    // prendo l'exception_state dalla BIOSDATAPAGE al fine di trovare 
    state_t* exception_state = (state_t *) BIOSDATAPAGE;
    int p = ENTRYHI_GET_VPN(exception_state -> entry_hi);

    setENTRYHI(current_process->p_supportStruct->sup_privatePgTbl[p].pte_entryHI);
    setENTRYLO(current_process->p_supportStruct->sup_privatePgTbl[p].pte_entryLO);
    
    // scrivo nel TLB
    TLBWR();        

    //Return control to the Current Process to retry the instruction that caused the TLB-Refill event:
    //LDST on the saved exception state located at the start of the BIOS Data Page.                                                                                                                                                                                                                                                                                                                                                                                             
    LDST(exception_state);
}
