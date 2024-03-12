#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../headers/listx.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/msg.h"
#include "interrupts.h"
#include "initial.h"
#include <umps/const.h>
#include <umps/libumps.h>
#include <umps/arch.h>

// #define DEV_REG_ADDR(line, dev) (DEV_REG_START + ((line) - DEV_IL_START) * N_DEV_PER_IL * DEV_REG_SIZE + (dev) * DEV_REG_SIZE)
#define ADDR_TO_NUM(addr, line) (addr - (DEV_REG_START + ((line) - DEV_IL_START) * N_DEV_PER_IL * DEV_REG_SIZE) / DEV_REG_SIZE);

void exceptionHandler();
void uTLB_RefillHandler();

void pippo(int *line, int *n_dev, int *address){
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; i < 5; i++)
        {
            if(DEV_REG_ADDR(j, i) == address)
                *line = j, *dev = i;
        }
    }
    
};

#endif
