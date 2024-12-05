#ifndef FS_MANEJO_ARCHIVOS
#define FS_MANEJO_ARCHIVOS

#include "../include/es_gestor.h"
#include <commons/bitarray.h>
#include <fcntl.h>       // Para O_RDWR, O_CREAT
#include <sys/mman.h>    // Para mmap, PROT_READ, PROT_WRITE, MAP_SHARED, MAP_FAILED
#include <sys/stat.h>    // Para S_IRUSR, S_IWUSR
#include <unistd.h>      // Para close, ftruncate, access
#include <dirent.h>      // Para opendir, readdir, closedir, DT_REG



void iniciar_file_system();

void crear_paths();

void hacer_ocupar_bloques(t_bitarray *bitarray, int inicio, int cantidad);
//void verificar_dir(char* path);

//t_list* listaMetadatas;
//PATHS
void crear_blocks_path();
void crear_bitmap_path();
char* crear_metadata_path(char*nombreArchivo);

void crear_archivo_de_bloques();
void crear_bitmap();

FILE *abrir_archivo_de_bloques();

void vaciar_archivo(t_metadata *metadataArchivo);
void escribir_en_bloque(uint32_t posicion, uint32_t cantidadBytesAEscribir, char *informacionAEscribir);
void sync_file(void* bloques_datos_addr, size_t tamaño_total);

//BITMAP
void assignBlock(int blockIndex);
uint32_t bitmap_encontrar_bloque_libre();
void bitmap_mostrar_por_pantalla();
void bitmap_marcar_bloque_libre(uint32_t numeroBloque);
void bitmap_marcar_bloque_ocupado(uint32_t numeroBloque);
bool bitmap_encontrar_bloques_contiguos_libres_bool(uint32_t cantidadBloques, uint32_t bloqueInicial);
uint32_t bitmap_encontrar_bloques_contiguos_libres(uint32_t cantidadBloques, uint32_t bloqueInicial);
bool bitmap_hay_bloques_contiguos_libres(uint32_t bloqueInicial, uint32_t cantidadBloquesAsignados, uint32_t cantidadBloquesNecesarios);
void verificar_los_bits_ocupado(t_bitarray *bitarray);
t_bitarray* obtener_bitarray();
void hacer_libre_bloques(t_bitarray *bitarray, int inicio, int cantidad);
void desasignar_bloques(t_metadata *metadataArchivo, uint32_t cantidadBloques);
void liberar_bloques_desde_hasta(t_bitarray* bitmap, int bloque_inicial, int bloque_final);
void extender_bloques(t_bitarray* bitmap, int cantidad_bloques_a_agregar, int primer_bloque_a_asignar);
int cantidad_bloques_libres(t_bitarray* bitmap);

//METADATAS
void recuperar_lista();
t_metadata *crear_nuevo_metadata(char *nombreArchivo);

char* metadata_get_nombre_archivo(t_metadata *metadata);

uint32_t metadata_get_tamanio_archivo(t_metadata *metadata);
void metadata_set_tamanio_archivo(t_metadata *metadata, uint32_t nuevoTamanio);

uint32_t metadata_get_bloqueInicial_archivo(t_metadata *metadata);
void metadata_set_bloqueInicial_archivo(t_metadata *metadata, uint32_t nuevoBloqueInicial);

uint32_t metadata_get_cantidad_bloques_asignados_archivo(t_metadata *metadata);
void metadata_set_cantidad_bloques_asignados_archivo(t_metadata *metadata, uint32_t nueva_cantidad_bloques_asignados);

bool crear_archivo_nuevo_metadata(t_metadata *nuevometadata, char* metadata_path);
//void recorrer_directorio_metadatas(void);
bool persistir_metadata(t_metadata* metadata, char* metadata_path);
void recuperar_lista();
uint32_t redondear_hacia_arriba(uint32_t nuevoTamanio, uint32_t tamanio);
uint32_t redondear_hacia_abajo(uint32_t numero, uint32_t tamanio);

void string_append(char** original, const char* string_to_add);
char* string_new();

#endif