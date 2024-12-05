#include <../include/fs_manejo_archivos.h>

void iniciar_file_system() 
{
	//iniciar_log();
	log_info(es_logger, "Inicializando FILE SYSTEM");

	crear_paths();

    crear_archivo_de_bloques();
    
    log_info(es_logger, "Archivo de bloques creado correctamente");

	crear_bitmap();

    bitmap_mostrar_por_pantalla();

    log_info(es_logger, "Bitmap creado correctamente");
	//crear_metadata();

    listaMetadatas = list_create();
    
    recuperar_lista();

    bitmap_mostrar_por_pantalla();

    log_info(es_logger,"ListaMetadatas creada");
	
	log_info(es_logger, "File system inicializado");
}

// PATHS-----------------------------------------------------------------------

void crear_paths() 
{
	log_info(es_logger, "Creando paths");
            
	crear_blocks_path();                       
	crear_bitmap_path();         

	log_info(es_logger, "Paths creados correctamente");
	return;
}


void crear_blocks_path() 
{
	block_path = string_new();
	string_append(&block_path, PATH_BASE_DIALFS);
	string_append(&block_path, "/bloques.dat");
}

void crear_bitmap_path() 
{
	bitmap_path = string_new();
	string_append(&bitmap_path, PATH_BASE_DIALFS);
	string_append(&bitmap_path, "/bitmap.dat");
}

char* crear_metadata_path(char* nombreArchivo) {
    metadata_path = string_new();
    string_append(&metadata_path, PATH_BASE_DIALFS);
    string_append(&metadata_path, "/");
    string_append(&metadata_path, nombreArchivo);
    log_trace(es_logger,"Soy crear_metadata_path, muestro el path: %s", metadata_path);
    return metadata_path;
}

// ARCHIVO DE BLOQUES-----------------------------------------------------------------------

void crear_archivo_de_bloques()
{
    printf("Creando Archivo de Bloques\n");

    int chequeo = access(block_path, F_OK) ;
    int fd_bloque_de_datos = open(block_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bloque_de_datos == -1) {
        perror("Error al abrir bloques.dat");
        exit(EXIT_FAILURE);
    }

    if (chequo == -1) { 
	    log_info(es_logger, "Entre a truncar");
        if (ftruncate(fd_bloque_de_datos, BLOCK_COUNT * BLOCK_SIZE) == -1) {
            perror("Error al truncar bloques.dat");
            close(fd_bloque_de_datos);
            exit(EXIT_FAILURE);
        }
    }
    
    bloques_datos_addr = malloc(BLOCK_COUNT * BLOCK_SIZE);
    puntero_auxiliar_mmap = mmap(NULL, BLOCK_COUNT * BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bloque_de_datos, 0);

    if (puntero_auxiliar_mmap == MAP_FAILED) {
        perror("Error al mapear bloques.dat");
        close(fd_bloque_de_datos);
        exit(EXIT_FAILURE);
}
}

FILE *abrir_archivo_de_bloques()
{
    archivoDeBloques = fopen(block_path, "r+b");

    if (archivoDeBloques == NULL) {
        log_error(es_logger, "No se pudo abrir el archivo.");
    }
    return archivoDeBloques;
}

void escribir_en_bloque(uint32_t posicion, uint32_t cantidadBytesAEscribir, char *informacionAEscribir)
{
    archivoDeBloques = abrir_archivo_de_bloques();
    fseek(archivoDeBloques, posicion, SEEK_SET);
    fwrite(informacionAEscribir ,sizeof(char) , cantidadBytesAEscribir, archivoDeBloques);
    fclose(archivoDeBloques);
} 

// BITMAP-------------------------------------------------------------------------------------------------

