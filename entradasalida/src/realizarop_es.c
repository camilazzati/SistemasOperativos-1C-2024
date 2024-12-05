#define _GNU_SOURCE
#include "../include/realizarop_es.h"



void atender_generica_es_kernel()
{
    int corte = 1;
    while(corte){
		t_buffer* unBuffer;
		int cod_op = recibir_operacion(fd_kernel);
    switch (cod_op) {

		case IO_GEN_SLEEP:
            log_trace(es_logger, "RECIBI ORDEN DE IO_GEN_SLEEP...");
            unBuffer = recibir_un_paquete(fd_kernel);
            atender_peticion_de_gen(unBuffer);
			break; 

           default:
			log_warning(es_logger, "Operacion desconocida GENERICA");
            corte = 0; 
			free(unBuffer);
			break;
		}
    }
}

void atender_stdin_es_kernel()
{
    int corte = 1;
    while(corte){
		t_buffer* unBuffer;
		int cod_op = recibir_operacion(fd_kernel);
    switch (cod_op) {

		    case IO_STDIN_READ:
            log_trace(es_logger, "RECIBI ORDEN DE IO_STDIN_READ...");
            unBuffer = recibir_un_paquete(fd_kernel);
            atender_peticion_de_stdin(unBuffer);
            break;

            default:
			log_warning(es_logger, "Operacion desconocida STDIN");
            corte = 0;
			free(unBuffer);
			break;
		}
    }
}

void atender_stdout_es_kernel()
{
    int corte = 1;
    while(corte){
		t_buffer* unBuffer;
		int cod_op = recibir_operacion(fd_kernel);

    switch (cod_op) {

		    case IO_STDOUT_WRITE:
            log_trace(es_logger, "RECIBI ORDEN DE IO_STDOUT_WRITE...");
            unBuffer = recibir_un_paquete(fd_kernel);
            atender_peticion_de_stdout(unBuffer);
            break;

           default:
			log_warning(es_logger, "Operacion desconocida STDOUT");
            corte = 0;
			free(unBuffer);
			break;
		}
    }
}

void atender_dialfs_es_kernel(){

    int corte = 1;
    while(corte){
		t_buffer* unBuffer;
		int cod_op = recibir_operacion(fd_kernel);
        

    switch (cod_op) {

		case IO_FS_CREATE:
            log_trace(es_logger, "RECIBI ORDEN DE IO_FS_CREATE...");
            unBuffer = recibir_un_paquete(fd_kernel);
            atender_peticion_de_fs_create(unBuffer);
            int cantidad_archivos_en_lista = list_size(listaMetadatas);
            for(int i = 0; i<cantidad_archivos_en_lista; i++){
                t_metadata* una_metadata = list_get(listaMetadatas,i);
                log_trace(es_logger,"NOMBRE ARCHIVO %d: %s, BLOQUE INICIAL: %d, TAMAÑO: %d",i,una_metadata->nombreArchivo,una_metadata->BLOQUE_INICIAL,una_metadata->TAMANIO_ARCHIVO);

            }
            bitmap_mostrar_por_pantalla();
			break; 

        case IO_FS_READ:
            log_trace(es_logger, "RECIBI ORDEN DE IO_FS_READ...");
            unBuffer = recibir_un_paquete(fd_kernel);
            atender_peticion_de_fs_read(unBuffer);
			break; 

        case IO_FS_WRITE:
            log_trace(es_logger, "RECIBI ORDEN DE IO_FS_WRITE...");
            unBuffer = recibir_un_paquete(fd_kernel);
            atender_peticion_de_fs_write(unBuffer);
			break; 

        case IO_FS_TRUNCATE:
            log_trace(es_logger, "RECIBI ORDEN DE IO_FS_TRUNCATE...");
            unBuffer = recibir_un_paquete(fd_kernel);
            atender_peticion_de_fs_truncate(unBuffer);
			break; 

         case IO_FS_DELETE:
            log_trace(es_logger, "RECIBI ORDEN DE IO_FS_DELETE...");
            unBuffer = recibir_un_paquete(fd_kernel);
            atender_peticion_de_fs_delete(unBuffer);
			break; 

           default:
			log_warning(es_logger, "Operacion desconocida DIALFS");
            corte = 0;
			free(unBuffer);
			break;
		}
    }
}



//------GEN-------------------------------------------------------

void atender_peticion_de_gen(t_buffer* un_buffer)
{

    char* nombre_interfaz = extraer_string_del_buffer(un_buffer);
    int pid = extraer_int_del_buffer(un_buffer);
    int un_tiempo = extraer_int_del_buffer(un_buffer);

    usleep(un_tiempo * TIEMPO_UNIDAD_TRABAJO*1000);

    respuesta_de_operacion_a_kernel();

    log_trace(es_logger, "EJECUCION DE IO_GEN_SLEEP FINALIZADA");
    free(un_buffer);
}

//--------------STDIN--------------------------------------------------

