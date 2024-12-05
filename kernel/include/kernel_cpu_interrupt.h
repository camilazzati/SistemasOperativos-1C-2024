#ifndef KERNEL_CPU_INTERRUPT_H_
#define KERNEL_CPU_INTERRUPT_H_

#include "../include/k_gestor.h"

void esperar_cpu_interrupt_kernel();
void esperar_conexiones_cpu_interrupt();
void _gestionar_interrupcion(pcb* un_pcb, interrupcion motivo_interrupcion);

#endif /* KERNEL_CPU_INTERRUPT_H_ */