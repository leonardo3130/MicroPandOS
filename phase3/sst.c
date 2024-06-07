#include "include/sst.h"

typedef unsigned int devregtr;

/* richiesta del proprio support_t alla ssi */
support_t *support_request(){
    support_t *sup;
    ssi_payload_t payload = {
        .service_code = GETSUPPORTPTR,
        .arg = NULL,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&sup), 0);
    return sup;
}

unsigned int sst_terminate(int asid){
    //mandare messaggio al test per comunicare terminazione SST
    for (int i = 0; i < POOLSIZE; ++i)
        if (swap_pool_table[i].sw_asid == asid)
            swap_pool_table[i].sw_asid = -1; // libero il frame
    SYSCALL(SENDMESSAGE, (unsigned int)test_pcb, 0, 0);
    //richesta termprocess alla ssi
    int ret;
    ssi_payload_t payload = {
        .service_code = TERMPROCESS,
        .arg = NULL,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&ret), 0);

    return (unsigned int)ret;
}

unsigned int sst_write(support_t *sup, unsigned int device_type, sst_print_t *payload){

    //controllo lunghezza stringa
    if(payload->string[payload->length] != '\0'){
        payload->string[payload->length] = '\0';
    }
    
    pcb_t *dest;
    switch(device_type){
        case 6:
            dest = printer_pcbs[sup->sup_asid-1];
        case 7:
            dest = terminal_pcbs[sup->sup_asid-1];
        default:
            break;
    }


    SYSCALL(SENDMESSAGE, (unsigned int) dest, (unsigned int)payload->string, 0);

    klog_print_hex((unsigned int)dest);
    klog_print(" Prima SST\n\n");

    pcb_t *sender;
    sender = SYSCALL(RECEIVEMESSAGE, (unsigned int) dest, 0, 0);
    klog_print_hex((unsigned int)sender);
    klog_print(" dopo SST\n\n");

    return 1;
}

/*  Esecuzione continua del processo SST attraverso un ciclo while   */
void SST_loop(){
    support_t *sup = support_request(); 
    state_t *state = &UProc_state[sup->sup_asid - 1];
    
    //klog_print_dec(sup->sup_asid - 1);
    create_process(state, sup);
    
    //klog_print_dec(sup->sup_asid - 1);

    while(TRUE){
        //klog_print_dec(sup->sup_asid - 1);
        unsigned int payload;

        //attesa di una richiesta da parte del figlio attraverso SYS2
        unsigned int sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&payload), 0);
        //klog_print_dec(sup->sup_asid - 1);

        //tentativo di soddisfare la richiesta 
        unsigned int ret = SSTRequest(sup, (ssi_payload_t *)payload);

        //se necessario restituzione di messaggio di ritorno attraverso SYS1
        if(ret != -1)
            SYSCALL(SENDMESSAGE, (unsigned int)sender, ret, 0);
    }
}

/*  Funzione che gestisce mediante il payload ricevuto dal sender la richiesta di un servizio   */
unsigned int SSTRequest(support_t *sup, ssi_payload_t *payload){
    unsigned int ret = 0;
    switch(payload->service_code){
        case GET_TOD:
            ret = getTOD();

        case TERMINATE:
            ret = sst_terminate(sup->sup_asid);

        case WRITEPRINTER:
            ret = sst_write(sup, 6, (sst_print_t *)payload->arg);

        case WRITETERMINAL:
            ret = sst_write(sup, 7, (sst_print_t *)payload->arg);

        default:
            ret = sst_terminate(sup->sup_asid);
            break;
    }
    return ret;
}
