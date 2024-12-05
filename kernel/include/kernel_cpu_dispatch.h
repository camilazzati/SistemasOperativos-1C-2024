#ifndef KERNEL_CPU_DISPATCH_H_
#define KERNEL_CPU_DISPATCH_H_

#include "../include/k_gestor.h"
#include "../include/shared_kernel.h"
#include "../include/control_procesos.h"

void esperar_cpu_dispatch_kernel();
void enviar_pcb_CPU_dispatch(pcb* un_pcb);
void esperar_conexiones_cpu_dispatch();
void extraer_datos_auxiliares(t_buffer* un_buffer,instruccion_interfaz instruccion_solicitada, pcb* un_pcb);

#endif /* KERNEL_CPU_DISPATCH_H_ */