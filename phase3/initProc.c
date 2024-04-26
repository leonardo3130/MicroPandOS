#include "./include/initProc.h"

support_t ss_array[UPROCMAX]; //support struct array
pcb_t *swap_mutex_pcb;
swap_t swap_pool_table[POOLSIZE];
pcb_t *sst_array[UPROCMAX];
//pcb_t *terminal_pcbs_recv[UPROCMAX];
pcb_t *terminal_pcbs[UPROCMAX];
pcb_t *printer_pcbs[UPROCMAX];

state_t swap_mutex_state;
memaddr curr;

extern pcb_t *ssi_pcb;
extern pcb_t *current_process;

pcb_t *create_process(state_t *s, support_t *sup)
{
    pcb_t *p;
    ssi_create_process_t ssi_create_process = {
        .state = s,
        .support = sup,
    };
    ssi_payload_t payload = {
        .service_code = CREATEPROCESS,
        .arg = &ssi_create_process,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&p), 0);
    return p;
}

static void initSwapPoolTable() 
{
    for (i = 0; i < POOLSIZE; i++)
        swap_pool_table[i].sw_asid = -1;
}

static void initUProc()
{
    RAMTOP(curr);
    curr -= 3 * PAGESIZE; //start after test and ssi space
    for (int asid = 1; asid <= UPROCMAX; asid++) 
    {
        //inizializzazione stato
        state_t UProc_state;
        UProc_state.reg_sp = (memaddr)USERSTACKTOP;
        UProc_state.reg_pc = (memaddr)UPROCSTARTADDR;
        UProc_state.status = ALLOFF | IEPBITON | CAUSEINTMASK | TEBITON;
        UProc_state.entry_hi = asid << ASIDSHIFT;

        curr -= 2 * PAGESIZE; // general e tlb --> 2 pagine --> moltiplico per 2

        //inizializzazione support struct SST (stesse degli U-proc)
        ss_array[asid - 1].sup_asid = asid;
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].stackPtr = (memaddr)curr;
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].status = ALLOFF | IEPBITON | CAUSEINTMASK | TEBITON;
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].pc = (memaddr)generalExceptionHandler;
        ss_array[asid - 1].sup_exceptContext[PGFAULTEXCEPT].stackPtr = (memaddr)curr + PAGESIZE;
        ss_array[asid - 1].sup_exceptContext[PGFAULTEXCEPT].status = ALLOFF | IEPBITON | CAUSEINTMASK | TEBITON;
        ss_array[asid - 1].sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)TLBHandler; // nome da cambiare in base a come Andre nominerà la funzione
        
        //qualche inizializzazione con ss_array[asid].sup_privatePbTbl 
        //create_process(&SST_state,  &ss_array[asid - 1]); --> chiamata che deve fare l'SST
        //se la faccio qua gli UProc diventerebbero figli del test
    }
}

static void initSST()
{
    for (int asid = 1; asid <= UPROCMAX; asid++) 
    {
        curr -= PAGESIZE; 
        //inizializzazione stato
        state_t SST_state;
        SST_state.reg_sp = (memaddr)curr;
        SST_state.reg_pc = (memaddr)SST; //da modificare in base a come chiamerà la funzione Luca 
        SST_state.status = ALLOFF | IEPBITON | CAUSEINTMASK | TEBITON;
        SST_state.entry_hi = asid << ASIDSHIFT;

        create_process(&SST_state,  &ss_array[asid - 1]);
    }
}

static void initSwapMutex()
{
    curr -= PAGESIZE;
    swap_mutex_state.reg_sp = (memaddr)curr;
    swap_mutex_state.reg_pc = ...; //da definire
    swap_mutex_state.status = ALLOFF | IEPBITON | CAUSEINTMASK | TEBITON;

    create_process(&swap_mutex_state, NULL);
}

static void initSemProc()
{
    //creazione e inizializzazione dei processi che si comportano come semafori dei 
    //dispositivi, gestendo le rispettive richieste I/O
    for (int i = 0; i < 16; i++) {
        curr -= PAGESIZE;

        state_t p_state;
        swap_mutex_state.reg_sp = (memaddr)curr;
        swap_mutex_state.reg_pc = ...; //da definire
        swap_mutex_state.status = ALLOFF | IEPBITON | CAUSEINTMASK | TEBITON;
        
        if(i < 8)
            terminal_pcbs[i] = create_process(&p_state, NULL);
        else
            printer_pcbs[i - 8] = create_process(&p_state, NULL);
    } 
}

//funzione che verrà eseguita dal processo inizializzato in fase 2
void test() 
{
    pcb_t *test = current_process;
    initSwapPoolTable();    //Swap Pool init
    initUProc();            //init U-procs
    initSST();              //init and create SSTs 
    initSwapMutex();        //init and create swap mutex process
    initSemProc();          //init and create devices semaphore processes
}
