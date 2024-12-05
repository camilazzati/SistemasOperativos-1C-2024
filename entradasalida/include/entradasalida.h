#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include "../include/es_gestor.h"
#include "../include/inicializar_es.h"
#include "../include/comunicaciones_es.h"
#include "../include/realizarop_es.h"
#include <readline/readline.h>
#include <utils/include/shared.h>
#include <pthread.h>
#include <readline/readline.h>

// VARIABLES GLOBALES
t_log* es_logger;
t_log* es_logger_extra;


t_list* listaMetadatas;
char* nombre_interfaz_global;
t_config* es_config;

int fd_kernel;
int fd_memoria;

// Variables de config
char* TIPO_INTERFAZ;
int TIEMPO_UNIDAD_TRABAJO;

char* IP_KERNEL;
char* PUERTO_KERNEL;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;

char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;
int RETRASO_COMPACTACION;


void enviar_handshake_generica(char* nombre_interfaz);
void enviar_handshake_stdout(char* nombre_interfaz);
void enviar_handshake_stdin(char* nombre_interfaz);
void enviar_handshake_dialfs(char* nombre_interfaz);

char* datos_leidos_fwrite;
int tamanio_direcc_fwrite;

sem_t sem_pedido_lectura;

FILE* archivoDeBloques;
t_bitmap *bitmap;
uint32_t tamanioBloques;
void* bloques_datos_addr;
char* puntero_auxiliar_mmap;
char* block_path;
char* metadata_path;
char* bitmap_path;

#endif