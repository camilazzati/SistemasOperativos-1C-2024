#ifndef RECURSOS_KERNEL_H_
#define RECURSOS_KERNEL_H_

#include "../include/k_gestor.h"
#include "../include/shared_kernel.h"
#include "../include/exit_process.h"


void manejar_pedido_de_recurso(pcb *pcb_recibido);
void control_request_de_recursos(instancia_recurso* un_recurso);
void agregar_recurso_a_pcb (pcb* un_pcb, char* un_recurso);
void manejar_signal_de_recurso(pcb *pcb_recibido);
void quitar_recurso (pcb* un_pcb, char* un_recurso);


#endif /* RECURSOS_KERNEL_H_ */