void atender_peticion_de_stdin(t_buffer* un_buffer)
{

    char* nombre_interfaz = extraer_string_del_buffer(un_buffer);
    int pid = extraer_int_del_buffer(un_buffer);
    uint32_t una_direccion = extraer_int_del_buffer(un_buffer);
    int tamanio_direcc = extraer_int_del_buffer(un_buffer);
    
    char contenido_ingresado[256];

    log_info(es_logger, "Ingrese el texto: ");
    fgets(contenido_ingresado, sizeof(contenido_ingresado), stdin);


    // Remover el salto de línea si es que existe
    int tam_contenido_ingresado = strlen(contenido_ingresado);
    if (tam_contenido_ingresado > 0 && contenido_ingresado[tam_contenido_ingresado-1] == '\n') {
        contenido_ingresado[tam_contenido_ingresado-1] = '\0';
    }

    tam_contenido_ingresado = strlen(contenido_ingresado); // Volver a calcular el tamaño después de sacar el salto de línea

    log_info(es_logger,"Se ingreso el texto: %s, de tamanio: %d", contenido_ingresado, tam_contenido_ingresado);


    // Compara el tamaño de la entrada con el tamaño de la dirección física
    if (tam_contenido_ingresado > tamanio_direcc) {
        // Truncar el texto
        contenido_ingresado[tamanio_direcc] = '\0';
        tam_contenido_ingresado = strlen(contenido_ingresado);
        log_info(es_logger, "Texto truncado a %d caracteres.\n", tamanio_direcc);
    } 

    enviar_contenido_a_memoria_stdin(pid, una_direccion, tam_contenido_ingresado, contenido_ingresado);
    log_trace(es_logger, "EJECUCION DE IO_STDIN_READ FINALIZADA");

    //respuesta_de_operacion_a_kernel();
}

//----STDOUT------------------------------------------------------------
void atender_peticion_de_stdout(t_buffer* un_buffer) 
{

    char* nombre_interfaz = extraer_string_del_buffer(un_buffer);
    int pid = extraer_int_del_buffer(un_buffer);
    int una_direccion = extraer_int_del_buffer(un_buffer);
    log_info(es_logger,"%d",una_direccion);
    int tamanio_direcc = extraer_int_del_buffer(un_buffer);
    log_info(es_logger,"%d",tamanio_direcc);

    enviar_contenido_a_memoria_stdout(pid, una_direccion, tamanio_direcc);


    log_trace(es_logger, "EJECUCION DE IO_STDOUT_WRITE FINALIZADA");
}


//-------------DIALFS---------------------------------------------------

 void atender_peticion_de_fs_create(t_buffer* un_buffer)
 {
    //char* nombre_interfaz = extraer_string_del_buffer(un_buffer);
    int pid = extraer_int_del_buffer(un_buffer);
	char* nombreArchivo = extraer_string_del_buffer(un_buffer);

    log_trace(es_logger,"DialFS - Crear Archivo: PID: %d - Crear Archivo: %s", pid, nombreArchivo);

    crear_archivo(nombreArchivo);

    respuesta_de_operacion_a_kernel();

    log_trace(es_logger, "EJECUCION DE IO_FS_CREATE FINALIZADA");
}

t_metadata *crear_archivo(char *nombreArchivo)
{   
    // Crear un archivo metadata correspondiente al nuevo archivo, con tamaño 0 y sin bloques asociados.
    char* metadata_path = crear_metadata_path(nombreArchivo);
    t_metadata* nuevoMetadata = crear_nuevo_metadata(nombreArchivo);
    bool archivoCreado = crear_archivo_nuevo_metadata(nuevoMetadata, metadata_path);
    bitmap_mostrar_por_pantalla();

    if (archivoCreado)
    {   
        log_info(es_logger,"Archivo creado: %s", nombreArchivo);

    } else {
        log_info(es_logger, "No pude crear el archivo: %s", nombreArchivo);
    }

    return nuevoMetadata;
    
}


// FREAD------------------------------------------------------------

void atender_peticion_de_fs_read(t_buffer* un_buffer)
{
    char* nombre_archivo = extraer_string_del_buffer(un_buffer);
    int pid = extraer_int_del_buffer(un_buffer);
    int una_direccion = extraer_int_del_buffer(un_buffer);
    int tamanio = extraer_int_del_buffer(un_buffer);
    int puntero = extraer_int_del_buffer(un_buffer);

    log_trace(es_logger, "DialFS - PID: %d - Leer Archivo: %s - Tamaño a Leer: %d - Puntero Archivo: %d", pid, nombre_archivo, tamanio, puntero);

    // Recuperar metadata del archivo
    t_metadata* metadata = obtener_metadata_por_nombre(listaMetadatas, nombre_archivo);
    if(metadata == NULL){
        log_info(es_logger, "No se encontró el metadata en listaMetadatas");
        return;
    }

    log_trace(es_logger, "Metadata encontrado <%s>, con bloque inicial: <%d> y tamaño: <%d> ", metadata->nombreArchivo, metadata->BLOQUE_INICIAL, metadata->TAMANIO_ARCHIVO);

    // Verificar que el puntero y el tamaño no excedan el tamaño del archivo
    if (puntero < 0 || puntero >= metadata->TAMANIO_ARCHIVO || tamanio <= 0 || (puntero + tamanio) > metadata->TAMANIO_ARCHIVO) {
        log_info(es_logger, "El puntero y/o el tamaño a leer están fuera de los límites del archivo");
        return;
    }

    // Calcular la posición en el archivo de bloques
    int posicionAbsoluta = (metadata->BLOQUE_INICIAL * BLOCK_SIZE) + puntero;
    log_trace(es_logger, "Posición a leer: <%u>", posicionAbsoluta);

    // Leer los datos directamente del archivo de bloques
    char *informacion = leer_de_bloque(posicionAbsoluta, tamanio);
    if (informacion == NULL) {
        log_info(es_logger, "Error al leer los datos del bloque");
        return;
    }

    log_trace(es_logger, "Datos leídos: <%s>", informacion);

    // Enviar la información a memoria para ser escrita a partir de la dirección física
    solicitar_escritura_memoria(una_direccion, tamanio, informacion, pid);

    respuesta_de_operacion_a_kernel();

    log_trace(es_logger, "EJECUCIÓN DE IO_FS_READ FINALIZADA");

    // Liberar recursos
    free(informacion);
}

