#include "../headers/const.h"
#include "../headers/types.h"
#include <umps/libumps.h>

void SSIRequest(msg_t request){
	SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, request->m_payload, 0);


}

void SSIRequest_handler(int r_val){
    switch(r_val){
        case:
        break;
        default:
        break;
    }

}

void SSILoop{
    while(true){
        int val = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
        SSIRequest_handler(val);
        break;
    }

}
