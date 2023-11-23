#include "./headers/msg.h"
#define TRUE 1
#define FALSE 0

static msg_t msgTable[MAXMESSAGES];
LIST_HEAD(msgFree_h);

void initMsgs() {
    for(int i=0;i<MAXMESSAGES;i++){
        //Scorro msgTable e aggiungo ogni elemento a msgFree
        list_add_tail(&(msgTable[i].m_list), &msgFree_h);
    }
}

void freeMsg(msg_t *m) {
    list_add_tail(&(m->m_list), &msgFree_h);
}

msg_t *allocMsg() {
    msg_t *tmp;
    if(list_empty(&msgFree_h) /* == TRUE*/)
        return NULL;
    else{
        tmp = container_of((&msgFree_h) -> next, msg_t, m_list);
        list_del((&msgFree_h) -> next);
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
    list_add_tail(&(m->m_list), head);
}

void pushMessage(struct list_head *head, msg_t *m) {
    list_add(&(m->m_list), head);
}

msg_t *popMessage(struct list_head *head, pcb_t *p_ptr) {

    if(p_ptr == NULL)
        return container_of(head->next, msg_t, m_list);
 
    else if(list_empty(head)) //|| found == FALSE)
        return NULL;

    else {
        struct list_head *pos = head;
        msg_t *tmp;
        int found = FALSE;
        list_for_each(pos, head){
            if(container_of(pos, msg_t, m_list)->m_sender == p_ptr && found == FALSE){
                found = TRUE;
                tmp = container_of(pos, msg_t, m_list);
                list_del(pos);
            }
        }
        if(found == FALSE)
            return NULL;
        else 
            return tmp;
    }
}

msg_t *headMessage(struct list_head *head) {
    if(head!= NULL){
        return container_of(head->next, msg_t, m_list);
    }
    else{
        return NULL;
    }
}
