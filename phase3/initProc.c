#include "./include/initProc.h"
#include "../klog.c"

support_t ss_array[UPROCMAX]; //support struct array
state_t UProc_state[UPROCMAX];
pcb_t *swap_mutex_pcb;
swap_t swap_pool_table[POOLSIZE];
pcb_t *sst_array[UPROCMAX];
pcb_t *terminal_pcbs[UPROCMAX];
pcb_t *printer_pcbs[UPROCMAX];
pcb_t *swap_mutex_process;

LIST_HEAD(locked_proc_mutex);

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

void swapMutex(){
    int value = 1; //valore del semaforo
    for(;;) {
        // ricezione richiesta (conterra' il valore P o V)
        unsigned int payload;

        //attesa di una richiesta da parte di un client attraverso SYS2
        unsigned int sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&payload), 0);

        if (payload == P)
        {
            if(value >= 1){     // semaforo non binario
                value--;
                //mando il messaggio per sbloccare il processo
                SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
            }else if (value == 0){
                insertProcQ(&locked_proc_mutex, (pcb_t *)sender);
            }
        }
        else{
            if(value == 0){
                value++;
                if(!emptyProcQ(&locked_proc_mutex)){
                    //sblocco processo
                    pcb_t *unblocked_pcb = removeProcQ(&locked_proc_mutex);
                    //mando messaggio
                    SYSCALL(SENDMESSAGE, (unsigned int) unblocked_pcb, 0, 0);
                }
            }
        }
        //se P and val == 0 --> blocco processo su lista 
        //se P and val == 1 --> v--, messaggio al processo per sbloccarlo --> ha la mutua esclusione
        //se V and val == 0 --> v++, se la coda dei bloccati NON è vuota
        //prendo il primo processo in attesa e lo sblocco inviando un messaggio
        //se V and val == 1 --> non gestito, se succede c'è qualche errore       
    }
}

static void initSwapPoolTable() 
{
    for (int i = 0; i < POOLSIZE; i++)
    {
        swap_pool_table[i].sw_asid = -1;
        // swap_pool_table[i].count = 0;
    }
}

static void initPageTableEntry(pteEntry_t *entry, int asid, int i) {
    if(i < 31)
        entry->pte_entryHI = KUSEG + (i << VPNSHIFT) + (asid << ASIDSHIFT);
    else
        entry->pte_entryHI = 0xBFFFF000 + (asid << ASIDSHIFT); //stack page 
    entry->pte_entryLO = DIRTYON;
}

static void initUProc()
{
    RAMTOP(curr);
    curr -= 3 * PAGESIZE; //start after test and ssi space
    for (int asid = 1; asid <= UPROCMAX; asid++) 
    {
        //inizializzazione stato
        UProc_state[asid - 1].reg_sp = (memaddr)USERSTACKTOP;
        UProc_state[asid - 1].pc_epc = (memaddr)UPROCSTARTADDR;
        UProc_state[asid - 1].reg_t9 = (memaddr)UPROCSTARTADDR;
        UProc_state[asid - 1].status = ALLOFF | USERPON | IEPON | IMON | TEBITON;
        UProc_state[asid - 1].entry_hi = asid << ASIDSHIFT;

        curr -= 2 * PAGESIZE; // general e tlb --> 2 pagine --> moltiplico per 2

        //inizializzazione support struct SST (stesse degli U-proc)
        ss_array[asid - 1].sup_asid = asid;
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].stackPtr = (memaddr)curr;
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].status = ALLOFF | IEPON | IMON | TEBITON;
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].pc = (memaddr)generalExceptionHandler;
        ss_array[asid - 1].sup_exceptContext[PGFAULTEXCEPT].stackPtr = (memaddr)curr + PAGESIZE;
        ss_array[asid - 1].sup_exceptContext[PGFAULTEXCEPT].status = ALLOFF | IEPON | IMON | TEBITON;
        ss_array[asid - 1].sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)pager; // nome da cambiare in base a come Andre nominerà la funzione
        
        //inizializzazione page table del processo
        for (int i = 0; i < USERPGTBLSIZE; i++)  
            initPageTableEntry(&(ss_array[asid - 1].sup_privatePgTbl[i]), asid, i); 
        //create_process(&SST_state,  &ss_array[asid - 1]); --> chiamata che deve fare l'SST
        //se la faccio qua gli UProc diventerebbero figli del test
    }
}

static void initSST()
{
    for (int asid = 1; asid <= UPROCMAX; asid++) 
    {
        curr -= PAGESIZE; 
        // inizializzazione stato
        state_t SST_state;
        SST_state.reg_sp = (memaddr)curr;
        SST_state.pc_epc = (memaddr)SST_loop;
        SST_state.status = ALLOFF | IEPON | IMON | TEBITON;
        SST_state.entry_hi = asid << ASIDSHIFT;

        sst_array[asid-1] = create_process(&SST_state,  &ss_array[asid - 1]);
    }
}