char* leer_de_bloque(int posicionAbsoluta, int tamanio)
{
    char* informacionLeida = malloc(tamanio + 1);
    archivoDeBloques = abrir_archivo_de_bloques();
    fseek(archivoDeBloques, posicionAbsoluta, SEEK_SET);
    fread(informacionLeida ,sizeof(char) , tamanio, archivoDeBloques);
    informacionLeida[tamanio]='\0';
    fclose(archivoDeBloques);
    return informacionLeida;
} 

// FWRITE-----------------------------------------------------------------

void atender_peticion_de_fs_write(t_buffer* un_buffer) 
{
    //char* nombre_interfaz = extraer_string_del_buffer(un_buffer);
    char* nombre_archivo = extraer_string_del_buffer(un_buffer);
    int pid = extraer_int_del_buffer(un_buffer);
    int una_direccion = extraer_int_del_buffer(un_buffer);
    int tamanio = extraer_int_del_buffer(un_buffer);
    int puntero = extraer_int_del_buffer(un_buffer);

    enviar_contenido_a_memoria_fwrite(pid, una_direccion, tamanio);

    sem_wait(&sem_pedido_lectura);

    //char* valor_leido = recibir_datos_a_escribir_fwrite_de_memoria(un_buffer);

    log_trace(es_logger, "DialFS - PID: <%d> - Escribir Archivo: <%s> - Tamaño a Leer: <%d> - Puntero Archivo: <%d>", pid, nombre_archivo, tamanio, puntero);

    //void* valor_leido2 = malloc(tamanio);
    char* valor_a_escribir = malloc(strlen(datos_leidos_fwrite));

    memcpy(valor_a_escribir, datos_leidos_fwrite, strlen(datos_leidos_fwrite));

    t_metadata* metadata = obtener_metadata_por_nombre(listaMetadatas, nombre_archivo);
    if(metadata == NULL){
        log_info(es_logger, "No se encontro el metadata en listaMetadatas");
    } else {
        log_trace(es_logger, "Encontre el Archivo: <%s>", metadata->nombreArchivo);
    }

    uint32_t posicion = (metadata->BLOQUE_INICIAL * BLOCK_SIZE) + puntero;

    if(metadata->TAMANIO_ARCHIVO - puntero < tamanio){
        log_error(es_logger, "El tamanio a escribir es demasiado grande");
        
        valor_a_escribir[metadata->TAMANIO_ARCHIVO - puntero] = '\0';
        int tam_truncado = strlen(valor_a_escribir);
        log_info(es_logger, "Texto truncado a %d caracteres", tam_truncado);

        escribir_en_bloque(posicion, tam_truncado, valor_a_escribir);

        memcpy(valor_a_escribir, bloques_datos_addr + posicion, tam_truncado);

    } else {

        escribir_en_bloque(posicion, tamanio, valor_a_escribir);

        //memcpy(datos_leidos_fwrite, bloques_datos_addr + posicion, tamanio);
        memcpy(valor_a_escribir, bloques_datos_addr + posicion, tamanio);
    }

    msync(bloques_datos_addr, BLOCK_COUNT * BLOCK_SIZE, MS_SYNC);

    respuesta_de_operacion_a_kernel();

    log_info(es_logger, "EJECUCION DE IO_FS_WRITE FINALIZADA");

    free(valor_a_escribir);

}

//--------------------FTRUNCATE-----------------------------------------------

void atender_peticion_de_fs_truncate(t_buffer* un_buffer)
{   
    //char* nombre_interfaz = extraer_string_del_buffer(un_buffer);
    char* nombreArchivo = extraer_string_del_buffer(un_buffer);
    int pid = extraer_int_del_buffer(un_buffer);
    int tamanioNuevo = extraer_int_del_buffer(un_buffer);

    uint32_t bloquesAsignados, bloquesNuevos;
    int cantidad_met = list_size(listaMetadatas);

    usleep(TIEMPO_UNIDAD_TRABAJO * 1000);

    log_trace(es_logger,"DialFS - Truncar Archivo: PID: %d - Truncar Archivo: %s- Tamaño: %d", pid, nombreArchivo, tamanioNuevo);

    // Busco el metadata relacionado al archivo que quiero truncar

    t_metadata* metadataArchivo = obtener_metadata_por_nombre(listaMetadatas, nombreArchivo);
    if(metadataArchivo == NULL){
        log_info(es_logger, "No se encontro el metadata en listaMetadatas");
    }

    bloquesAsignados = metadata_get_cantidad_bloques_asignados_archivo(metadataArchivo);
    bloquesNuevos = ceil(tamanioNuevo / BLOCK_SIZE);

    // AMPLIAR TAMAÑO
    if (bloquesAsignados < bloquesNuevos)
    {   
        ampliar_archivo(metadataArchivo, tamanioNuevo, pid, nombreArchivo);
    }
    // REDUCIR TAMAÑO
    else if (bloquesAsignados > bloquesNuevos)
    {   
        if(bloquesNuevos == 0){

        int path_length1 = strlen(PATH_BASE_DIALFS) + strlen(nombreArchivo) + 2; // +2 para '/' y '\0'
        char *path_archivo_a_truncar = malloc(path_length1 * sizeof(char));
        snprintf(path_archivo_a_truncar, path_length1, "%s/%s", PATH_BASE_DIALFS, nombreArchivo);

        FILE *archivo = fopen(path_archivo_a_truncar, "r+");
        if (archivo == NULL) {
        log_info(es_logger, "Error abriendo el archivo");
        free(path_archivo_a_truncar);
        return false;
        }

        rewind(archivo);
        fprintf(archivo, "TAMANIO_ARCHIVO=%d\n", tamanioNuevo);
        fprintf(archivo, "BLOQUE_INICIAL=%d", metadataArchivo->BLOQUE_INICIAL);

        fclose(archivo);

        // no necesita bloques nuevos pero le cambia el tamanio al archivo
        metadata_set_tamanio_archivo(metadataArchivo, tamanioNuevo);
        } else {
            reducir_archivo(metadataArchivo, tamanioNuevo);
        }
    }
    else { // bloquesAsignados == bloquesNuevos
        // la cantidad de bloques es la misma pero el tamanio podria cambiar
        metadata_set_tamanio_archivo(metadataArchivo, tamanioNuevo);
    }

    int cantidad_archivos_en_lista = list_size(listaMetadatas);
    for(int i = 0; i<cantidad_archivos_en_lista; i++){
        t_metadata* una_metadata = list_get(listaMetadatas,i);
        log_trace(es_logger,"NOMBRE ARCHIVO %d: %s, BLOQUE INICIAL: %d, TAMAÑO: %d",i,una_metadata->nombreArchivo,una_metadata->BLOQUE_INICIAL,una_metadata->TAMANIO_ARCHIVO);
    }

    respuesta_de_operacion_a_kernel();

    log_trace(es_logger, "EJECUCION DE FSTRUNCATE REALIZADA CORRECTAMENTE");
} 

