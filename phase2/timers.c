#include "include/timers.h"

/*
 * semplice libreria che funge da wrapper 
 * per le funzioni di utilizzo dei vari timer
 */

//ritorna il valore del TOD al momento della chiamata
unsigned int getTOD() {
  unsigned int tmp;
  STCK(tmp);
  return tmp;
}

//aggiorna p_time del processo p
void updateCPUtime(pcb_t *p) {
  int end = getTOD();
  p->p_time += (end - start);
  start = end;
}

void setIntervalTimer(unsigned int t) {
  LDIT(t);
}

void setPLT(unsigned int t) {
  setTIMER(t);
}

unsigned int getPLT() {
  return getTIMER();
}
