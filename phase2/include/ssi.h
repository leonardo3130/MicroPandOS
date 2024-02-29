#ifndf SSI
#define SSI

#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../headers/listx.h"
#include "../../phase1/headers/pcb.h"
#include "./timers.h"
#include <umps/libumps.h>

LIST_HEAD(clock_waiting_pcb);
extern void SSIRequest(pcb_t* sender, int service, msg_t* arg);
void SSIRequest_handler();
pcb_t* ssi_new_process(ssi_create_process_t p_info, pcb_t* parent);
pcb_t ssi_terminate_process(pcb_t* proc);
extern void SSILoop()

#endif
