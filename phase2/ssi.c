#include "include/ssi.h"

void SSILoop(){
    while(!emptyMessageQ(ssi_pcb->msg_inbox)){
        msg_t message = allocMsg();
        message->sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, ssi_pcb->p_s->gpr[5], 0);
        message->ssi_payload = (ssi_payload_t)ssi_pcb->p_s->gpr[5];
        msg_t *ret = SSIRequest(sender, message->ssi_payload->service_code, message->ssi_payload->arg);
        SYSCALL(SENDMESSAGE,    , ret, 0);
    }
}

msg_t *SSIRequest(pcb_t* sender, int service, void* arg){
    msg_t *ret = allocMsg();
    ret->m_sender = ssi_pcb;
    
    switch(service){
        case CREATEPROCESS:
            ret->m_payload = (int)ssi_new_process(arg->body, arg->message->m_sender);
            break;

        case TERMPROCESS:
            //terminates the sender process if arg is NULL, otherwise terminates arg
            if(arg == NULL){
                ssi_terminate_process(sender);
            }
            else{
                ssi_terminate_process(arg);
            }
            ret->m_payload = NULL;
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
    return ret;
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

//recursive function which terminates proc and all its progeny
void ssi_terminate_process(pcb_t* proc){
    if(emptyProcQ(proc->p_child)){
        outProcQ(/*lista processi*/, proc);
        freePcb(proc);
    }
    else{
        struct list_head *iter;
        list_for_each(iter, proc->p_child){
            ssi_terminate_process(container_of(iter, pcb_t, proc->p_child));
        }
        ssi_terminate_process(proc);
    }
}

void ssi_clockwait(pcb_t *sender){
    insertProcQ(Locked_pseudo_clock, sender);
}

int ssi_getprocessid(pcb_t *sender, void *arg){
    if((int)arg == 0){
        return sender->p_pid;
    }
    else{
        return sender->p_parent->p_pid;
    }   
}

