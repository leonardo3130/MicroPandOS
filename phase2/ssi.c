#include "include/ssi.h"

void SSIRequest(pcb_t* sender, int service, void* arg){
    msg_t *ret = allocMsg();
    ret->m_sender = ssi_pcb;
    
    switch(service){
        case CREATEPROCESS:
            ret->m_payload = (int)ssi_new_process(arg->body, arg->message->m_sender);
            break;

        case TERMPROCESS:
            //terminates the sender process if arg is NULL, otherwise terminates arg
            if(arg == NULL){
                ret->m_payload = (int)ssi_terminate_process(arg->message->m_sender);
            }
            else{
                ret->m_payload = (int)ssi_terminate_process(arg->body);
            }
            break;

        case DOIO:
            break;

        case GETTIME:
            ret->m_payload = (int)sender->p_time;
            break;

        case CLOCKWAIT:
            ssi_clockwait(sender);
            ret->m_payload = NULL;
            break;

        case GETSUPPORTPTR:
            ret->m_payload = (int)sender->p_supportStruct;
            break;

        case GETPROCESSID:
            ret->m_payload = ssi_getprocessid(sender, arg);
            break;

        default:
            ret->m_payload = MSGNOGOOD;
            break;
    }

    //syscall con valori di ritorno
}

pcb_t* ssi_new_process(ssi_create_process_t p_info, pcb_t* parent){
    //if lack of resources (e.g. no more free PBCs)
    //return NOPROC

    pcb_t* child = allocPcb();
    child->p_pid = pid_counter++;
    
    child->p_s = p_info->state;
    child->p_supportStruct = p_info->support;
    insertChild(parent, child);
    insertProcQ(Ready_Queue, child);
    child->p_time = 0;

    Current_Process->p_s->pc_epc ++;
    return child;
}

pcb_t ssi_terminate_process(pcb_t* proc){
    if(proc == NULL){
        return NULL
    }
    else{
        pcb_t tmp = removeChild(proc);
        removeProcQ(tmp);
        return ssi_terminate_process(head, tmp);
    }
}

void ssi_clockwait(pcb_t *sender){
    insertProcQ(Locked_pseudo_clock, arg->message->m_sender);
}

int ssi_getprocessid(pcb_t *sender, void *arg){
    if((int)arg == 0){
        return sender->p_pid;
    }
    else{
        return sender->p_parent->p_pid;
    }   
}