void ampliar_archivo(t_metadata *metadataArchivo, uint32_t tamanioNuevo, int pid, char* nombreArchivo)
{
    //uint32_t cantidadBloquesAsignadosActual = metadata_get_cantidad_bloques_asignados_archivo(metadataArchivo);
    int cantidadBloquesNuevos = (int)ceil(tamanioNuevo / BLOCK_SIZE);
    double resto = ((int)tamanioNuevo) % BLOCK_SIZE;

    if(resto > 0){
        cantidadBloquesNuevos++;
    }

    log_info(es_logger, "estoy en ampliar_archivo");

    if (!asignar_bloques_archivo(metadataArchivo, cantidadBloquesNuevos, tamanioNuevo)) {

        // Si no hay suficientes bloques contiguos, realizamos la compactación
        log_trace(es_logger, "DialFS - Inicio Compactación: PID: %d - Inicio Compactación.", pid);
        usleep(RETRASO_COMPACTACION * 1000);
        compactar(bitmap->bitarray, nombreArchivo, tamanioNuevo, metadata_get_tamanio_archivo(metadataArchivo));

        log_trace(es_logger, "DialFS - Fin Compactación: PID: %d - Fin Compactación.", pid);

        // Intentamos asignar bloques nuevamente después de la compactación

        //if (!asignar_bloques_archivo(metadataArchivo, tamanioNuevoEnBloques)) {

         //   log_error(es_logger, "No se pudo asignar bloques contiguos después de la compactación.");
         //   return;
        //} 
    }

    metadata_set_tamanio_archivo(metadataArchivo, tamanioNuevo);
    metadata_set_cantidad_bloques_asignados_archivo(metadataArchivo, cantidadBloquesNuevos);
    
    int cantidad_archivos_en_lista = list_size(listaMetadatas);
    for(int i = 0; i<cantidad_archivos_en_lista; i++){
        t_metadata* una_metadata = list_get(listaMetadatas,i);
        log_trace(es_logger,"NOMBRE ARCHIVO %d: %s, BLOQUE INICIAL: %d, TAMAÑO: %d",i,una_metadata->nombreArchivo,una_metadata->BLOQUE_INICIAL,una_metadata->TAMANIO_ARCHIVO);
    }

} 


void reducir_archivo(t_metadata *metadataArchivo, uint32_t tamanioNuevo)
{

    log_info(es_logger, "estoy en reducir_archivo");
    uint32_t cantidadBloquesDesasignar, cantidadBloquesAsignadosActual, tamanioNuevoEnBloques;
    if (tamanioNuevo == 0)
    {
        vaciar_archivo(metadataArchivo);
        return;
    }
    
    cantidadBloquesAsignadosActual = metadata_get_cantidad_bloques_asignados_archivo(metadataArchivo);
    tamanioNuevoEnBloques = ceil(tamanioNuevo / BLOCK_SIZE);
    cantidadBloquesDesasignar = cantidadBloquesAsignadosActual - tamanioNuevoEnBloques;
    
    desasignar_bloques(metadataArchivo, cantidadBloquesDesasignar);

    metadata_set_tamanio_archivo(metadataArchivo, tamanioNuevo);
}

