#ifndef CPU_H_
#define CPU_H_

#include "c_gestor.h"
#include "inicializar_cpu.h"
#include "comunicaciones_cpu.h"
#include <utils/include/shared.h>
#include <pthread.h>

// VARIABLES GLOBALES
t_log* cpu_logger;
t_log* cpu_logger_extra;
t_contexto* contexto;
t_config* cpu_config;
t_log* cpu_log_obligatorio;

// SEMAFOROS
pthread_mutex_t mutex_instruccion;
pthread_mutex_t mutex_interrupcion_quantum;
pthread_mutex_t mutex_interrupcion_exit;
pthread_mutex_t mutex_tlb;
pthread_mutex_t mutex_ordenCargaGlobal;
pthread_mutex_t mutex_marco;
pthread_mutex_t mutex_rta_lectura;
pthread_mutex_t mutex_rta_escritura;
pthread_mutex_t mutex_resultado;
pthread_mutex_t mutex_manejo_contexto;

sem_t sem_pedido_tamanio_pag;
sem_t sem_pedido_instruccion;
sem_t sem_pedido_marco;
sem_t sem_rta_resize;
sem_t sem_solicitud_lectura;
sem_t sem_solicitud_escritura;

//char* path_config_CPU = "/home/utnso/so-deploy/tp-2024-1c-ubuntunel/cpu/CPU.config"; //para el labo
//char* path_config_CPU = "/home/utnso/Documents/tp-2024-1c-ubuntunel/cpu/CPU.config"; //para casa

// Variables de config
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;

int CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;

int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_kernel_dispatch;
int fd_kernel_interrupt;
int fd_memoria;

char* respuesta_marco_escritura;
char* respuesta_marco_lectura;

int marco;
int tamanio_pagina;
int resultado;
char** instruccion_dividida;
int algoritmo_tlb;
t_tlb* tlb;
int ordenCargaGlobal;

bool primer_pcb;

#endif
