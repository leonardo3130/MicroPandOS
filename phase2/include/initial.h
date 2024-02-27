#ifndef INITIAL
#define INITIAL

#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../headers/listx.h"
#include "../../phase1/headers/pcb.h"
#include "./timers.h"
#include <umps/libumps.h>

//  1. Declare the Level 3 global variables.
unsigned int process_count;
unsigned int soft_blocked_count;
unsigned int start;
unsigned int pid_counter;


LIST_HEAD(Ready_Queue);
LIST_HEAD(Locked_disk);
LIST_HEAD(Locked_flash);
LIST_HEAD(Locked_terminal_in);
LIST_HEAD(Locked_terminal_out);
LIST_HEAD(Locked_ethernet);
LIST_HEAD(Locked_printer);
LIST_HEAD(Locked_Message)


pcb_t *current_process;
pcb_t *ssi_pcb;

extern void test();

void initNucleus();
static void insert_ready_proc(pcb_t *toInsert);

#endif