bool asignar_bloques_archivo(t_metadata *metadataArchivo, uint32_t cantidadBloquesNuevos, uint32_t tamanioNuevo)
{
    uint32_t bloqueInicial = metadata_get_bloqueInicial_archivo(metadataArchivo);
    uint32_t cantidadBloquesAsignadosActual = metadata_get_cantidad_bloques_asignados_archivo(metadataArchivo);
    uint32_t bloqueFinal = bloqueInicial + cantidadBloquesAsignadosActual - 1;

    int bloquesNecesarios = cantidadBloquesNuevos - cantidadBloquesAsignadosActual;

    // Verificar si los bloquesNecesarios están contiguos y libres
    for (int i = bloqueFinal + 1; i <= bloqueFinal + bloquesNecesarios; i++) {
        if (bitarray_test_bit(bitmap->bitarray, i)) {
            log_info(es_logger, "Bloque %d ocupado, no se pueden asignar bloques necesarios", i);
            return false;
        }
    }

    // Si todos los bloques están libres, los asignamos
    for (int i = bloqueFinal + 1; i <= bloqueFinal + bloquesNecesarios; i++) {
        bitarray_set_bit(bitmap->bitarray, i);
    }

    // Actualizar el archivo de metadata y el tamaño del archivo
    metadata_set_cantidad_bloques_asignados_archivo(metadataArchivo, cantidadBloquesNuevos);

    int path_length1 = strlen(PATH_BASE_DIALFS) + strlen(metadataArchivo->nombreArchivo) + 2; // +2 para '/' y '\0'
    char *path_archivo_a_truncar = malloc(path_length1 * sizeof(char));
    snprintf(path_archivo_a_truncar, path_length1, "%s/%s", PATH_BASE_DIALFS, metadataArchivo->nombreArchivo);

    FILE *archivo = fopen(path_archivo_a_truncar, "r+");
    if (archivo == NULL) {
        log_info(es_logger, "Error abriendo el archivo");
        free(path_archivo_a_truncar);
        return false;
    }

    rewind(archivo);
    fprintf(archivo, "TAMANIO_ARCHIVO=%d\n", tamanioNuevo);
    fprintf(archivo, "BLOQUE_INICIAL=%d", metadataArchivo->BLOQUE_INICIAL);

    fclose(archivo);
    free(path_archivo_a_truncar);

    log_info(es_logger, "no se necesita compactacion, bloques actualizados");

    return true;
}


