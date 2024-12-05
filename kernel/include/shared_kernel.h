#ifndef SERVICIOS_KERNEL_H_
#define SERVICIOS_KERNEL_H_

#include "../include/k_gestor.h"
#include "../include/kernel_memoria.h"


// FUNCIONES DE PCB

int asignar_pid();
int generar_ticket();
void list_add_pcb_sync(t_list* lista, pcb* un_pcb, pthread_mutex_t* mutex, estado_pcb nuevo_estado);
void actualizar_pcb(pcb* pcb_desactualizado,pcb* pcb_nuevo);
void destruir_pcb(pcb* un_pcb);
pcb* _eliminar_pcb_de_lista_sync(pcb* un_pcb, t_list* una_lista, pthread_mutex_t* mutex);
void agregar_a_ready(pcb* un_pcb);
void cambiar_estado_pcb(pcb* un_pcb, estado_pcb nuevo_estado);
void obtener_contexto_pcb(t_buffer* un_buffer, pcb* un_pcb,char* modulo);
pcb* buscar_pcb_en_sistema_(int pid);
pcb* _buscar_pcb_en_lista(int pid, t_list* una_lista, pthread_mutex_t* mutex_lista);
void mensaje_transicion_estado(pcb* un_pcb, estado_pcb nuevo_estado);
char* estado_a_char(estado_pcb un_estado);
void mostrar_contenido(t_list* una_lista_pcbs,pthread_mutex_t* mutex);

// FUNCIONES DE LISTAS

void agregar_int_a_lista(t_list* una_lista, int un_valor);
void agregar_string_a_lista(t_list* una_lista, const char* un_string);
void list_add_sync(t_list* lista, void* un_elemento, pthread_mutex_t* mutex);
void eliminar_datos_auxiliares(pcb* un_pcb);

// FUNCIONES GENÉRICAS

void decrementar_procesos_en_core();
void mostrar_contexto(pcb* un_pcb);

// FUNCIONES QUE QUEDARON Y NO SE UTILIZAN

bool _pcb_esta_en_lista(pcb* un_pcb, t_list* una_lista, pthread_mutex_t* mutex);
pcb* _extraer_pcb_de_lista_sistema(pcb* un_pcb);
pcb* _extraer_pcb_de_lista(int pid, t_list* una_lista, pthread_mutex_t* mutex_lista);


#endif