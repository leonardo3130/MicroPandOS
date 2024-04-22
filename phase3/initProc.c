#include "./include/initProc.h"

support_t ss_array[UPROCMAX]; //support struct array
pcb_t *swap_mutex_pcb;
swap_t swap_pool_table[POOLSIZE];
pcb_t *sst_array[UPROCMAX];
pcb_t *terminal_pcbs_recv[UPROCMAX];
pcb_t *terminal_pcbs_transm[UPROCMAX];
pcb_t *printer_pcbs[UPROCMAX];

pcb_t *create_process(state_t *s, support_t *sup)
{
    pcb_t *p;
    ssi_create_process_t ssi_create_process = {
        .state = s,
        .support = sup,
    };
    ssi_payload_t payload = {
        .service_code = CREATEPROCESS,
        .arg = &ssi_create_process,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&p), 0);
    return p;
}

void initSwapPoolTable() 
{
    for (i = 0; i < POOLSIZE; i++)
        swap_pool_table[i].sw_asid = -1;
}

void initSwapMutex()
{
    
}

void test() 
{
    pcb_t *test = current_process;

}
