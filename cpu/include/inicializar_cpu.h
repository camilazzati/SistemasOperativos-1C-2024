#ifndef INICIALIZAR_CPU_H
#define INICIALIZAR_CPU_H

#include "c_gestor.h"

#define LRU 1
#define FIFO 2

void inicializar_cpu();
void iniciar_logs();
void iniciar_config();
void imprimir_config();
void inicializar_mutexs();
void inicializar_semaforos();
void inicializar_variables();

t_tlb* crear_TLB();

#endif 

