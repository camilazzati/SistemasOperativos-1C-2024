#ifndef OPERACIONES_PROCESP_H_
#define OPERACIONES_PROCESO_H_

#include "m_gestor.h"
#include "encargarse_kernel.h"


t_proceso* obtener_proceso_por_pid(int pid);
char* extraer_instruccion_por_ip(t_proceso* proceso, int ip_proceso);
void destruir_proceso(t_proceso* proceso);
void crear_proceso_nuevo(int pid, char* path);


#endif