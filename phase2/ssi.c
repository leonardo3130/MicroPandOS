#include "include/ssi.h"

void SSILoop(){
    while(TRUE){
        pcb_t *ret = allocPcb();
        if(!emptyMessageQ(&(ssi_pcb->msg_inbox))){
            msg_t *message = allocMsg(); //non sono sicuro di doverlo allocare
            message = popMessage(&(ssi_pcb->msg_inbox), NULL);
            pcb_t *sender = message->m_sender;
            ssi_payload_t *payload = message->ssi_payload;
            ret = SSIRequest(message, sender, payload);
            freeMsg(message);
        }
        else{
            ret = NULL;
        }
        //SYSCALL(SENDMESSAGE, , ret, 0);
        SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, NULL, 0);
        freePcb(ret);
    }
}

msg_t *SSIRequest(msg_t *message, pcb_t* sender, ssi_payload_t *payload){
    msg_t *ret = allocMsg();
    ret->m_sender = ssi_pcb;

    switch(payload->service_code){
        case CREATEPROCESS:
            ret->m_payload = (int)ssi_new_process(message, (ssi_create_process_PTR)payload->arg, sender);
            break;

        case TERMPROCESS:
            //terminates the sender process if arg is NULL, otherwise terminates arg
            if(payload->arg == NULL){
                ssi_terminate_process(sender);
            }
            else{
                ssi_terminate_process(payload->arg);
            }
            ret->m_payload = NULL;
            break;

        case DOIO:
            ssi_doio(sender, payload->arg);
            ret->m_payload = NULL;
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
            ret->m_payload = ssi_getprocessid(sender, payload->arg);
            break;

        default:
            ssi_terminate_process(sender);
            ret->m_payload = MSGNOGOOD;
            break;
    }
    return ret;
}

pcb_t* ssi_new_process(msg_t *message, ssi_create_process_t *p_info, pcb_t* parent){
    if(emptyProcQ(&pcbFree_h)){
        //reinserting the message at the end of the inbox
        insertMessage(&(ssi_pcb->msg_inbox), message);
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
    switch((unsigned int)doio->commandAddr){
        //non so se ogni indirizzo è associato alla lista del device corretto (ho messo le liste in odine di dichiarazione)
        case DEV0ON:
            insertProcQ(&Locked_disk, sender);
        break;
        case DEV1ON:
            insertProcQ(&Locked_flash, sender);
            break;
        case DEV2ON:
            insertProcQ(&Locked_terminal_in, sender);
            break;
        case DEV3ON:
            insertProcQ(&Locked_terminal_out, sender);
            break;
        case DEV4ON:
            insertProcQ(&Locked_ethernet, sender);
            break;
        case DEV5ON:
            insertProcQ(&Locked_printer, sender);
            break;
        case DEV6ON:
            insertProcQ(&Locked_Message, sender);
            break;
        case DEV7ON:
            insertProcQ(&Locked_pseudo_clock, sender);
            break;
        default:
            break;
    }
    doio->commandValue = doio->commandAddr; //??
}
