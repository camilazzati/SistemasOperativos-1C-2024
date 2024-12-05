#ifndef REALIZAROP_ES
#define REALIZAROP_ES

#include "../include/es_gestor.h"
#include "../include/fs_manejo_archivos.h"
#include <math.h>   
#include <commons/bitarray.h>


// =========== ATENDER KERNEL ===============

void atender_generica_es_kernel();
void atender_stdin_es_kernel();
void atender_stdout_es_kernel();
void atender_dialfs_es_kernel();

void atender_peticion_de_gen(t_buffer* un_buffer);
void atender_peticion_de_stdin(t_buffer* un_buffer);
void atender_peticion_de_stdout(t_buffer* un_buffer);
void atender_peticion_de_fs_create(t_buffer* unBuffer);
void atender_peticion_de_fs_read(t_buffer* un_buffer);
void atender_peticion_de_fs_write(t_buffer* un_buffer);
void atender_peticion_de_fs_truncate(t_buffer* un_buffer);
void atender_peticion_de_fs_delete(t_buffer* un_buffer);


void ampliar_archivo(t_metadata *metadataArchivo, uint32_t tamanioNuevo, int pid, char* nombreArchivo);
void reducir_archivo(t_metadata *metadata, uint32_t tamanioNuevo);
bool asignar_bloques_archivo(t_metadata *metadataArchivo, uint32_t cantidadBloques, uint32_t tamanioNuevo);
//void desasignar_bloques(t_metadata *metadataArchivo, uint32_t cantidadBloques);
void compactar(t_bitarray *bitarray, char* nombre_archivo_a_truncar, int tamano_nuevo, int tamano_actual);
int buscar_bloque_con_n_bloques_libres_contiguos (t_bitarray* bitmap, int cantidad_bloques_a_asignar, int bloque_inicial);
int asignar_bloques(t_bitarray* bitarray, int cantidad_bloques_a_agregar, int bloque_inicial);

void verificar_existencia_archivo(char *nombreArchivo);
t_metadata *crear_archivo(char *nombreArchivo);
char* leer_de_bloque(int posicionAbsoluta, int tamanio);

// =========== ENVIAR A KERNEL ============================

void respuesta_de_operacion_a_kernel();


// =========== ENVIAR A MEMORIA ===============

void enviar_contenido_a_memoria_stdin(int pid, int una_direccion,int tam_contenido_ingresado, char* contenido_ingresado);
void enviar_contenido_a_memoria_stdout(int pid, int una_direccion, int tamanio_direcc);
void solicitar_escritura_memoria(int direccionFisica, int cantidadBytes, char* informacion, int pid);
char* recibir_datos_a_escribir_fwrite_de_memoria(t_buffer* un_buffer);
void enviar_contenido_a_memoria_fwrite(int pid, int una_direccion, int tamanio);

int cantidad_bloques_segun_bytes(int bytes, int tamanio_bloque);
int cantidad_bloques_segun_bytes(int bytes, int tamanio_bloque);
int ultimo_bloque(int primer_bloque, int tamanio_archivo, int tamanio_bloque);



// =========== REC MEMORIA ===============

char* recibir_datos_a_escribir_de_memoria(t_buffer* un_buffer);

void mostrar_datos_leidos(char* datos_leidos, int tamanio_direcc);

t_metadata* obtener_metadata_por_nombre(t_list* listaMetadatas, char* nombre_archivo);

void* list_get_first(t_list* list);
//uint32_t redondear_hacia_arriba(uint32_t nuevoTamanio, uint32_t tamanio);
//uint32_t redondear_hacia_abajo(uint32_t numero, uint32_t tamanio);

#endif