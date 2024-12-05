#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "../include/k_gestor.h"
#include "../include/shared_kernel.h"
#include "../include/control_procesos.h"
#include "../include/exit_process.h"

#include <utils/include/shared.h>

void iniciar_consola();
bool validacion_de_instruccion_de_consola(char* leido);
void atender_instruccion(char* leido);
void f_iniciar_proceso(t_buffer* un_buffer);
void imprimir_procesos(t_list* una_lista, pthread_mutex_t* un_mutex);
void imprimir_procesos_exit(t_list* una_lista, pthread_mutex_t* un_mutex);

t_list* obtener_instrucciones_del_archivo(char* path_archivo_instrucciones);
t_list* procesar_archivo(const char* path_archivo);
void imprimir_comandos();
void imprimir_recursos_procesos(t_list* una_lista, pthread_mutex_t* un_mutex);
void modificar_grado_multiprogramacion(int* valor_solicitado);

#endif