/*
 * Questo file contiene l'implementazione delle funzioni utilizzate per l'inizializzazione dei processi
 * nel sistema operativo PandOS. Le funzioni inizializzano le strutture dati necessarie per la gestione
 * dei processi, creano i processi semaforo per i dispositivi e inizializzano le tabelle di pagine.
 */

#include "./include/initProc.h"
#include "../klog.c"

support_t ss_array[UPROCMAX]; // array di strutture di supporto
state_t UProc_state[UPROCMAX]; // array di stati dei processi utente
pcb_t *swap_mutex_pcb; // puntatore al processo mutex per lo swap
swap_t swap_pool_table[POOLSIZE]; // tabella di swap
pcb_t *sst_array[UPROCMAX]; // array di puntatori ai processi SST
pcb_t *terminal_pcbs[UPROCMAX]; // array di puntatori ai processi terminali
pcb_t *printer_pcbs[UPROCMAX]; // array di puntatori ai processi stampanti
pcb_t *swap_mutex_process; // puntatore al processo mutex per lo swap
pcb_t *test_pcb; // puntatore al processo di test

state_t swap_mutex_state; // stato del processo mutex per lo swap
memaddr curr; // indirizzo corrente

extern pcb_t *ssi_pcb; // puntatore al processo SSI
extern pcb_t *current_process; // puntatore al processo corrente

/*
 * Funzione per la creazione di un processo.
 * Riceve come argomenti lo stato del processo e la struttura di supporto.
 * Restituisce il puntatore al processo creato.
 */
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

    //klog_print_hex((unsigned int)p);
    //klog_print("\n");
    return p;
}

/*
 * Funzione per la gestione del mutex per lo swap.
 * Riceve messaggi dai processi che vogliono la mutua esclusione e la concede
 * (ad un processo alla volta, ovviamente) aspettando che ognuno la rilasci dopo averla ottenuta.
 */
void swapMutex(){
    for(;;) {
        unsigned int sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
        SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
        SYSCALL(RECEIVEMESSAGE, sender, 0, 0);
    }
}

/*
 * Funzione per l'inizializzazione della tabella di swap.
 * Imposta il campo sw_asid di ogni entry a -1.
 */
static void initSwapPoolTable() 
{
    for (int i = 0; i < POOLSIZE; i++)
    {
        swap_pool_table[i].sw_asid = NOPROC;
    }
}

/*
 * Funzione per l'inizializzazione di una entry nella tabella delle pagine.
 * Riceve come argomenti un puntatore all'entry, l'asid e l'indice.
 * Imposta i campi pte_entryHI e pte_entryLO dell'entry in base all'asid e all'indice.
 */
static void initPageTableEntry(pteEntry_t *entry, int asid, int i) {
    if(i < 31)
        entry->pte_entryHI = KUSEG + (i << VPNSHIFT) + (asid << ASIDSHIFT);
    else
        entry->pte_entryHI = 0xBFFFF000 + (asid << ASIDSHIFT); //stack page 
    entry->pte_entryLO = DIRTYON;
}

/*
 * Funzione per l'inizializzazione dei processi utente.
 * Inizializza gli stati dei processi, le strutture di supporto e le tabelle delle pagine.
 */
