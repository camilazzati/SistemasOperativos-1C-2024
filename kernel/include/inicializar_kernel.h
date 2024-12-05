#ifndef INICIALIZAR_KERNEL_H_
#define INICIALIZAR_KERNEL_H_

#include "k_gestor.h"
#include "../include/control_procesos.h"

void inicializar_kernel();
void inicializar_logs();
void inicializar_configs();
void crear_listas();
void inicializar_semaforos();
void inicializar_mutexes();
void establecer_algoritmo_seleccionado();
void inicializar_planificadores();
void establecer_recursos();
void inicializar_asistentes_de_recurso();


#endif