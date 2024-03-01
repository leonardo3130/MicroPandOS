#ifndf SSI
#define SSI

#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../headers/listx.h"
#include "../../phase1/headers/pcb.h"
#include "./timers.h"
#include <umps/libumps.h>


//support stuct for SSIRequest argument passing
struct sys_arg{
    msg_t* message;
    void* body;
} sys_arg, *sys_arg_ptr;

void SSIRequest(pcb_t* sender, int service, msg_t* arg);
void SSIRequest_handler();
pcb_t* ssi_new_process(ssi_create_process_t p_info, pcb_t* parent);
pcb_t ssi_terminate_process(pcb_t* proc);
void SSILoop()

#endif
