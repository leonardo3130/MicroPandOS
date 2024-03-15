#include "include/ssi.h"

void SSILoop(){
    while(TRUE){
        ssi_payload_t *payload;
        pcb_t *sender = (RECEIVEMESSAGE, ANYMESSAGE, payload, 0);
        
        //utilizzo la struct ssi payload come ritorno (il dato richiesto sarà salvato in arg)
        ssi_payload_t *ret = malloc(sizeof(ssi_payload_t));
        ret = SSIRequest(sender, payload);
        if(payload->service_code != CLOCKWAIT && payload->service_code != DOIO){
            SYSCALL(SENDMESSAGE, ssi_pcb, ret, 0);
        }
    }
}

ssi_payload_t *SSIRequest(pcb_t* sender, ssi_payload_t *payload){
    ssi_payload_t *ret;
    switch(payload->service_code){
        case CREATEPROCESS:
            ret->arg = ssi_new_process((ssi_create_process_PTR)payload->arg, sender);
            break;

        case TERMPROCESS:
            //terminates the sender process if arg is NULL, otherwise terminates arg
            if(payload->arg == NULL){
                ssi_terminate_process(sender);
            }
            else{
                ssi_terminate_process(payload->arg);
            }
            ret->arg = NULL;
            break;

        case DOIO:
            ssi_doio(sender, payload->arg);
            ret->arg = NULL;
            break;

        case GETTIME:
            ret->arg = (int)sender->p_time;
            break;

        case CLOCKWAIT:
            ssi_clockwait(sender);
            ret->arg = NULL;
            break;

        case GETSUPPORTPTR:
            ret->arg = (int)sender->p_supportStruct;
            break;

        case GETPROCESSID:
            ret->arg = ssi_getprocessid(sender, payload->arg);
            break;

        default:
            ssi_terminate_process(sender);
            ret->arg = MSGNOGOOD;
            break;
    }
    return ret;
}

pcb_t* ssi_new_process(ssi_create_process_t *p_info, pcb_t* parent){
    if(emptyProcQ(&pcbFree_h)){
        return NOPROC;
    }

    pcb_t* child = allocPcb();
    child->p_pid = pid_counter++;

    child->p_s = *(p_info->state);
    child->p_supportStruct = p_info->support;
    insertChild(parent, child);
    insertProcQ(&Ready_Queue, child);
    child->p_time = 0;

    current_process->p_s.pc_epc ++;
    return child;
}

//recursive function which terminates proc and all its progeny
void ssi_terminate_process(pcb_t* proc){
    if(emptyChild(proc)){
        outChild(proc);
        freePcb(proc);
    }
    else{
        struct list_head *iter;
        list_for_each(iter, &proc->p_child){
            ssi_terminate_process(container_of(iter, pcb_t, p_child));
        }
        ssi_terminate_process(proc);
    }
}

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

}
