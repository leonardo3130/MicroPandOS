#include "./headers/msg.h"
//#include "../klog.c"
#define TRUE 1
#define FALSE 0

static msg_t msgTable[MAXMESSAGES];
LIST_HEAD(msgFree_h);

void initMsgs() {
    for(int i=0;i<MAXMESSAGES;i++){
        //Scorro msgTable e aggiungo ogni elemento in coda a msgFree
        list_add_tail(&(msgTable[i].m_list), &msgFree_h);
    }
}

void freeMsg(msg_t *m) {
    //inserisco la list_head puntata da m in coda alla msgFree
    list_add_tail(&(m->m_list), &msgFree_h);
}

msg_t *allocMsg() {

    //variabile ausiliaria usata per ritornare l'elemento della lista che viene eliminato
    msg_t *tmp;

    if(list_empty(&msgFree_h))
        return NULL;

    else{
        tmp = container_of((&msgFree_h)->next, msg_t, m_list);
        list_del((&msgFree_h)->next);

        tmp->m_sender = NULL;
        tmp->m_payload = 0;
        INIT_LIST_HEAD(&(tmp->m_list));

        //scorro la msgTable e metto tutti i payload = 0
        for(int i=0;i<MAXMESSAGES;i++){
            msgTable[i].m_payload = 0;
        }
        return tmp;
    }
}

void mkEmptyMessageQ(struct list_head *head) {
    INIT_LIST_HEAD(head);
}

int emptyMessageQ(struct list_head *head) {
    return list_empty(head);
}

void insertMessage(struct list_head *head, msg_t *m) {
    //inserisco la list_head puntata da m in coda alla lista head
    list_add_tail(&(m->m_list), head);
}

void pushMessage(struct list_head *head, msg_t *m) {
    //inserisco la list_head puntata da m in testa alla lista head
    list_add(&(m->m_list), head);
}

msg_t *popMessage(struct list_head *head, pcb_t *p_ptr) { 
	  if(list_empty(head)){
        //klog_print("tmp1");
        return NULL;
    }
    //variabile ausiliaria usata per ritornare l'elemento eliminato dalla lista
    msg_t *tmp;
    if(!p_ptr) {
		    tmp = container_of(head->next, msg_t, m_list);
		    list_del(head->next);
        //klog_print("tmp2");
        return tmp;
	  }
    

    int found = FALSE;
    //struct list_head *pos;
    /*list_for_each(pos, head){
        if(container_of(pos, msg_t, m_list)->m_sender == p_ptr && !found){
            found = TRUE;
            tmp = container_of(pos, msg_t, m_list);
            list_del(pos);
        }
    }*/ 
    
    //klog_print_hex((unsigned int)(p_ptr));
    //klog_print("\n");
    list_for_each_entry(tmp, head, m_list) {
        //klog_print_hex((unsigned int)(tmp->m_sender));
        if(tmp->m_sender == p_ptr && found == FALSE) {
            found = TRUE;
        }
    }

    if(found == FALSE){
        //klog_print("\n");
        //klog_print("tmp3");
        return NULL;
    }
    else { 
        //klog_print("tmp4");
        return tmp;
    }
}

msg_t *headMessage(struct list_head *head) {
    if(head == NULL)
        return NULL;

    else
        return container_of(head->next, msg_t, m_list);

}