void crear_bitmap() 
{
    bitmap = malloc(sizeof(t_bitmap));
    printf("Creando el Bitmap\n");

    int fd_bitarray = open(bitmap_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bitarray == -1) {
        printf("No se pudo abrir/crear el archivo bitmap.dat\n");
        return;
    }

    bitmap->tamanio = BLOCK_COUNT / 8; 

    log_trace(es_logger, "BITMAP TAMANIO: %d", bitmap->tamanio);

    if (ftruncate(fd_bitarray, bitmap->tamanio ) == -1) {
        printf("Error al establecer el tamaño del archivo bitmap.dat\n");
        close(fd_bitarray);
        return;
    }

    bitmap->direccion = mmap(NULL, bitmap->tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitarray, 0);
    if (bitmap->direccion == MAP_FAILED) {
        printf("Error al mapear el archivo bitmap.dat en memoria\n");
        close(fd_bitarray);
        return;
    }

    memset(bitmap->direccion, 0, bitmap->tamanio);

    bitmap->bitarray = bitarray_create_with_mode(bitmap->direccion, bitmap->tamanio, LSB_FIRST);
    if (! bitmap->bitarray) {
        printf("Error al asignar memoria para el objeto bitarray\n");
        munmap(bitmap->direccion, bitmap->tamanio);
        close(fd_bitarray);
        return;
    }
    if (msync(bitmap->direccion, bitmap->tamanio, MS_SYNC) == -1) {
        printf("Error en la sincronización con msync()\n");
    }

    close(fd_bitarray);

}

// Función para obtener el bitarray
t_bitarray* obtener_bitarray()
{
    // Verifica que la variable esté inicializada
    if (bitmap == NULL) {
        log_error(es_logger, "El bitmap  no está inicializado.");
        return NULL;
    }
    // Retorna el bitarray asociado con el bitmap
    return bitmap->bitarray;
}



uint32_t bitmap_encontrar_bloque_libre()
{   
    // false = 0 --> libre
    // true = 1 --> ocupado
    uint32_t i;
    bool bloqueOcupado;
    for (i=0; i < (bitmap->tamanio * 8); i++)
    {   
        bloqueOcupado  = bitarray_test_bit(bitmap->bitarray, i);
        // Si encuentra un bloque que esté en 0 devuelve la posición de ese bloque
        if(!bloqueOcupado)
        {
            return i;
            break;
        }
    }
     // Si no encuentra un bloque libre, retorna -1
    return -1;
}


void extender_bloques(t_bitarray* bitmap, int cantidad_bloques_a_agregar, int primer_bloque_a_asignar){
    int i = primer_bloque_a_asignar;
    // No la estábamos usando -> cant_bloques_contiguos = 0;

    for(int i = primer_bloque_a_asignar; i < cantidad_bloques_a_agregar + primer_bloque_a_asignar; i++){
        bitarray_set_bit(bitmap, i);
    }
}

int cantidad_bloques_libres(t_bitarray* bitmap){
    int cantidad_libres = 0;
    int cantidad_bloques_totales = bitarray_get_max_bit(bitmap);
    for(int i = 0; i < cantidad_bloques_totales; i++){
        if(!bitarray_test_bit(bitmap, i)){
            cantidad_libres++;
        }
    }
    return cantidad_libres;
}

bool bitmap_encontrar_bloques_contiguos_libres_bool(uint32_t cantidadBloques, uint32_t bloqueInicial) {
    uint32_t bloquesLibresContiguos = 0;

    // Comenzar desde bloqueInicial y buscar hasta BLOCK_COUNT
    for (uint32_t i = bloqueInicial; i < BLOCK_COUNT; i++) {
        if (!bitarray_test_bit(bitmap->bitarray, i)) {
            if (bloquesLibresContiguos == 0) {
                bloqueInicial = i; // Iniciar secuencia desde el primer bloque libre encontrado
            }
            bloquesLibresContiguos++;
            if (bloquesLibresContiguos == cantidadBloques) {
                return true; // Se encontró suficiente espacio contiguo
            }
        } else {
            bloquesLibresContiguos = 0; // Resetea el contador si se encuentra un bloque ocupado
        }
    }

    return false; // No se encontraron bloques contiguos suficientes
}


