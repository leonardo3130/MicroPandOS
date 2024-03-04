#ifndf SSI
#define SSI

#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../headers/listx.h"
#include "../../phase1/headers/pcb.h"
#include "./timers.h"
#include <umps/libumps.h>

void SSIRequest(pcb_t* sender, int service, msg_t* arg);
pcb_t* ssi_new_process(ssi_create_process_t p_info, pcb_t* parent);
pcb_t ssi_terminate_process(pcb_t* proc);
void SSILoop();
void ssi_clockwait(pcb_t *sender);
int ssi_getprocessid(pcb_t *sender, void *arg);

#endif
