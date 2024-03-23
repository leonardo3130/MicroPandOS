#include "include/ssi.h"

void SSILoop(){
    while(TRUE){
        unsigned int payload;
        unsigned int sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&payload), 0);

        unsigned int ret = SSIRequest((pcb_t *)sender, (ssi_payload_t *)payload);
        if( ((ssi_payload_t *)payload)->service_code != CLOCKWAIT &&
            ((ssi_payload_t *)payload)->service_code != DOIO &&
            ((ssi_payload_t *)payload)->service_code != TERMPROCESS ){
                SYSCALL(SENDMESSAGE, (unsigned int)sender, ret, 0);
        }
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
    if(!outProcQ(&Ready_Queue, proc))
        --soft_blocked_count;
    
    outChild(proc);
    freePcb(proc);
}

/*void term_p(pcb_t *proc){
    outChild(proc);
    // prima di fare la free controllo che il processo in questione non sia bloccato in nessuna lista dei processi bloccati
    // e in caso verrà levato e verrà decrementato il contatore dei processi bloccati
    if(outProcQ(&Locked_disk, proc) != NULL){
        soft_blocked_count--;            
    }else if(outProcQ(&Locked_flash, proc) != NULL){
        soft_blocked_count--;
    }else if(outProcQ(&Locked_terminal_in, proc) != NULL){
        soft_blocked_count--;
    }else if(outProcQ(&Locked_terminal_out, proc) != NULL){
        soft_blocked_count--;
    }else if(outProcQ(&Locked_ethernet, proc) != NULL){
        soft_blocked_count--;
    }else if(outProcQ(&Locked_printer, proc) != NULL){
        soft_blocked_count--;
    }else if(outProcQ(&Locked_Message, proc) != NULL){
        soft_blocked_count--;
    }else if(outProcQ(&Locked_pseudo_clock, proc) != NULL){
        soft_blocked_count--;
    }
    
    freePcb(proc);
}*/

void ssi_clockwait(pcb_t *sender){
    insertProcQ(&Locked_pseudo_clock, sender);
}

int ssi_getprocessid(pcb_t *sender, void *arg){
    if((int)arg == 0){
        return sender->p_pid;
    }
    else{
        return sender->p_parent->p_pid;
    }
}

void ssi_doio(pcb_t *sender, ssi_do_io_t *doio){
    *(doio->commandAddr) = doio->commandValue;
}

unsigned int SSIRequest(pcb_t* sender, ssi_payload_t *payload){
    unsigned int ret = 0;
    switch(payload->service_code){
        case CREATEPROCESS:
            if(emptyProcQ(&pcbFree_h)){
                return NOPROC;
            }
            ret = (unsigned int) ssi_new_process((ssi_create_process_PTR)payload->arg, sender);
            break;

        case TERMPROCESS:
            //terminates the sender process if arg is NULL, otherwise terminates arg
            if(payload->arg == NULL){
                ssi_terminate_process(sender, 0);
            }else{
                ssi_terminate_process(payload->arg, 0);
            }
            break;

        case DOIO:
            ssi_doio(sender, payload->arg);
            break;

        case GETTIME:
            ret = (unsigned int)sender->p_time;
            break;

        case CLOCKWAIT:
            ssi_clockwait(sender);
            break;

        case GETSUPPORTPTR:
            ret = (unsigned int)sender->p_supportStruct;
            break;

        case GETPROCESSID:
            ret = (unsigned int)ssi_getprocessid(sender, payload->arg);
            break;

        default:
            ssi_terminate_process(sender, 0);
            ret = MSGNOGOOD;
            break;
    }
    return ret;
}
