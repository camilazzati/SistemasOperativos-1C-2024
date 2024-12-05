#ifndef EXIT_KERNEL_H_
#define EXIT_KERNEL_H_

#include "../include/k_gestor.h"
#include "../include/shared_kernel.h"
#include "../include/kernel_cpu_interrupt.h"
#include "../include/kernel_memoria.h"


void planificar_proceso_exit(pcb* un_pcb);
void planificar_proceso_exit_en_hilo(pcb* un_pcb);
void _gestionar_salida(pcb* un_pcb);


// MANEJO DE RECURSOS
void liberar_recursos_pcb (pcb* un_pcb);
void liberar_recursos(pcb* un_pcb);
void _signal_recurso_exit(instancia_recurso_pcb* recurso_pcb);
void eliminar_de_lista_recurso(pcb* un_pcb);

// MANEJO DE EXIT INTERFACES
void liberar_interfaces(pcb* un_pcb);
void eliminar_de_lista_interfaz(pcb* un_pcb);

#endif /* EXIT_KERNEL_H_ */