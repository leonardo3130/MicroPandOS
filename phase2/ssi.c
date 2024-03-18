#include "include/ssi.h"

void SSILoop(){
    while(TRUE){
        unsigned int payload;
        unsigned int sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, payload, 0);
        unsigned int ret = SSIRequest((pcb_t *)sender, (ssi_payload_t *)payload);
        if( ((ssi_payload_t *)payload)->service_code != CLOCKWAIT &&
            ((ssi_payload_t *)payload)->service_code != DOIO &&
            ((ssi_payload_t *)payload)->service_code != TERMPROCESS ){
                SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, ret, 0);
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
                ssi_terminate_process(sender);
            }
            else{
                ssi_terminate_process(payload->arg);
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
            ssi_terminate_process(sender);
            ret = MSGNOGOOD;
            break;
    }
    return ret;
}