void compactar(t_bitarray *bitarray, char* nombre_archivo_a_truncar, int tamanio_nuevo, int tamanio_actual)
{
    char* nombre_archivo;
    int bloque_inicial;
    int bloques_libres;
    int tamanio;
    int bloques_ocupados;
    int cantidad_archivos_en_lista = list_size(listaMetadatas); 
    int cant_archivos_leidos = 0;
    int bloque_fin_ocupado;

    for(int i = 0; i<cantidad_archivos_en_lista; i++){
        t_metadata* una_metadata = list_get(listaMetadatas,i);
        log_trace(es_logger,"NOMBRE ARCHIVO %d: %s",i,una_metadata->nombreArchivo);
    }

    //int tamanio_bloques_ocupados;
    int path_length1 = strlen(PATH_BASE_DIALFS) + strlen(nombre_archivo_a_truncar) + 2; // +2 para '/' y '\0'
    char *path_archivo_a_truncar = malloc(path_length1 * sizeof(char));
    snprintf(path_archivo_a_truncar, path_length1, "%s/%s", PATH_BASE_DIALFS, nombre_archivo_a_truncar);

    log_info(es_logger, "nombre de archivo a truncar: %s", nombre_archivo_a_truncar);

    // ASIGNO MEMORIA DE TAMAÑO VIEJO AL ARCHIVO NUEVO
    void* buffer_archivo = malloc(tamanio_actual);

    t_metadata * metadataTruncado;

    // ENCUENTRO ARCHIVO A TRUNCAR EN LA LISTA METADATAS
    for (int i = 0; i < cantidad_archivos_en_lista; i++) {
        t_metadata *nuevometadata = list_get(listaMetadatas, i); 
        
        if (strcmp(nuevometadata->nombreArchivo, nombre_archivo_a_truncar) == 0) {
            metadataTruncado = list_remove(listaMetadatas, i);
            cantidad_archivos_en_lista--;
        }
    }

    // COPIA EN BUFFER_ARCHIVO EL ARHICVO A TRUNCAR (VERSION VIEJA)
    memcpy(buffer_archivo, bloques_datos_addr + metadataTruncado->BLOQUE_INICIAL * BLOCK_SIZE, tamanio_actual);

    // CREO LISTA TEMPORAL PARA REACOMODAR
    t_list* lista_temp = list_create();

    log_info(es_logger, "Liberando todos los bits en el bitarray.");

    // LIBERO BITARRAY PARA ACTUALIZARLO
    hacer_libre_bloques(bitarray, 0, BLOCK_COUNT);

    // CREO BUFFER TEMPORAL PARA REACOMODAR FILESYSTEM
    void* buffer_auxiliar = malloc(BLOCK_COUNT * BLOCK_SIZE);

    uint32_t desplazamiento_en_bloques = 0;
    int i = 0;
    // REACOMODO ARCHIVOS EN LISTA GLOBAL (NO CONTIENE ARCHIVO A TRUNCAR)
    while(cantidad_archivos_en_lista > cant_archivos_leidos){

        t_metadata *nuevometadata = list_get(listaMetadatas, i); 

        // OBTENGO PATH ABSOLUTO
        int path_length2 = strlen(PATH_BASE_DIALFS) + strlen(nuevometadata->nombreArchivo) + 2;
        char *path_archivo = malloc(path_length2 * sizeof(char));
        snprintf(path_archivo, path_length2, "%s/%s", PATH_BASE_DIALFS, nuevometadata->nombreArchivo); 

        log_info(es_logger, "Archivo a mover: %s", nuevometadata->nombreArchivo);

        // ABRO ARCHIVO
        FILE *archivo = fopen(path_archivo, "r+");
        if (archivo == NULL) {
            log_info(es_logger,"Error abriendo el archivo");
            free(path_archivo);
            continue; // O manejar el error de otra manera adecuada
        }

        log_warning(es_logger, "BloqueInicial antes de leer fscan: %d", nuevometadata->BLOQUE_INICIAL);
        log_warning(es_logger, "Tamanio antes de leer fscan: %d", nuevometadata->TAMANIO_ARCHIVO);

        // PASO A BUFFER_AUXILIAR EL ARCHIVO QUE OBTUVE ANTES
        memcpy(buffer_auxiliar + desplazamiento_en_bloques * BLOCK_SIZE, bloques_datos_addr + nuevometadata->BLOQUE_INICIAL * BLOCK_SIZE , nuevometadata->TAMANIO_ARCHIVO);
        
        // SACO CANT. BLOQUES QUE OCUPA EL ARCHIVO
        if(nuevometadata->TAMANIO_ARCHIVO < BLOCK_SIZE) {
            bloques_ocupados = 1;
        }else{
            bloques_ocupados = (int)ceil(nuevometadata->TAMANIO_ARCHIVO / BLOCK_SIZE);
            double resto = ((int)nuevometadata->TAMANIO_ARCHIVO) % BLOCK_SIZE;

            if(resto > 0){
                bloques_ocupados++;
            }
        }

        log_info(es_logger, "Ocupando bloques en bitmap.dat");

        // BUSCO PRIMER BLOQUE LIBRE (RECORDAR QUE LO LIMPIÉ ANTERIORMENTE)
        uint32_t j = bitmap_encontrar_bloque_libre();
        int numeroBloqueInicial = (int)j;

        //Volver al incio de archivo
        rewind(archivo);

        fprintf(archivo, "TAMANIO_ARCHIVO=%d\n", nuevometadata->TAMANIO_ARCHIVO);
        fprintf(archivo, "BLOQUE_INICIAL=%d", numeroBloqueInicial);

        log_info(es_logger, "Bloque incial de archivo %s: %i", nuevometadata->nombreArchivo, numeroBloqueInicial);
        
        // ACTUALIZO BLOQUE INICIAL
        metadata_set_bloqueInicial_archivo(nuevometadata, numeroBloqueInicial);

        // RECORRO Y ACTUALIZO BITARRAY
        for(int i = 0 ; i < bloques_ocupados; i++ ){
            bitarray_set_bit(bitarray, j);
            bloques_libres = bloques_libres - 1;
            j++;
        }

        free(path_archivo);
        fclose(archivo);

        log_info("la cantidad de bloques es %d, ",nuevometadata->TAMANIO_ARCHIVO/BLOCK_SIZE);
        
        // ACTUALIZO DESPLAZAMIENTO EN BLOQUES
        desplazamiento_en_bloques += cantidad_bloques_segun_bytes(nuevometadata->TAMANIO_ARCHIVO, BLOCK_SIZE);

        log_info("el desplazamiento es %d, ",desplazamiento_en_bloques);

        // AGREGO ARCHIVO A LA LISTA TEMPORAL
        list_add(lista_temp, nuevometadata);
        cant_archivos_leidos++;
        i++;
    }

    log_info(es_logger, "Archivo a mover: %s", nombre_archivo_a_truncar);

    // TRAIGO ARCHIVO A TRUNCAR
    FILE *archivo_a_truncar = fopen(path_archivo_a_truncar, "r+");
    if (archivo_a_truncar == NULL) {
        log_info(es_logger,"Error abriendo el archivo");
        exit(EXIT_FAILURE);
    }

    buffer_archivo = realloc(buffer_archivo,tamanio_nuevo);
    memcpy(buffer_auxiliar + desplazamiento_en_bloques * BLOCK_SIZE, buffer_archivo, tamanio_nuevo);
    desplazamiento_en_bloques += cantidad_bloques_segun_bytes(tamanio_nuevo, BLOCK_SIZE);

    // ACTUALIZO BITARRAY DE ARCHIVO TRUNCADO
    if(tamanio_nuevo < BLOCK_SIZE) {
            bloques_ocupados = 1;
        }else{
            bloques_ocupados = (int)ceil(tamanio_nuevo/ BLOCK_SIZE);
            double resto = ((int)tamanio_nuevo) % BLOCK_SIZE;

            if(resto > 0){
                bloques_ocupados++;
            }
        }

        uint32_t h = bitmap_encontrar_bloque_libre();

        int num = (int)h; 
        metadata_set_bloqueInicial_archivo(metadataTruncado, num);

        for(int i = 0 ; i < bloques_ocupados; i++ ){
            bitarray_set_bit(bitarray, num);
            bloques_libres = bloques_libres - 1;
            num++;
        }


    log_info(es_logger, "Bloque Inicial de archivo despues de mover: %i", h);

    log_info(es_logger, "Tamanio de archivo despues de mover: %i", tamanio_nuevo);
    
    rewind(archivo_a_truncar);
    fprintf(archivo_a_truncar, "TAMANIO_ARCHIVO=%i\n", tamanio_nuevo);
    fprintf(archivo_a_truncar, "BLOQUE_INICIAL=%i\n", h);

    char* puntero_destino = bloques_datos_addr;
    char* puntero_origen = buffer_auxiliar;

    memcpy(puntero_destino, puntero_origen, BLOCK_SIZE * BLOCK_COUNT);
    msync(puntero_auxiliar_mmap, BLOCK_COUNT * BLOCK_SIZE, MS_SYNC);

        list_add(lista_temp, metadataTruncado);

        log_info(es_logger, "bloques ocupados: %d", bloques_ocupados);

        list_clean(listaMetadatas);

        list_add_all(listaMetadatas,lista_temp);

        log_info(es_logger,"la cantidad de archivos en lista luego de compactar es: %d", list_size(listaMetadatas));

    fclose(archivo_a_truncar);
    free(path_archivo_a_truncar);

    cantidad_archivos_en_lista = list_size(listaMetadatas);
    for(int i = 0; i<cantidad_archivos_en_lista; i++){
        t_metadata* una_metadata = list_get(listaMetadatas,i);
        log_trace(es_logger,"NOMBRE ARCHIVO %d: %s, BLOQUE INICIAL: %d, TAMAÑO: %d",i,una_metadata->nombreArchivo,una_metadata->BLOQUE_INICIAL,una_metadata->TAMANIO_ARCHIVO);

    }

    for(int i = 0; i<cantidad_archivos_en_lista; i++){
        t_metadata* una_metadata = list_get(lista_temp,i);
        log_trace(es_logger,"NOMBRE ARCHIVO %d: %s",i,una_metadata->nombreArchivo);
    }
    bitmap_mostrar_por_pantalla();
}

