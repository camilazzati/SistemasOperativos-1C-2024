#ifndef K_GESTOR_H_
#define K_GESTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <readline/readline.h>

#include "../src/utils/include/shared.h"

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

//Variables globales

extern t_log* kernel_logger;
extern t_log* kernel_log_debug;
extern t_log* kernel_logger_extra;
extern t_config* kernel_config;

extern char* path_config_Kernel;

// Variables de config

extern char* PUERTO_ESCUCHA;

extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;

extern char* IP_ENTRADASALIDA;
extern char* PUERTO_ENTRADASALIDA;

extern char* IP_CPU;
extern char* PUERTO_CPU_DISPATCH;
extern char* PUERTO_CPU_INTERRUPT;

extern char* ALGORITMO_PLANIFICACION;
extern int QUANTUM;
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS;
extern int GRADO_MULTIPROGRAMACION;

extern int fd_memoria;
extern int fd_entradasalida;
extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
extern int fd_kernel;

typedef enum{
	NEW,
	READY,
	EXEC,
	BLOCKED,
	EXIT
}estado_pcb;


typedef enum{
	FIFO,
	RR,
	VRR
}algoritmo_de_planificacion;

typedef enum{
	PEDIDO_A_INTERFAZ,
	WAIT,
	SIGNAL,
	BLOQUEO_NO_DEFINIDO
}motivo_bloqueo;

typedef struct{
	char* nombre_interfaz;
	instruccion_interfaz instruccion_a_interfaz;
	t_list* datos_auxiliares_interfaz;
}pedido_interfaz;

typedef struct{
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
}registrosCPU;

typedef struct{
	char* nombre_recurso;
	int cantidad_instancias;
	// Lista de PCBs
	t_list* lista_procesos_en_cola;
    sem_t semaforo_recurso;
	sem_t semaforo_request_recurso;
	pthread_mutex_t mutex_lista_procesos_en_cola;  
}instancia_recurso;

typedef struct{
	char* nombre_recurso;
    int instancias_recurso;
}instancia_recurso_pcb;
typedef struct{ //
	
	int pid;
	int program_counter;
	int quantum;
	int tiempo_ejecutado;
	int ticket;
	char* path;
	registrosCPU* registros_CPU;
	estado_pcb estado; // Solamente sirve para los log_info
	motivo_bloqueo motivo_bloqueo; 
	pedido_interfaz* pedido_a_interfaz;
	char* pedido_recurso;
	// Lista de instancia_recurso_pcb 
	t_list* recursos_en_uso;
	
}pcb;

typedef struct{
	char* nombre_interfaz;
	int estado_conexion;
	// Lista de instruccion_interfaz
	t_list* instrucciones_disponibles;
	// Lista de PCBs
	t_list* lista_procesos_en_cola;
	int* fd_conexion; // Consultar si tiene que ser puntero o debe ser int
	resultado_operacion resultado_operacion_solicitada;
	pthread_mutex_t mutex_interfaz;
	
	sem_t sem_interfaz;
	sem_t sem_request_interfaz;
	sem_t sem_instruccion_interfaz;

}interfaz;


// LISTAS Y VARIABLES DE PLANIFICACIÓN

extern pthread_mutex_t mutex_lista_ready;
extern pthread_mutex_t mutex_lista_ready_plus;
extern pthread_mutex_t mutex_lista_exec;
extern pthread_mutex_t mutex_lista_new;
extern pthread_mutex_t mutex_lista_blocked;
extern pthread_mutex_t mutex_lista_exit;
extern pthread_mutex_t mutex_procesos_en_core;
extern pthread_mutex_t mutex_lista_interfaces;
extern pthread_mutex_t mutex_lista_recursos;


extern pthread_mutex_t mutex_ticket;
extern pthread_mutex_t mutex_pid;
extern pthread_mutex_t mutex_interrupt_pcp;
extern pthread_mutex_t mutex_interrupt_plp;

extern t_list* ready;						// mutex: mutex_lista_ready
extern t_list* ready_plus;					// mutex: mutex_lista_ready_plus
extern t_list* execute;						// mutex: mutex_lista_exec
extern t_list* new;							// mutex: mutex_lista_new
extern t_list* blocked;						// mutex: mutex_lista_blocked
extern t_list* lista_exit;					// mutex: mutex_lista_exit

extern t_list* interfaces_conectadas;		// mutex: mutex_lista_interfaces
extern t_list* lista_recursos;				// PENDIENTE VER SI AGREGO MUTEX

extern sem_t sem_enviar_interrupcion;
extern sem_t sem_interrupt_pcp;
extern sem_t sem_estructura_iniciada_en_memoria;
extern sem_t sem_interrupt_plp;
extern sem_t sem_multiprogramacion;
extern sem_t sem_listas_ready;
extern sem_t sem_lista_new;
extern sem_t sem_lista_execute;
extern sem_t sem_solicitud_interfaz;
extern sem_t sem_pcp;
extern sem_t sem_cpu_libre;
extern sem_t sem_RB;
extern sem_t sem_RA;
extern sem_t sem_RC;


extern algoritmo_de_planificacion ALGORITMO_PCP_SELECCIONADO;


extern int identificador_PID;	// mutex: mutex_pid
extern int contador_pcbs;
extern int ticket_actual;		// mutex: mutex_ticket
extern int procesos_en_core;

extern int flag_respuesta_creacion_proceso;
extern int flag_interrupt_pcp;
extern int flag_interrupt_plp;


#endif