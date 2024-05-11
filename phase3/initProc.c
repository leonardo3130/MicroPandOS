#include "./include/initProc.h"

support_t ss_array[UPROCMAX]; //support struct array
state_t UProc_state[UPROCMAX];
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
    for (int i = 0; i < POOLSIZE; i++)
    {
        swap_pool_table[i].sw_asid = -1;
        // swap_pool_table[i].count = 0;
    }
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
        UProc_state[asid - 1].status = ALLOFF | IEPON | IMON | TEBITON;
        UProc_state[asid - 1].entry_hi = asid << ASIDSHIFT;

        curr -= 2 * PAGESIZE; // general e tlb --> 2 pagine --> moltiplico per 2

        //inizializzazione support struct SST (stesse degli U-proc)
        ss_array[asid - 1].sup_asid = asid;
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].stackPtr = (memaddr)curr;
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].status = ALLOFF | IEPON | IMON | TEBITON;
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].pc = (memaddr)generalExceptionHandler;
        ss_array[asid - 1].sup_exceptContext[PGFAULTEXCEPT].stackPtr = (memaddr)curr + PAGESIZE;
        ss_array[asid - 1].sup_exceptContext[PGFAULTEXCEPT].status = ALLOFF | IEPON | IMON | TEBITON;
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
    swap_mutex_state.pc_epc = ...; //da definire
    swap_mutex_state.status = ALLOFF | IEPON | IMON | TEBITON;

    create_process(&swap_mutex_state, NULL);
}


void print(int device_number, unsigned int *base_address)
{
    while (1)
    {
        char *msg;
        unsigned int sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&msg), 0);
        char *s = msg;
        unsigned int *base0 = base_addres; //indirizzo base termiale 0 o stampante 0
        //basen =base0 + 4 * n
        unsigned int *command = base0 + 4 * device_number + 3;
        unsigned int status;
        
        while (*s != EOS)
        {
            unsigned int value = PRINTCHR | (((unsigned int)*s) << 8);
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
            
            if ((status & TERMSTATMASK) != RECVD)
                PANIC();
            s++;
        }
        
        SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
    }
}

void print_term0 { print(0, (unsigned int *)TERM0ADDR); }
void print_term1 { print(1, (unsigned int *)TERM0ADDR); }
void print_term2 { print(2, (unsigned int *)TERM0ADDR); }
void print_term3 { print(3, (unsigned int *)TERM0ADDR); }
void print_term4 { print(4, (unsigned int *)TERM0ADDR); }
void print_term5 { print(5, (unsigned int *)TERM0ADDR); }
void print_term6 { print(6, (unsigned int *)TERM0ADDR); }
void print_term7 { print(7, (unsigned int *)TERM0ADDR); }

void printer0 { print(0, (unsigned int *)PRINTER0ADDR); }
void printer1 { print(1, (unsigned int *)PRINTER0ADDR); }
void printer2 { print(2, (unsigned int *)PRINTER0ADDR); }
void printer3 { print(3, (unsigned int *)PRINTER0ADDR); }
void printer4 { print(4, (unsigned int *)PRINTER0ADDR); }
void printer5 { print(5, (unsigned int *)PRINTER0ADDR); }
void printer6 { print(6, (unsigned int *)PRINTER0ADDR); }
void printer7 { print(7, (unsigned int *)PRINTER0ADDR); }


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
            p_state.pc_epc = terminals[i]; 
        else
            p_state.pc_epc = printers[i - 8]; 
        
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
    pcb_t *test = current_process;
    initSwapPoolTable();    //Swap Pool init
    initUProc();            //init U-procs
    initSwapMutex();        //init and create swap mutex process
    initSemProc();          //init and create devices semaphore processes
    initSST();              //init and create SSTs 
}