//-----------FS DELETE---------------------------------------------------

void atender_peticion_de_fs_delete(t_buffer* un_buffer)
{
    //char* nombre_interfaz = extraer_string_del_buffer(un_buffer);
    char* nombre_archivo = extraer_string_del_buffer(un_buffer);
    int pid = extraer_int_del_buffer(un_buffer);

    t_metadata* metadataArchivo = obtener_metadata_por_nombre(listaMetadatas, nombre_archivo);
    if(metadataArchivo == NULL){
        log_info(es_logger, "No se encontro el metadata en listaMetadatas");
    }

    int path_length1 = strlen(PATH_BASE_DIALFS) + strlen(nombre_archivo) + 2; // +2 para '/' y '\0'
    char *path_archivo_delete = malloc(path_length1 * sizeof(char));
    snprintf(path_archivo_delete, path_length1, "%s/%s", PATH_BASE_DIALFS, nombre_archivo);

    int ultimo_bloque_archivo = ultimo_bloque(metadataArchivo->BLOQUE_INICIAL , metadataArchivo->TAMANIO_ARCHIVO, BLOCK_SIZE);

    liberar_bloques_desde_hasta(bitmap->bitarray, metadataArchivo->BLOQUE_INICIAL, ultimo_bloque_archivo);

    int cantidad_archivos_en_lista = list_size(listaMetadatas);

    for (int i = 0; i < cantidad_archivos_en_lista; i++) {
        t_metadata* una_metadata = list_get(listaMetadatas, i); 
        
        if (strcmp(una_metadata->nombreArchivo, nombre_archivo) == 0) {
            list_remove(listaMetadatas, i); 
            free(una_metadata->nombreArchivo);
            free(una_metadata);
            cantidad_archivos_en_lista --;
            i--; 
        }
    }

    if (remove(path_archivo_delete) == 0) {
            log_info(es_logger, "PID: <%d> - Eliminar Archivo: <%s>", pid, nombre_archivo);
            } 
            else {
                log_error(es_logger, "Error al eliminar el archivo %s", nombre_archivo);
            }

    cantidad_archivos_en_lista = list_size(listaMetadatas);
    for(int i = 0; i<cantidad_archivos_en_lista; i++){
        t_metadata* una_metadata = list_get(listaMetadatas,i);
        log_trace(es_logger,"NOMBRE ARCHIVO %d: %s, BLOQUE INICIAL: %d, TAMAÑO: %d",i,una_metadata->nombreArchivo,una_metadata->BLOQUE_INICIAL,una_metadata->TAMANIO_ARCHIVO);
    }
    bitmap_mostrar_por_pantalla();
    
    respuesta_de_operacion_a_kernel();
}



int ultimo_bloque(int primer_bloque, int tamanio_archivo, int tamanio_bloque){

    int cantidad_bloques = cantidad_bloques_segun_bytes(tamanio_archivo, tamanio_bloque);
    return primer_bloque + cantidad_bloques;//Preguntar si debería restar 1 -> Creo que no porque en liberar_bloques_desde_hasta uso < y no <=
}

int cantidad_bloques_segun_bytes(int bytes, int tamanio_bloque){
    return bytes == 0 ? 1 : ceil((double) bytes / tamanio_bloque);
}

//-----RESPUESTAS DE OPERACIONES PARA KERNEL ------------------------------------

 void respuesta_de_operacion_a_kernel()
 {
    log_warning(es_logger, "Enviando respuesta a Kernel");
    t_paquete* un_paquete = crear_paquete_con_buffer(RESPUESTA_INSTRUCCION_KES);
    cargar_string_a_paquete(un_paquete, nombre_interfaz_global);
    cargar_int_a_paquete(un_paquete, OK);
    enviar_paquete(un_paquete, fd_kernel);
    eliminar_paquete(un_paquete);
 }

//----- ENVIAR A MEMORIA -----------------------------------

void enviar_contenido_a_memoria_stdin(int pid, int una_direccion,int tam_contenido_ingresado, char* contenido_ingresado)
{
    t_paquete* un_paquete = crear_paquete_con_buffer(ENVIO_RECURSOS_STDIN_ESM);
	cargar_int_a_paquete(un_paquete,pid);
	cargar_int_a_paquete(un_paquete,una_direccion);
    cargar_int_a_paquete(un_paquete, tam_contenido_ingresado);
	cargar_string_a_paquete(un_paquete, contenido_ingresado);
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
}

void enviar_contenido_a_memoria_stdout(int pid, int una_direccion, int tamanio_direcc)
{
    t_paquete* un_paquete = crear_paquete_con_buffer(ENVIO_RECURSOS_STDOUT_ESM);
	cargar_int_a_paquete(un_paquete,pid);
	cargar_int_a_paquete(un_paquete,una_direccion);
    cargar_int_a_paquete(un_paquete, tamanio_direcc);
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
}

