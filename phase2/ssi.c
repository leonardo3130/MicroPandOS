#include "include/ssi.h"


void SSIRequest(pcb_t* sender, int service, void* arg){
    msg_t* request = new msg_t;
    request->m_sender = sender;
    request->m_payload = service;
    insertMessage(ssi_msg_queue, request);

    sys_arg_ptr tmp;
    tmp->message = request;
    tmp->body = arg;

    sender->p_s->gpr[5] = tmp;
	SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, tmp, 0);
    //sender->p_s->gpr[1] registro con valore di ritorno della syscall

}

void* SSIRequest_handler(sys_arg_ptr arg){
    switch(arg->message->m_payload){
        case CREATEPROCESS:
            ssi_create_process_PTR process = new ssi_create_process_t;

            //casting to ssi_create_process_t
            //process_info = ssi_create_process_t(arg->body);

            return ssi_new_process(arg->body, arg->message->m_sender);

        case TERMPROCESS:
            //terminates the sender process if arg is NULL, otherwise terminates arg
            if(arg->body == NULL){
                return ssi_terminate_process(arg->message->m_sender);
            }
            else{
                return ssi_terminate_process(arg->body);
            }

        case DOIO:
            break;
        case GETTIME:
            break;
        case CLOCKWAIT:
            break;

        case GETSUPPORTPTR:
            return arg->message->m_sender->p_supportStruct;

        case GETPROCESSID:
            if(arg->body == 0){
                return arg->message->m_sender->p_pid;
            }
            else{
                if(arg->message->m_sender->p_parent == NULL){
                    return 0;
                }
                else{
                    return arg->message->m_sender->p_parent->p_pid;
                }
            }

        default:
            return NULL;
    }

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
    /*if(proc == NULL){
        return NULL
    }
    else{
        pcb_t tmp = removeChild(proc);
        removeProcQ(tmp);
        return ssi_terminate_process(head, tmp);
    }*/
}


void SSILoop{

    while(!emptyMessageQ(ssi_msg_queue)){
        msg_t service = popMessage(ssi_msg_queue);

        service->m_sender->p_s->gpr[5] = SSIRequest_handler();
        break;
    }
}
