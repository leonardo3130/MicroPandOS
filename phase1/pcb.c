#include "./headers/pcb.h"
#include "../headers/types.h"

#define TRUE 1
#define FALSE 0

static pcb_t pcbTable[MAXPROC];
LIST_HEAD(pcbFree_h);
static int next_pid = 1;

void initPcbs() {
    for (int i = 0; i < MAXPROC; i++)
    {
        list_add_tail(&(pcbTable[i].p_list), &pcbFree_h);
    }
}

//Inserisce l'elemento puntato da p sulla lista pcbFree
void freePcb(pcb_t *p) {
    list_add_tail(&(p -> p_list), &pcbFree_h);
}

pcb_t *allocPcb() {
    if(list_empty(&pcbFree_h) /*== TRUE*/)
        return NULL;
    else
    {   /*
        struct list_head *to_del = &pcbFree_h;
        list_del(to_del->next);

        pcb_t *p = container_of(to_del->next, pcb_t, p_list);*/

        //struct list_head *to_del = &((&pcbFree_h) -> next);

        pcb_t *p = container_of((&pcbFree_h)->next, pcb_t, p_list);
        list_del((&pcbFree_h)->next);

        //coda dei processi
        INIT_LIST_HEAD(&(p -> p_list));
        INIT_LIST_HEAD(&(p -> p_child));
        p->p_parent = NULL;

        //campi albero processi
        INIT_LIST_HEAD(&(p -> p_sib));

        //informazioni sullo stato dei processi
        // p->p_s = (state_t) {0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0, 0, 0, 0, 0, 0, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0};
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
    list_add_tail(&(p -> p_list), head);
}

//ritorna NULL se la coda dei processi è vuota, altrimenti il PCB in testa
pcb_t *headProcQ(struct list_head *head) {
    return (emptyProcQ(head) ? NULL : container_of(head->next, pcb_t, p_list));
}

pcb_t *removeProcQ(struct list_head *head) {
    if(emptyProcQ(head))
        return NULL;
    else
    {
        pcb_t *firs_pcb = container_of(list_next(head), pcb_t, p_list);
        list_del(&(firs_pcb -> p_list));
        return firs_pcb;
    }
}
/*
Remove the PCB pointed to by p from the process queue whose head pointer is pointed to by head. 
If the desired entry is not in the indicated queue (an error condition), return NULL; otherwise, 
return p. Note that p can point to any element of the process queue.
*/
pcb_t *outProcQ(struct list_head *head, pcb_t *p) {
    struct list_head* iter; 
    pcb_t *q;
    list_for_each(iter, head){
        q = container_of(iter, pcb_t, p_list);
        if (q == p){
            list_del(&(q -> p_list));
            return p;
        }
    }
    return NULL;
}

int emptyChild(pcb_t *p) {
    return list_empty(&(p -> p_child)); //controllo che la lista dei figli sia vuota
}

void insertChild(pcb_t *prnt, pcb_t *p) {
    p -> p_parent = prnt;
    if(emptyChild(prnt))
		list_add(&(p -> p_sib) , &(prnt -> p_child));
    else
		list_add_tail(&(p -> p_sib), &(prnt -> p_child)); //aggiungo in coda per rispettare la condazione FIFO
}

pcb_t *removeChild(pcb_t *p) {
    if(emptyChild(p))
        return NULL;
    else {
		pcb_t *first_child = container_of(list_next(&(p -> p_child)), pcb_t, p_sib); //ricerca primo figlio
		list_del(&(first_child -> p_sib)); //rimozione figlio dalla lista dei fratelli
		first_child -> p_parent = NULL; //rimozione legame padre-figlio
		return first_child;
    }
}

pcb_t *outChild(pcb_t *p) {
	if(p -> p_parent == NULL) 
		return NULL;
	else {
		list_del(&(p -> p_sib)); //rimozione p dalla lista dei fratelli
		p -> p_parent = NULL; //rimozione legame padre-figlio
		return p;
	}
}