uint32_t bitmap_encontrar_bloques_contiguos_libres(uint32_t cantidadBloques, uint32_t bloqueInicial) {
    uint32_t bloquesLibresContiguos = 0;
    uint32_t primerBloqueLibre = -1;

    // Comenzar desde el bloque inicial y buscar bloques libres contiguos
    for (uint32_t i = bloqueInicial; i < BLOCK_COUNT; i++) {
        if (!bitarray_test_bit(bitmap->bitarray, i)) {
            if (bloquesLibresContiguos == 0) {
                primerBloqueLibre = i; // Iniciar secuencia desde el primer bloque libre encontrado
            }
            bloquesLibresContiguos++;
            if (bloquesLibresContiguos == cantidadBloques) {
                return primerBloqueLibre; // Retornar el primer bloque de la secuencia
            }
        } else {
            bloquesLibresContiguos = 0; // Resetea el contador si se encuentra un bloque ocupado
        }
    }
    return -1; // No se encontraron bloques contiguos suficientes
}


// Para pruebas
void bitmap_mostrar_por_pantalla()
{   
    // false = 0 --> libre
    // true = 1 --> ocupado
    uint32_t i;
    bool bloqueLeido;
    for (i=0; i < (bitmap->tamanio * 8); i++)
    {
        bloqueLeido  = bitarray_test_bit(bitmap->bitarray, i);
        // Si encuentra un bloque que esté en 0 devuelve la posición de ese bloque
        if(bloqueLeido == 0)
        {
            log_info(es_logger, "Bloque %u: 0", i);
        }
        else
        {
            log_info(es_logger, "Bloque %u: 1", i);
        }
    }
    return;
}

void liberar_bloques_desde_hasta(t_bitarray* bitmap, int bloque_inicial, int bloque_final){
    for (int i = bloque_inicial; i < bloque_final; i++){
        bitarray_clean_bit(bitmap, i);
    }
}

void bitmap_marcar_bloque_libre(uint32_t numeroBloque) 
{
    bitarray_clean_bit(bitmap->bitarray, numeroBloque);
    // Sincronizar los cambios en el archivo y verificar que se haga de forma correcta
    if (msync(bitmap->direccion, bitmap->tamanio, MS_SYNC) == -1) {
        log_error(es_logger,"Error al sincronizar los cambios en el Bitmap");
    }
    log_info(es_logger, "marcando numero de bloque como libre en bitmap, %d", numeroBloque);
    return;
}

void bitmap_marcar_bloque_ocupado(uint32_t numeroBloque) 
{
    bitarray_set_bit(bitmap->bitarray, numeroBloque);
    // Sincronizar los cambios en el archivo y verificar que se haga de forma correcta
    if (msync(bitmap->direccion, bitmap->tamanio, MS_SYNC) == -1) {
        log_error(es_logger,"Error al sincronizar los cambios en el Bitmap");
    }

    log_info(es_logger, "Marcando numero de bloque como ocupados en bitmap, %d", numeroBloque);

    return;
}

void hacer_libre_bloques(t_bitarray *bitarray, int inicio, int cantidad) {
    for (int i = inicio; i < inicio + cantidad; i++) {
        bitarray_clean_bit(bitarray, i);
    }
}

void hacer_ocupar_bloques(t_bitarray *bitarray, int inicio, int cantidad) {
    for (int i = inicio; i < inicio + cantidad; i++) {
        bitarray_set_bit(bitarray, i);
    }
}

// ESTRUCTURA DE METADATAS----------------------------------------------------------------------------

t_metadata *crear_nuevo_metadata(char *nombre_archivo)
{

    t_metadata *metadata = malloc(sizeof(metadata));
    metadata->nombreArchivo = malloc(sizeof(char) * strlen(nombre_archivo));
    strcpy(metadata->nombreArchivo, nombre_archivo);
    metadata->TAMANIO_ARCHIVO = 0;
    metadata->cantidad_bloques_asignados = 1;

    uint32_t bloqueInicial = bitmap_encontrar_bloque_libre();

    if(bloqueInicial == -1){
        log_error(es_logger, "No se encontro bloque libre para asignarle al archivo");
        exit(EXIT_FAILURE);
    }
    else{
        metadata->BLOQUE_INICIAL = bloqueInicial;
    }

    log_warning(es_logger, "el tamanio y bloque inicial es: %d, %d", metadata->TAMANIO_ARCHIVO, bloqueInicial);
    

    return metadata;
}

char* metadata_get_nombre_archivo(t_metadata *metadata)
{
    return metadata->nombreArchivo;
}

uint32_t metadata_get_tamanio_archivo(t_metadata *metadata)
{
    return metadata->TAMANIO_ARCHIVO;
}