void enviar_contenido_a_memoria_fwrite(int pid, int una_direccion, int tamanio)
{
    t_paquete* un_paquete = crear_paquete_con_buffer(ENVIO_RECURSOS_FWRITE_ESM);
	cargar_int_a_paquete(un_paquete,pid);
	cargar_int_a_paquete(un_paquete,una_direccion);
    cargar_int_a_paquete(un_paquete, tamanio);
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
}

// Enviar información a Memoria para que la escriba en la dirección física. 
void solicitar_escritura_memoria(int direccionFisica, int cantidadBytes, char* informacion, int pid)
{
    
    t_paquete* un_paquete = crear_paquete_con_buffer(ENVIO_RECURSOS_FREAD_ESM);
    cargar_int_a_paquete(un_paquete,pid);
	cargar_int_a_paquete(un_paquete, direccionFisica);
    cargar_int_a_paquete(un_paquete, cantidadBytes);
    cargar_string_a_paquete(un_paquete, informacion);
    
    enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
    return;
}


//----- RECIBIR DE MEMORIA -----------------------------------

char* recibir_datos_a_escribir_de_memoria(t_buffer* un_buffer) 
{
    return extraer_string_del_buffer(un_buffer);
}

char* recibir_datos_a_escribir_fwrite_de_memoria(t_buffer* un_buffer) 
{
    return extraer_string_del_buffer(un_buffer);
}

//---------------------------------------------------------------------

void mostrar_datos_leidos(char* datos_leidos, int tamanio_direcc) 
{
    // Calcular el tamaño total necesario para el mensaje concatenado
    int tamanio_total = 0;
    int pos = 0;
    
    while (pos < tamanio_direcc) {
        int mensaje_len = strlen(datos_leidos + pos);
        tamanio_total += mensaje_len;
        pos += mensaje_len + 1; // +1 para saltar el carácter nulo
    }
    
    // Crear un buffer para el mensaje concatenado, incluyendo espacios y el carácter nulo final
    char* mensaje_concatenado = malloc(tamanio_total + 1 + (tamanio_total - 1)); // Espacios entre mensajes y carácter nulo
    if (mensaje_concatenado == NULL) {
        log_error(es_logger, "Error al mostrar datos leidos de memoria.\n");
        return;
    }

    // Concatena los mensajes
    mensaje_concatenado[0] = '\0'; 
    pos = 0;
    
    while (pos < tamanio_direcc) {
        int mensaje_len = strlen(datos_leidos + pos);
        if (pos != 0) {
            strcat(mensaje_concatenado, " "); // Añadir un espacio entre mensajes
        }
        strcat(mensaje_concatenado, datos_leidos + pos);
        pos += mensaje_len + 1; 
    }

    log_info(es_logger, "%s\n", mensaje_concatenado);

    free(mensaje_concatenado);
}


t_metadata* obtener_metadata_por_nombre(t_list* lista_metadata, char* nombre_archivo){


	bool coincide_nombre (t_metadata* una_metadata){
        log_info(es_logger,"Busco: %s",nombre_archivo);
        log_info(es_logger,"Encuentro: %s",una_metadata->nombreArchivo);
		return strcmp(nombre_archivo,una_metadata->nombreArchivo) == 0;
	}

    log_info(es_logger,"Busco: %s",nombre_archivo);
    t_metadata* metadata_archivo = NULL;
	
	if(list_any_satisfy(lista_metadata,(void*)coincide_nombre)){
		metadata_archivo = list_find(lista_metadata,(void*)coincide_nombre);
	}else{
        log_error(es_logger,"No Encontré a la metadata del archivo con nombre: %s", nombre_archivo);
        return NULL;
    }

    log_info(es_logger,"Encontré a la metadata del archivo con nombre: %s", metadata_archivo->nombreArchivo);

	return metadata_archivo;
}


int asignar_bloques(t_bitarray* bitmap, int cantidad_bloques_a_agregar, int bloque_inicial){//Bloque inicial es bloque desde donde voy a asignar y no tu último bloque

    if(cantidad_bloques_a_agregar == 0)
        return bloque_inicial;

    int bloque_inicial_nuevo = bloque_inicial;
    int bloque_desde_donde_extender = buscar_bloque_con_n_bloques_libres_contiguos(bitmap, cantidad_bloques_a_agregar, bloque_inicial);

    if(

        bloque_desde_donde_extender != -1 &&
        bloque_desde_donde_extender == bloque_inicial // Si no son iguales, significa que no se puede extender desde el bloque inicial

    ){

        extender_bloques(bitmap, cantidad_bloques_a_agregar , bloque_inicial);

    } else {

        int cantidad_bloques_sin_asignar = cantidad_bloques_libres(bitmap);

        if(cantidad_bloques_sin_asignar >= cantidad_bloques_a_agregar){

           return -2; //Si hay suficientes bloques libres pero no contiguos, retorna -2.

        } else {

           return -1; //Si no hay suficientes bloques libres, retorna -1.

        }
    }

    return bloque_inicial_nuevo;
}

int buscar_bloque_con_n_bloques_libres_contiguos (t_bitarray* bitmap, int cantidad_bloques_a_asignar, int bloque_inicial)
{
    int cant_bloques_contiguos = 0;
    
    for (int bloque_actual = bloque_inicial; bloque_actual < bitarray_get_max_bit(bitmap); bloque_actual++) {
        if (!bitarray_test_bit(bitmap, bloque_actual)) {
            cant_bloques_contiguos++;

            if (cant_bloques_contiguos == cantidad_bloques_a_asignar)return bloque_actual - cantidad_bloques_a_asignar + 1;
        }
        else cant_bloques_contiguos = 0;
    }
    return -1; //No hay bloques contiguos
}