static void initUProc()
{
    RAMTOP(curr);
    curr -= 3 * PAGESIZE; // inizia dopo lo spazio per test e ssi
    for (int asid = 1; asid <= UPROCMAX; asid++) 
    {
        // inizializzazione stato
        UProc_state[asid - 1].reg_sp = (memaddr)USERSTACKTOP;
        UProc_state[asid - 1].pc_epc = (memaddr)UPROCSTARTADDR;
        UProc_state[asid - 1].status = ALLOFF | IEPON | IMON | USERPON | TEBITON;
        UProc_state[asid - 1].reg_t9 = (memaddr)UPROCSTARTADDR;
        UProc_state[asid - 1].entry_hi = asid << ASIDSHIFT;

        // inizializzazione strutture di supporto SST
        ss_array[asid - 1].sup_asid = asid;
        ss_array[asid - 1].sup_exceptContext[PGFAULTEXCEPT].stackPtr = (memaddr) curr;
        ss_array[asid - 1].sup_exceptContext[PGFAULTEXCEPT].status = ALLOFF | IEPON | IMON | TEBITON;
        ss_array[asid - 1].sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr) pager;
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].stackPtr = (memaddr) (curr - PAGESIZE);
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].status = ALLOFF | IEPON | IMON | TEBITON;
        ss_array[asid - 1].sup_exceptContext[GENERALEXCEPT].pc = (memaddr)generalExceptionHandler;
        // klog_print_hex(curr);
        // klog_print("\n");

        curr -= 2 * PAGESIZE; // general e tlb --> 2 pagine

        // inizializzazione tabella delle pagine del processo
        for (int i = 0; i < USERPGTBLSIZE; i++)  
            initPageTableEntry(&(ss_array[asid - 1].sup_privatePgTbl[i]), asid, i); 
    }
}

/*
 * Funzione per l'inizializzazione dei processi SST.
 * Crea i processi SST e inizializza i loro stati.
 */
static void initSST()
{
    state_t SST_state[UPROCMAX];
    for (int asid = 1; asid <= UPROCMAX; asid++) 
    {
        SST_state[asid - 1].reg_sp = (memaddr)curr;
        SST_state[asid - 1].pc_epc = (memaddr)SST_loop;
        SST_state[asid - 1].status = ALLOFF | IEPON | IMON | TEBITON;
        SST_state[asid - 1].entry_hi = asid << ASIDSHIFT;
        sst_array[asid - 1] = create_process(&SST_state[asid - 1],  &ss_array[asid - 1]); // pid 6

        // klog_print_hex((memaddr) curr);
        // klog_print("\n");

        curr -= PAGESIZE;
    }
}

/*
 * Funzione per l'inizializzazione del processo mutex per lo swap.
 * Crea il processo mutex per lo swap e inizializza il suo stato.
 */
static void initSwapMutex()
{
    swap_mutex_state.reg_sp = (memaddr)curr;
    swap_mutex_state.pc_epc = (memaddr)swapMutex;
    swap_mutex_state.status = ALLOFF | IEPON | IMON | TEBITON;
    
    // klog_print_hex((memaddr) curr);
    // klog_print("\n");
    
    curr -= PAGESIZE;
    swap_mutex_process = create_process(&swap_mutex_state, NULL);   // pid 3
}

//void br(){}


/*
 * Funzione per la stampa di una stringa su un dispositivo specificato.
 * Riceve come argomenti il numero del dispositivo e l'indirizzo di base.
 * Riceve messaggi contenenti le stringhe da stampare e invia messaggi di risposta.
 */
void print(int device_number, unsigned int *base_address)
{
    while (1)
    {
        char *s;
        pcb_t *sender;
        sender = (unsigned int) SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&s), 0);
        
        klog_print(" to print\n");
        
        unsigned int *base = base_address + 4 * device_number;
        unsigned int *command;
        if(base_address == (unsigned int *)TERM0ADDR)
            command = base + 3;
        else
            command = base + 1;
        unsigned int *data0 = base + 2;
        unsigned int status;
        
        while (*s != EOS)       // stampo finchè non incontro il terminatore della stringa
        {    
            unsigned int value;
            if(base_address == (unsigned int *)TERM0ADDR)
                value = PRINTCHR | (((unsigned int)*s) << 8);
            else {
                value = PRINTCHR;
                *data0 = (unsigned int)*s;
            }

            // mando la DOIO a SSI per fare la print di un singolo char
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

            ++s;    //passo al char successivo
        }

        
        SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
        
        // klog_print_hex((unsigned int)current_process);
        // klog_print("\n");
        // klog_print_hex((unsigned int)sender) ;
        // klog_print(" Ok send \n");
    }
}

/*
 * Wrapper delle funzioni di stampa per poterle assegnare ai program counter dei vari device.
 */
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