void metadata_set_tamanio_archivo(t_metadata *metadata, uint32_t nuevoTamanio)
{
    metadata->TAMANIO_ARCHIVO = nuevoTamanio;
    return;
}

uint32_t metadata_get_bloqueInicial_archivo(t_metadata *metadata)
{
    return metadata->BLOQUE_INICIAL;
}

void metadata_set_bloqueInicial_archivo(t_metadata *metadata, uint32_t nuevoBloqueInicial)
{
    metadata->BLOQUE_INICIAL = nuevoBloqueInicial;
    return;
}

uint32_t metadata_get_cantidad_bloques_asignados_archivo(t_metadata *metadata)
{
    return metadata->cantidad_bloques_asignados;
}

void metadata_set_cantidad_bloques_asignados_archivo(t_metadata *metadata, uint32_t nueva_cantidad_bloques_asignados)
{
    metadata->cantidad_bloques_asignados = nueva_cantidad_bloques_asignados;
    return;
}

void desasignar_bloques(t_metadata *metadataArchivo, uint32_t cantidadBloques)
{
    uint32_t bloqueInicial = metadata_get_bloqueInicial_archivo(metadataArchivo);
    uint32_t cantidadBloquesAsignados = metadata_get_cantidad_bloques_asignados_archivo(metadataArchivo);

    for (uint32_t i = cantidadBloquesAsignados - cantidadBloques; i < cantidadBloquesAsignados; i++) {
        bitmap_marcar_bloque_libre(bloqueInicial + i);
    }
    metadata_set_cantidad_bloques_asignados_archivo(metadataArchivo, cantidadBloquesAsignados - cantidadBloques);
}

void vaciar_archivo(t_metadata *metadataArchivo)
{   
    log_info(es_logger, "Se vacia el archivo.");
    uint32_t cantidadBloquesDesasignar = metadata_get_cantidad_bloques_asignados_archivo(metadataArchivo);

    // Desasignar todos los bloques directamente
    desasignar_bloques(metadataArchivo, cantidadBloquesDesasignar);

    // Resetear la metadata del archivo
    metadata_set_cantidad_bloques_asignados_archivo(metadataArchivo, 0);
    metadata_set_tamanio_archivo(metadataArchivo, 0);
}

// ARCHIVOS DE METADATA--------------------------------------------------------------------

bool crear_archivo_nuevo_metadata(t_metadata *nuevometadata, char* metadata_path)
{
    FILE *archivo;
    char *nombreArchivo = nuevometadata->nombreArchivo;  
    uint32_t tamanioArchivo = metadata_get_tamanio_archivo(nuevometadata);
    uint32_t bloqueInicialArchivo = metadata_get_bloqueInicial_archivo(nuevometadata);
    log_info(es_logger,"BLOQUE INICIAR: %d", bloqueInicialArchivo);
    //char metadata_path[PATH_MAX];
    
    // Ruta completa del archivo
    log_trace(es_logger,"muestro el path: %s", metadata_path);
    /* if(snprintf(metadata_path, sizeof(metadata_path), "%s/%s", PATH_BASE_DIALFS, nombreArchivo) < 0) {
        log_error(es_logger, "Error al construir la ruta del archivo del metadata %s.", nombreArchivo);
        free(nombreArchivo); // Liberar memoria en caso de error
        return false;
    }  */

    archivo = fopen(metadata_path, "w");
    if (archivo == NULL) {
        log_error(es_logger, "Error al crear el archivo del metadata %s.", nombreArchivo);
        free(nombreArchivo); // Liberar memoria en caso de error
        return false;
    }

    fprintf(archivo, "TAMANIO_ARCHIVO=%u\n", tamanioArchivo);
    fprintf(archivo, "BLOQUE_INICIAL=%u\n", bloqueInicialArchivo);

    int bloque_int = (int)bloqueInicialArchivo;
    int tamanio_int = (int)tamanioArchivo;

    log_warning(es_logger, "el tamanio y bloque inicial es: %d, %d", tamanioArchivo, bloqueInicialArchivo);

    log_warning(es_logger, "el tamanio y bloque inicial en INT es: %d, %d", tamanio_int, bloque_int);

    bitmap_marcar_bloque_ocupado(bloqueInicialArchivo);

    log_info(es_logger, "estoy recien creado, ocupe bloque %d", bloqueInicialArchivo);

    fclose(archivo);

    log_info(es_logger, "cree piola el archivo metadata: %s", nombreArchivo);

    // Si se pudo crear el archivo satisfactoriamente

     if (!listaMetadatas) {
        log_error(es_logger, "Lista de metadatas no inicializada.");
        return false;
    }

    log_info(es_logger,"antes de agregar a %s, tengo %d archivos en la lista", nombreArchivo, list_size(listaMetadatas));

    list_add(listaMetadatas, nuevometadata);
    
    t_metadata * metadata0 = list_get(listaMetadatas, 0);

    log_info(es_logger, "metadata nombre, %s", metadata0->nombreArchivo);

    log_info(es_logger,"despues de agregar a %s a la lista, tengo %d archivos, en la lista", nombreArchivo, list_size(listaMetadatas));

    return true;
}

