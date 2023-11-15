#include "./headers/pcb.h"
#include "../headers/types.h"
#define TRUE 1
#define FALSE 0

static pcb_t pcbTable[MAXPROC];
LIST_HEAD(pcbFree_h);
static int next_pid = 1;

void initPcbs() {
    int size = sizeof(pcbTable)/sizeof(pcbTable[0]);

    // list_add(pcbFree_h,

}

//Inserisce l'elemento puntato da p sulla lista pcbFree
void freePcb(pcb_t *p) {
    list_add_tail(&(p -> p_list), &pcbFree_h);
}

pcb_t *allocPcb() {
    if(list_empty(&pcbFree_h) /*== TRUE*/)
        return NULL;
    else
    {
        struct list_head *to_del = &pcbFree_h;
        list_del(to_del->next);

        pcb_t *p = container_of(to_del->next, pcb_t, p_list);

        //coda dei processi
        INIT_LIST_HEAD(&(p -> p_list));
        INIT_LIST_HEAD(&(p -> p_child));
        p->p_parent = NULL;

        //campi albero processi
        INIT_LIST_HEAD(&(p -> p_sib));

        //informazioni sullo stato dei processi
        //p->p_s
        p->p_time=0;

        //primo messaggio della coda dei messaggi
        INIT_LIST_HEAD(&(p->msg_inbox));

        //puntatore alla struct di supporto
        p->p_supportStruct = NULL;

        //process ID
        p->p_pid=0;

        return p;
    }
}

//inizializza una variable come puntatore alla testa della coda dei processi
void mkEmptyProcQ(struct list_head *head) {
    INIT_LIST_HEAD(head);
}


// se la coda la cui testa è puntata da head è vuota ritrna TRUE, altrimeti FALSE
int emptyProcQ(struct list_head *head) {
    return list_empty(head);
}

//inserisce il PCB puntato da p nella coda dei processi
void insertProcQ(struct list_head *head, pcb_t *p) {
    list_add(head, &(p -> p_list));
}

//ritorna NULL se la coda dei processi è vuota, altrimenti il PCB in testa
pcb_t *headProcQ(struct list_head *head) {
    return (emptyProcQ(head) ? NULL : container_of(head, pcb_t, p_list));
}

pcb_t *removeProcQ(struct list_head *head) {
    if(emptyProcQ(head))
        return NULL;
    else
    {

    }
}

pcb_t *outProcQ(struct list_head *head, pcb_t *p) {

}

int emptyChild(pcb_t *p) {
    //return list_empty(&(p -> p_child)); --> sbagliato
    if(p -> p_child.next == NULL)
        return TRUE;
    return FALSE;
}

void insertChild(pcb_t *prnt, pcb_t *p) {
    p -> p_parent = prnt;
    if(emptyChild(prnt)) {
        prnt -> p_child.next = &(p -> p_child);
    }
    else {
        pcb_t *first_child = container_of(prnt -> p_child.next, pcb_t, p_child);
        list_add_tail(&(p -> p_sib), &(first_child) -> p_sib);
    }
}

pcb_t *removeChild(pcb_t *p) {
    if(emptyChild(p))
        return NULL;
    else {
        //puntatore child del padre dovra puntatore non più a quello a cui punta ora, ma al suo next
        //quindi devo fare controllo per un solo figlio, e devo rimuover il primo figlio dalla lista dei sib
        //
    }
}

pcb_t *outChild(pcb_t *p) {
}
