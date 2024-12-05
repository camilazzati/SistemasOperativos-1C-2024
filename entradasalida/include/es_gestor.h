#ifndef ES_H_
#define ES_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


#include <utils/include/shared.h> 




extern t_list* listaMetadatas;
//extern t_list *lista_archivos;

extern char* nombre_interfaz_global;
struct metadata
{
    char* nombreArchivo;
    uint32_t BLOQUE_INICIAL;
    uint32_t TAMANIO_ARCHIVO;
    uint32_t cantidad_bloques_asignados;
};
typedef struct metadata t_metadata;

struct bitmap
{
    char *direccion;
    uint32_t tamanio;
    t_bitarray *bitarray;
};
typedef struct bitmap t_bitmap;

// VARIABLES GLOBALES

// LOGS Y CONFIGS
extern t_log* es_logger;
extern t_log* es_logger_extra;

extern t_config* es_config;

// Variables de config
extern char* TIPO_INTERFAZ;
extern int TIEMPO_UNIDAD_TRABAJO;

extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;

extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;

extern char* PATH_BASE_DIALFS;
extern int BLOCK_SIZE;
extern int BLOCK_COUNT;
extern int RETRASO_COMPACTACION;


//PATHS
extern char* block_path;
extern char* bitmap_path;
extern char* metadata_path;

// FILE DESCRIPTORS
extern int fd_kernel;
extern int fd_memoria;


typedef struct {
    char* nombre_interfaz;
    char* tipo_interfaz;
    int tiempo_unidad_trabajo;
    char* ip_kernel;
    int puerto_kernel;
} configuracion_interfaz;

extern char* datos_leidos_fwrite;
extern int tamanio_direcc_fwrite;

extern sem_t sem_pedido_lectura;


extern FILE* archivoDeBloques;
extern t_bitmap *bitmap;
extern uint32_t tamanioBloques;
extern void* bloques_datos_addr;
extern char* puntero_auxiliar_mmap;
extern char* block_path;
extern char* metadata_path;
extern char* bitmap_path;

#endif