// RECUPERAR DATOS DESP DE TIRAR ABAJO FS--------------------------------------------------------------

void recuperar_lista() {
    DIR *d;
    struct dirent *dir;
    d = opendir(PATH_BASE_DIALFS);
    t_metadata* metadata = NULL;
    int tamanio = 0;
    int bloque_inicial = 0;

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, "bitmap.dat") != 0 && strcmp(dir->d_name, "bloques.dat") != 0 && strcmp(dir->d_name, "..") != 0 && strcmp(dir->d_name, ".") != 0) {
                
                printf("%s\n", dir->d_name);

                char* archivo_path = string_new();  // Usar string_new para crear una nueva cadena
                string_append(&archivo_path, PATH_BASE_DIALFS);
                string_append(&archivo_path, "/");
                string_append(&archivo_path, dir->d_name);

                FILE *archivo = fopen(archivo_path, "r");
                if (archivo == NULL) {
                    log_info(es_logger, "Error abriendo el archivo");
                    exit(EXIT_FAILURE);
                }

                if (fscanf(archivo, "TAMANIO_ARCHIVO=%d\n", &tamanio) != 1) {
                    log_info(es_logger, "Error leyendo TAMANIO_ARCHIVO del archivo: %s", archivo_path);
                }

                if (fscanf(archivo, "BLOQUE_INICIAL=%d\n", &bloque_inicial) != 1) {
                    log_info(es_logger, "Error leyendo BLOQUE_INICIAL del archivo: %s", archivo_path);
                }

                log_info(es_logger, "nombre: %s, tamanioArchivo: %d, bloqueinicial %d", archivo_path, tamanio, bloque_inicial);

                metadata = malloc(sizeof(t_metadata));
                metadata->nombreArchivo = strdup(dir->d_name);
                metadata->BLOQUE_INICIAL = bloque_inicial;
                metadata->TAMANIO_ARCHIVO = tamanio;
                metadata->cantidad_bloques_asignados = tamanio / BLOCK_SIZE;

                list_add(listaMetadatas, metadata);

                fclose(archivo);
                free(archivo_path);
            }
        }
        closedir(d);
    } else {
        perror("opendir");
    }
}

// FUNCIONES AUX-------------------------------------------------------------------------------------

char* string_new() {
    char* new_string = (char*)malloc(1);
    if (new_string) {
        new_string[0] = '\0';
    }
    return new_string;
}

void string_append(char** original, const char* string_to_add) {
    if (!original || !string_to_add) {
        return;
    }
    size_t original_length = *original ? strlen(*original) : 0;
    size_t new_string_length = strlen(string_to_add);
    char* new_string = (char*)realloc(*original, original_length + new_string_length + 1);
    if (new_string) {
        memcpy(new_string + original_length, string_to_add, new_string_length + 1);
        *original = new_string;
    }
}

uint32_t redondear_hacia_arriba(uint32_t nuevoTamanio, uint32_t tamanio)
{
    uint32_t resultado = (nuevoTamanio + tamanio - 1) / tamanio;
    if ((nuevoTamanio % tamanio) == 0)
    {
        return resultado;
    }
    return (resultado+1);
}
