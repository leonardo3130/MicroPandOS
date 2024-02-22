#include "include/timers.h"

//TOD --> time of day clock, readonly, per l'accesso serve kernel mode
//accesso al suo valore --> STCK(int), non genera interrupt
//
//Interval Timer --> 32-bit unsigned int decrementato ogni ciclo, settato al boot 
//a 0xFFFFFFFF, genera interrupt su linea 2 con la transizione 0x00000000 --> 0xFFFFFFFF
//lettura/scrittura richiedono kernel mode, eseguite tramite:
//- LDIT(t) --> t diventa nuovo valore dle timer --> solo scrittura
//- accesso diretto a Bus Register Memory Location = 0x10000020 (per lettura e scrittura)
// 
//PLT --> processor local timer --> ogni processo ha il suo, impkementato con CP0 timer Register
//decrementato ad ogni clock, interrupt con transizione: 0x00000000 --> 0xFFFFFFFF
//interrupt che può essere disabilitato, a differenza di interval timer 
//accesso in lettura/scrittura con getTIMER e setTIMER

//questa libreria è un semplice wrapper per le funzioni sui timer

unsigned int getTOD() {
  unsigned int tmp;
  STCK(tmp);
  return tmp;
}

void updateCPUtime(pcb_t *p, unsigned int *start) {
  unsigned int end = getTOD();
  p->p_time += (end - *start);
  *start = end;
}

void setIntervalTimer(unsigned int t) {
  LDIT(T);
}

void setPLT(unsigned int t) {
  setTIMER(t);
}

unsigned int getPLT() {
  return getTIMER();
}