/*
 * Array di puntatori ai wrapper delle funzioni di stampa per una maggiore comodità durante l'assegnamento al program counter.
 */
void (*terminals[8]) () = {print_term0, print_term1, print_term2, print_term3, print_term4, print_term5, print_term6, print_term7};
void (*printers[8]) () = {printer0, printer1, printer2, printer3, printer4, printer5, printer6, printer7};


/*
 *Funzione che inizializza i processi per i dispositivi, 
 *ricevono le richieste di operazioni DOIO dagli SST e le eseguono sui 
 *rispettivi device
*/
static void initDevProc()
{
    state_t dev_state[UPROCMAX * 2];

    for (int i = 0; i < UPROCMAX * 2; i++) {
        dev_state[i].reg_sp = (memaddr)curr;
        
        if(i < UPROCMAX) {
            dev_state[i].pc_epc = (unsigned int)terminals[i]; 
            dev_state[i].entry_hi = (i + 1) << ASIDSHIFT;
        } else {
            dev_state[i].pc_epc = (unsigned int)printers[i - UPROCMAX]; 
            dev_state[i].entry_hi = (i - UPROCMAX + 1) << ASIDSHIFT;
        }
        
        dev_state[i].status = ALLOFF | IEPON | IMON | TEBITON;
        
        if(i < UPROCMAX)
            terminal_pcbs[i] = create_process(&dev_state[i], &ss_array[i]);                         // pid 4
            //terminal_pcbs[i] = create_process(&dev_state[i], NULL);
        else
            printer_pcbs[i - UPROCMAX] = create_process(&dev_state[i], &ss_array[i - UPROCMAX]);    // pid 5
            //printer_pcbs[i - UPROCMAX] = create_process(&dev_state[i], NULL);
        // klog_print_hex((memaddr) curr);
        // klog_print("\n");
        curr -= PAGESIZE;
    } 
}

/*
 * Funzione di test che viene eseguita dal processo inizializzato in fase 2.
 * Inizializza la tabella di swap, i processi utente, il processo mutex per lo swap,
 * i processi semaforo per i dispositivi e i processi SST.
 * Infine, aspetta che tutti i processi SST e utente terminino e termina il processo di test 
 * mandando il sistema in HALT (in caso di corretta terminazione).
 */
void test() 
{
    test_pcb = current_process; 

    // inizializzo la tabella di swap
    initSwapPoolTable();

    // inizializzo i processi utente
    initUProc();

    // inizializzo il processo mutex per la muta esclusione
    initSwapMutex();

    // inizializzo i processi per i dispositivi
    initDevProc();

    // inizializzo i processi SST
    initSST();

    // klog_print("test: ");
    // klog_print_dec(test_pcb->p_pid);
    // klog_print("\n");                            pid 2

    // klog_print("mutex: ");
    // klog_print_dec(swap_mutex_process->p_pid);
    // klog_print("\n");                            pid 3

    // klog_print("terminal: ");
    // klog_print_dec(terminal_pcbs[0]->p_pid);
    // klog_print("\n");                            pid 4

    // klog_print("printer: ");
    // klog_print_dec(printer_pcbs[0]->p_pid);
    // klog_print("\n");                            pid 5
    
    // klog_print("sst: ");
    // klog_print_dec(sst_array[0]->p_pid);
    // klog_print("\n");                            pid 6

    // attendo la terminazione di tutti i processi SST e utente
    for (int i = 0; i < UPROCMAX; i++)
        SYSCALL(RECEIVEMESSAGE, (unsigned int)sst_array[i], 0, 0);

    klog_print("\nfine1");

    // chiedo al processo SSI la terminazione del processo di test
    ssi_payload_t payload = {
        .service_code = TERMPROCESS,
        .arg = NULL,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, 0, 0);

    klog_print("\nfine");
    // Qui i processi test, swap mutex, dispositivi, SST e utente non dovrebbero più esistere
    // Rimane solo il processo SSI in stato di HALT
}
