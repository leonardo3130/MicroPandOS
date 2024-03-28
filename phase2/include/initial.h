#ifndef INITIAL
#define INITIAL

#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../headers/listx.h"
#include "../../phase1/headers/pcb.h"
#include "exceptions.h"
#include "timers.h"
#include <umps/libumps.h>

//  1. Declare the Level 3 global variables.
extern int process_count;
extern int soft_blocked_count;
extern int start;
extern int pid_counter;



extern struct list_head Ready_Queue;
extern struct list_head Locked_disk;
extern struct list_head Locked_flash;
extern struct list_head Locked_terminal_recv;
extern struct list_head Locked_terminal_transm;
extern struct list_head Locked_ethernet;
extern struct list_head Locked_printer;
extern struct list_head Locked_pseudo_clock;


extern pcb_t *current_process;
extern pcb_t *ssi_pcb;

extern void test();

void initPassupVector();
void initFirstProcesses();

#endif
