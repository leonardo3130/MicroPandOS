#ifndf SSI
#define SSI

#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../phase1/headers/pcb.h"
#include "./includes/initial.h"
#include <umps/libumps.h>

//support stuct for SSIRequest argument passing
struct sys_arg{
    msg_t* message;
    void* body;
} sys_arg, *sys_arg_ptr;

LIST_HEAD(ssi_msg_queue);
void SSIRequest(pcb_t* sender, int service, msg_t* arg);
void SSIRequest_handler();
ssi_create_process_t* ssi_new_process(pcb_t parent);
pcb_t ssi_terminate_process(list_head *head, pcb_t *proc);


#endif
