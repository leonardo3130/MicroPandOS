#include "../headers/const.h"
#include "../headers/types.h"
#include <umps/libumps.h>

LIST_HEAD_INIT(ssi_msg_queue);

void SSIRequest(pcb_t* sender, int service, void* arg){
	SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, request->m_payload, 0);
}

msg_t SSIReceive(pcb_t current_proc){
    msg_t request = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
    insertMessage(ssi_msg_queue, request);
    state_t *tmp = BIOSDATAPAGE;
    current_proc.p_s = tmp;
    current_proc.p_time += accum_time;  //accum_time da definire

    //  *chiamata a scheduler*

    return r_val;
}
void SSIRequest_handler(){
        msg_t request = SSIReceive(Current_Process);
        switch(request.m_payload){
        case CREATEPROCESS:
            ssi_create_process_t* ret = ssi_new_process(request.m_sender);
            //  *ritorna il controllo al processo corrente*
            break;
        case TERMPROCESS:
            break;
        case DOIO:
            break;
        case GETTIME:
            break;
        case CLOCKWAIT:
            break;
        case GETSUPPORTPTR:
            break;
        case GETPROCESSID:
            break;
        default:
            break;
    }

}
ssi_create_process_t* ssi_new_process(pcb_t parent){
    ssi_create_process_t * new_p;
    pcb_t child;
    child.state = &new_p->state;
    child.p_supportStruct = &new_p->support;
    insertChild(parent, new_p);
    insertProcQ(Ready_Queue, new_p);
    //  *process count ++ *
    return *new_p;
}

void SSILoop{
    
    while(true){
        SSIRequest_handler();
        break;
    }
}