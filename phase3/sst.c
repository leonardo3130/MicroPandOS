#include "include/sst.h"

/* richiesta del proprio support_t alla ssi */
support_t support_request(){
    support_t *sup;
    ssi_payload_t payload = {
        .service_code = GETSUPPORTPTR,
        .arg = NULL,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&sup), 0);
    return sup;
}

unsigned int sst_terminate(pcb_t *sender){
    //richesta termprocess alla ssi
    int ret;
    ssi_payload_t payload = {
        .service_code = TERMPROCESS,
        .arg = NULL,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&ret), 0);

    return ret;
}

unsigned int sst_write(pcb_t *sender, unsigned int device_type, sst_print_t payload){
    
    //trovo il corretto asid e calcolo il relativo indirizzo di memoria del dispostivo sul quale voglio scrivere
    for(int i=1; i<8; i++){
        if(i == sender->p_supportStruct->sup_asid){
            memaddr *base = (memaddr*)(DEV_REG_ADDR(device_type, i));
        }
    }

    /*casting
    if(device_type == 7){
        base = (dtpreg_t *)base;
    }
    else if(device_type == 6){
        base = (termreg_t *)base;
    }*/

    memaddr *command = base + 3;
    memaddr ret;

    for(int i=0; i<(payload->len); i++){

        char *letter = payload->str[i];
        devregtr value = ((devregtr)*letter) << 8;

        ssi_do_io_t doio_struct = {
            .commandAddr = command;
            .commandValue = value;
        }
        ssi_payload_t payload = {
            .service_code = DOIO,
            .arg = &doio_struct,
        };
        SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
        SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&ret), 0);
    }
}

/*  Esecuzione continua del processo SST attraverso un ciclo while   */
void SST_loop(){
    support_t *sup = support_request(); 
    //state_t *state = ;

    create_process(&state, &sup);


    while(TRUE){
        unsigned int payload;

        //attesa di una richiesta da parte del figlio attraverso SYS2
        unsigned int sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&payload), 0);

        //tentativo di soddisfare la richiesta 
        unsigned int ret = SSTRequest((pcb_t *)sender, (ssi_payload_t *)payload);

        //se necessario restituzione di messaggio di ritorno attraverso SYS1
        if(ret != -1)
            SYSCALL(SENDMESSAGE, (unsigned int)sender, ret, 0);
    }
}

/*  Funzione che gestisce mediante il payload ricevuto dal sender la richiesta di un servizio   */
unsigned int SSTRequest(pcb_t* sender, ssi_payload_t *payload){
    unsigned int ret = 0;
    switch(payload->service_code){
        case GET_TOD:
            ret = getTOD();

        case TERMINATE:
            ret = sst_terminate(sender);

        case WRITEPRINTER:
            ret = sst_write(sender, 6, (sst_print_t)payload);

        case WRITETERMINAL:
            ret = sst_write(sender, 7, (sst_print_t)payload);

        default:
            ret = sst_terminate(sender);
            break;
    }
    return ret;
}
