#ifndef KERNEL_H_
#define KERNEL_H_

#include "k_gestor.h"

#include "kernel_cpu_dispatch.h"
#include "kernel_cpu_interrupt.h"
#include "kernel_entradasalida.h"
#include "kernel_memoria.h"
#include "inicializar_kernel.h"
#include <utils/include/shared.h>
#include <pthread.h>
#include "consola.h"

// VARIABLES GLOBALES
t_log* kernel_logger;
t_log* kernel_log_debug;
t_log* kernel_logger_extra;
t_config* kernel_config;

int fd_memoria;
int fd_entradasalida;
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_kernel;
// Variables de config
char* PUERTO_ESCUCHA;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;

char* IP_ENTRADASALIDA;
char* PUERTO_ENTRADASALIDA;

char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;

char* ALGORITMO_PLANIFICACION;
int QUANTUM;
char** RECURSOS;
char** INSTANCIAS_RECURSOS;
int GRADO_MULTIPROGRAMACION;

// LISTAS Y VARIABLES DE PLANIFICACIÓN

pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_lista_ready_plus;
pthread_mutex_t mutex_lista_exec;
pthread_mutex_t mutex_lista_new;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_lista_exit;
pthread_mutex_t mutex_procesos_en_core;
pthread_mutex_t mutex_lista_interfaces;
pthread_mutex_t mutex_lista_recursos;

pthread_mutex_t mutex_ticket;
pthread_mutex_t mutex_pid;
pthread_mutex_t mutex_interrupt_pcp;
pthread_mutex_t mutex_interrupt_plp;

t_list* ready;
t_list* ready_plus;
t_list* execute;
t_list* new;
t_list* blocked;
t_list* lista_exit;
t_list* interfaces_conectadas;
t_list* lista_recursos;

sem_t sem_enviar_interrupcion;
sem_t sem_interrupt_pcp;
sem_t sem_interrupt_plp;
sem_t sem_estructura_iniciada_en_memoria;
sem_t sem_multiprogramacion;
sem_t sem_listas_ready;
sem_t sem_lista_new;
sem_t sem_lista_execute;
sem_t sem_solicitud_interfaz;
sem_t sem_pcp;
sem_t sem_cpu_libre;

sem_t sem_RA;
sem_t sem_RB;
sem_t sem_RC;

algoritmo_de_planificacion ALGORITMO_PCP_SELECCIONADO;
  
int identificador_PID = 0;
int contador_pcbs = 0;
int ticket_actual = 0;  
int procesos_en_core = 0;

int flag_respuesta_creacion_proceso = 1;
int flag_interrupt_pcp = 1;
int flag_interrupt_plp = 1;

#endif