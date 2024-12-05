#ifndef CPU_GESTOR_H
#define CPU_GESTOR_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

#include <utils/include/shared.h>

#include <commons/log.h>
#include <commons/config.h>
#include <commons/temporal.h>

#define LIBRE 0
#define OCUPADA 1

// VARIABLES GLOBALES
extern t_log *cpu_logger;
extern t_log *cpu_logger_extra;
extern t_log *cpu_log_obligatorio;

extern t_config *cpu_config;

extern char *path_config_CPU;

typedef struct
{
    int proceso_pid;
    int proceso_pc;
    int proceso_tiempo_ejecutado;
    int proceso_ticket;
    uint32_t AX;
    uint32_t BX;
    uint32_t CX;
    uint32_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
} t_contexto;

extern t_contexto *contexto; // mutex

// SEMAFOROS
extern pthread_mutex_t mutex_manejo_contexto;
extern pthread_mutex_t mutex_ordenCargaGlobal;

extern sem_t sem_pedido_tamanio_pag;
extern sem_t sem_pedido_instruccion;
extern sem_t sem_pedido_marco;
extern sem_t sem_rta_resize;
extern sem_t sem_solicitud_lectura;
extern sem_t sem_solicitud_escritura;

// Variables de config
extern char *IP_MEMORIA;
extern char *PUERTO_MEMORIA;
extern char *PUERTO_ESCUCHA_DISPATCH;
extern char *PUERTO_ESCUCHA_INTERRUPT;

extern int CANTIDAD_ENTRADAS_TLB;
extern char *ALGORITMO_TLB;

extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
extern int fd_kernel_dispatch;
extern int fd_kernel_interrupt;
extern int fd_memoria;

// CICLO INSTRUCCION

extern void mostrar_pcb();

extern void realizarCicloInstruccion();

extern char **instruccion_dividida; 

extern bool hay_interrupcion_quantum; 
extern bool hay_interrupcion_exit; 

extern bool primer_pcb;

extern void iniciar_tiempo();

// TLB

typedef struct tlbEntrada
{
    uint32_t pid;
    uint32_t pagina;
    int32_t marco;
    int estado;

    int orden_carga;
    t_temporal *ultimo_uso;
} t_tlbEntrada;

typedef struct tlb
{
    t_tlbEntrada *entradas;
    int tamanio;
} t_tlb;

extern t_tlb *tlb; 
extern int algoritmo_tlb; 

extern int ordenCargaGlobal; 

extern int tamanio_pagina;

extern int marco; 

extern int resultado; 

extern char *respuesta_marco_lectura; 
extern char *respuesta_marco_escritura; 

#endif