static void initSwapMutex()
{
    curr -= PAGESIZE;
    swap_mutex_state.reg_sp = (memaddr)curr;
    swap_mutex_state.pc_epc = (memaddr)swapMutex;
    swap_mutex_state.status = ALLOFF | IEPON | IMON | TEBITON;

    swap_mutex_process = create_process(&swap_mutex_state, NULL);
}

//funzione che riceve una stringa e la stampa sul device specificato 
//dai 2 parametri
void print(int device_number, unsigned int *base_address)
{
    while (1)
    {
        char *msg;
        unsigned int sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&msg), 0);
        char *s = msg;
        unsigned int *base = base_address + 4 * device_number; //indirizzo base
        //basen =base0 + 4 * n
        unsigned int *command;
        if(base_address == (unsigned int *)TERM0ADDR)
            command = base + 3;
        else
            command = base + 1;
        unsigned int *data0 = base + 2; //usato solo con stampanti
        unsigned int status;
        
        while (*s != EOS)
        {    
            unsigned int value;
            if(base_address == (unsigned int *)TERM0ADDR)
                value = PRINTCHR | (((unsigned int)*s) << 8);
            else {
                value = PRINTCHR; //con le stampanti il valore va nel registro DATA0, non in command
                *data0 = (unsigned int)*s;
            }

            ssi_do_io_t do_io = {
                .commandAddr = command,
                .commandValue = value,
            };
            ssi_payload_t payload = {
                .service_code = DOIO,
                .arg = &do_io,
            };
            SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&payload), 0);
            SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&status), 0);
            
            if (base_address == (unsigned int *)TERM0ADDR && (status & TERMSTATMASK) != RECVD)
                PANIC();
            if (base_address == (unsigned int *)PRINTER0ADDR && status != READY)
                PANIC();
            s++;
        }
        
        SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
    }
}

//wrapper della funzione per poterla assegnare ai 
//program counter dei processi semafori

void print_term0 () { print(0, (unsigned int *)TERM0ADDR); }
void print_term1 () { print(1, (unsigned int *)TERM0ADDR); }
void print_term2 () { print(2, (unsigned int *)TERM0ADDR); }
void print_term3 () { print(3, (unsigned int *)TERM0ADDR); }
void print_term4 () { print(4, (unsigned int *)TERM0ADDR); }
void print_term5 () { print(5, (unsigned int *)TERM0ADDR); }
void print_term6 () { print(6, (unsigned int *)TERM0ADDR); }
void print_term7 () { print(7, (unsigned int *)TERM0ADDR); }

void printer0 () { print(0, (unsigned int *)PRINTER0ADDR); }
void printer1 () { print(1, (unsigned int *)PRINTER0ADDR); }
void printer2 () { print(2, (unsigned int *)PRINTER0ADDR); }
void printer3 () { print(3, (unsigned int *)PRINTER0ADDR); }
void printer4 () { print(4, (unsigned int *)PRINTER0ADDR); }
void printer5 () { print(5, (unsigned int *)PRINTER0ADDR); }
void printer6 () { print(6, (unsigned int *)PRINTER0ADDR); }
void printer7 () { print(7, (unsigned int *)PRINTER0ADDR); }

//array di puntatori ai wrapper soprastanti per 
//una maggiore comodità durante l'assegnamento al program counter

void (*terminals[8]) () = {print_term0, print_term1, print_term2, print_term3, print_term4, print_term5, print_term6, print_term7};
void (*printers[8]) () = {printer0, printer1, printer2, printer3, printer4, printer5, printer6, printer7};

static void initSemProc()
{
    //creazione e inizializzazione dei processi che si comportano come semafori dei 
    //dispositivi, gestendo le rispettive richieste I/O
    for (int i = 0; i < 16; i++) {
        curr -= PAGESIZE;

        state_t p_state;
        p_state.reg_sp = (memaddr)curr;
        
        if(i < 8)
            p_state.pc_epc = (unsigned int)terminals[i]; 
        else
            p_state.pc_epc = (unsigned int)printers[i - 8]; 
        
        p_state.status = ALLOFF | IEPON | IMON | TEBITON;
        
        if(i < 8)
            terminal_pcbs[i] = create_process(&p_state, NULL);
        else
            printer_pcbs[i - 8] = create_process(&p_state, NULL);
    } 
}

//funzione che verrà eseguita dal processo inizializzato in fase 2
void test() 
{
    initSwapPoolTable();    //Swap Pool init
    initUProc();            //init U-procs
    initSwapMutex();        //init and create swap mutex process
    initSemProc();          //init and create devices semaphore processes
    initSST();              //init and create SSTs
}
