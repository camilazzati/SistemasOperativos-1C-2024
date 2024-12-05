#ifndef KERNEL_MEMORIA_H_
#define KERNEL_MEMORIA_H_

#include "../include/k_gestor.h"

void esperar_memoria_kernel();
void esperar_conexiones_memoria();

void iniciar_estructura_en_memoria(pcb* un_pcb);
void liberar_memoria(pcb* un_pcb);

#endif /* KERNEL_MEMORIA_H_ */
