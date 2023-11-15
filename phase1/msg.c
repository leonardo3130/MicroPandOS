#include "./headers/msg.h"
#define TRUE 1
#define FALSE 0

static msg_t msgTable[MAXMESSAGES];
LIST_HEAD(msgFree_h);

void initMsgs() {
}

void freeMsg(msg_t *m) {
    //list_add() {
    
}

msg_t *allocMsg() {
    if(list_empty(msgFree_h) == true)
        return NULL;
    else{
        list_del(msgFree_h);
        for(int i=0;i<MAXMESSAGES;i++){
            msgTable[i].m_payload = 0;
        }
        return msgFree_h;
    }
}

void mkEmptyMessageQ(struct list_head *head) {
}

int emptyMessageQ(struct list_head *head) {
}

void insertMessage(struct list_head *head, msg_t *m) {
}

void pushMessage(struct list_head *head, msg_t *m) {
}

msg_t *popMessage(struct list_head *head, pcb_t *p_ptr) {
}

msg_t *headMessage(struct list_head *head) {
}
