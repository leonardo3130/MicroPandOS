#include "include/ssi.h"

static void blockProcessOnDevice(pcb_t* p, int line, int term){
  outProcQ(&Ready_Queue, p);
  switch (line) {
    case IL_DISK:
        insertProcQ(&Locked_disk, p);
        break;
    case IL_FLASH:
        insertProcQ(&Locked_flash, p);
        break;
    case IL_ETHERNET:
        insertProcQ(&Locked_ethernet, p);
        break;
    case IL_PRINTER:
        insertProcQ(&Locked_printer, p);
        break;
    case IL_TERMINAL:
        insertProcQ(((term > 0) ? &Locked_terminal_transm : &Locked_terminal_recv ), p);
        break;
    default:
        break;
  }
}

//funzione che dato l'indirizzo passato alla richiesta DOIO 
//determina la linea (campo device), il numero del device (campo dev_no)
//in caso di terminale setta il campo term a 0 per recv, a 1 per transm
static void addrToDevice(memaddr command_address, pcb_t *p){
    for (int j = 0; j < 8; j++){ 
        termreg_t *base_address = (termreg_t *)DEV_REG_ADDR(7, j);
        if((memaddr)&(base_address->recv_command) == command_address){
            p->dev_no = j;
            blockProcessOnDevice(p, 7, 0);
            return;
        }
        else if((memaddr)&(base_address->transm_command) == command_address){
            p->dev_no = j;
            blockProcessOnDevice(p, 7, 1);
            return;
        }
    }
    for (int i = 3; i < 7; i++)
    {
        for (int j = 0; j < 8; j++)
        { 
            dtpreg_t *base_address = (dtpreg_t *)DEV_REG_ADDR(i, j);
            if((memaddr)&(base_address->command) == command_address){
                p->dev_no = j;
                blockProcessOnDevice(p, i, -1);
                return;
            }
        }  
    }

    
}

void SSILoop(){
    while(TRUE){
        unsigned int payload;

        // attendo una richiesta da parte di un client
        unsigned int sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&payload), 0);

        // provo a soddisfare la richiesta
        unsigned int ret = SSIRequest((pcb_t *)sender, (ssi_payload_t *)payload);

        // se necessario messaggio di ritorno e/o l'operazione richiesta e' andata a buon fine
        if(ret != -1)
          SYSCALL(SENDMESSAGE, (unsigned int)sender, ret, 0);
    }
}

unsigned int ssi_new_process(ssi_create_process_t *p_info, pcb_t* parent){
    pcb_t* child = allocPcb();
    child->p_pid = pid_counter++;

    saveState(&(child->p_s), p_info->state);
    child->p_supportStruct = p_info->support;
    insertChild(parent, child);
    insertProcQ(&Ready_Queue, child);
    child->p_time = 0;

    process_count++;
    return (unsigned int)child;
}

// funzione che termina un processo e tutti i suoi figli
void ssi_terminate_process(pcb_t* proc){
    if(!(proc == NULL)){
        // ciclo in cui iterativamente si scorre la lista dei figli e ricorsivamente si guardano i fratelli
        while (!emptyChild(proc)){    
            ssi_terminate_process(removeChild(proc));
        }
    }  

    --process_count;

    // vedo se il processo si trova nella Ready, nel caso non è li significa che è 
    // bloccato e dunque decremento il contatore dei processi bloccati
    if(outProcQ(&Locked_disk, proc) != NULL){
        soft_blocked_count--;            
    }else if(outProcQ(&Locked_flash, proc) != NULL){
        soft_blocked_count--;
    }else if(outProcQ(&Locked_terminal_recv, proc) != NULL){
        soft_blocked_count--;
    }else if(outProcQ(&Locked_terminal_transm, proc) != NULL){
        soft_blocked_count--;
    }else if(outProcQ(&Locked_ethernet, proc) != NULL){
        soft_blocked_count--;
    }else if(outProcQ(&Locked_printer, proc) != NULL){
        soft_blocked_count--;
    }else if(outProcQ(&Locked_pseudo_clock, proc) != NULL){
        soft_blocked_count--;
    }
    
    outChild(proc);
    freePcb(proc);
}

void ssi_clockwait(pcb_t *sender){
    soft_blocked_count++;
    insertProcQ(&Locked_pseudo_clock, sender);
}

// funzione che ritorna il pid del sender se arg == NULL altrimenti pid del parent
int ssi_getprocessid(pcb_t *sender, void *arg){
    return (arg == NULL ? sender->p_pid : sender->p_parent->p_pid);
}

void ssi_doio(pcb_t *sender, ssi_do_io_t *doio){
    soft_blocked_count++;
    addrToDevice((memaddr)doio->commandAddr, sender);
    *(doio->commandAddr) = doio->commandValue;
}

// funzione che gestisce mediante il payload ricevuto dal sender la richiesta di un servizio
unsigned int SSIRequest(pcb_t* sender, ssi_payload_t *payload){
    unsigned int ret = 0;
    switch(payload->service_code){
        case CREATEPROCESS:
            ret = (emptyProcQ(&pcbFree_h) ? NOPROC : (unsigned int) ssi_new_process((ssi_create_process_PTR)payload->arg, sender));
            break;

        case TERMPROCESS:
            //terminates the sender process if arg is NULL, otherwise terminates arg
            if(payload->arg == NULL) {
                ssi_terminate_process(sender);
                ret = -1;
            }
            else {
                ssi_terminate_process(payload->arg);
                ret = 0; 
            }
            break;

        case DOIO:
            ssi_doio(sender, payload->arg);
            ret = -1;
            break;

        case GETTIME:
            ret = (unsigned int)sender->p_time;
            break;

        case CLOCKWAIT:
            ssi_clockwait(sender);
            ret = -1;
            break;

        case GETSUPPORTPTR:
            ret = (unsigned int)sender->p_supportStruct;
            break;

        case GETPROCESSID:
            ret = (unsigned int)ssi_getprocessid(sender, payload->arg);
            break;

        default:
            ssi_terminate_process(sender);
            ret = MSGNOGOOD;
            break;
    }
    return ret;
}
