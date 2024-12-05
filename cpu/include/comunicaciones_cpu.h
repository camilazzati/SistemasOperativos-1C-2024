#ifndef CPU_OPERACIONES_H_
#define CPU_OPERACIONES_H_

#include "c_gestor.h"
#include <commons/string.h>

//char** instruccion_dividida;

void esperar_kernel_cpu_dispatch();
void esperar_kernel_cpu_interrupt();
void esperar_memoria_cpu();

void atender_escritura(char*);
void atender_lectura();

void recibir_instruccion(t_buffer*);

void recibir_pcb_del_kernel(t_buffer*);
void iniciar_estructuras_para_recibir_pcb_por_primera_vez(t_buffer*);
void iniciar_estructuras_para_recibir_pcb(t_buffer*);
void mostrar_pcb();

void atender_interrupcion_quantum(t_buffer*);
void atender_interrupcion_exit(t_buffer*);

#endif