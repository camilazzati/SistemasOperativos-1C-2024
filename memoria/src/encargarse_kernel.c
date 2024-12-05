#include "../include/encargarse_kernel.h"


//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//FUNCIONES NECESARIAS


//-------------------------------------------------------------------------------------
void enviar_respuesta_liberar_estructuras(int resultado){
    t_paquete* un_paquete = crear_paquete_con_buffer(RTA_LIBERAR_ESTRUCTURA);
    cargar_int_a_paquete(un_paquete,resultado);
    enviar_paquete(un_paquete, fd_kernel);
    eliminar_paquete(un_paquete);
}


void liberar_estructura_proceso(t_buffer* unBuffer){
retardo_respuesta();
    int pid = extraer_int_del_buffer(unBuffer);
    log_trace(memoria_logger, "Guardando PID: %d", pid);
    t_proceso* proceso = obtener_proceso_por_pid(pid);
    int cantidad_paginas = cantidad_paginas_necesarias(proceso->size);
    int tamanio = proceso->size;

    if(list_remove_element(lista_procesos, proceso)){
        destruir_proceso(proceso);
        log_info(memoria_logger, "DESTRUCCION DE: PID: <%d> - TAMANIO: <%d> ",pid, tamanio);

    }else{
        log_error(memoria_logger, "Erorr: el proceso con el PID: <%d> no fue encontrado", pid);

    }

   // enviar_respuesta_liberar_estructuras(resultado);
}


//-------------------------------------------------------------------------------------
  // 1) saco los datos segun me mando kernel :path -> pid 
 //  2) creo proceso con el formato del struct 
//   3) lo sumo a mi litsa de procesos

void iniciar_estructura_proceso(t_buffer* unBuffer){
    retardo_respuesta();
    char* path  = extraer_string_del_buffer(unBuffer);
    log_info(memoria_logger,"Me llegó path: %s",path);
    int pid     = extraer_int_del_buffer(unBuffer);
    
    FILE* archivo = fopen(path, "rt");
    if (!archivo) {
        respuesta_negativa_enviar_kernel();
        log_error(memoria_logger, "Error al abrir el archivo: No se crea el proceso");
        exit(EXIT_FAILURE);
    }
    fclose(archivo); 

    crear_proceso_nuevo( pid,  path);
    respuesta_kernel_de_solicitud_iniciar_proceso();
    
}

void respuesta_negativa_enviar_kernel(){
    
    t_paquete* un_paquete = crear_paquete_con_buffer(RTA_INICIAR_ESTRUCTURA);
    cargar_int_a_paquete(un_paquete, 0);
    enviar_paquete(un_paquete, fd_kernel);
    eliminar_paquete(un_paquete);
}
void  respuesta_kernel_de_solicitud_iniciar_proceso(){
    
    t_paquete* un_paquete = crear_paquete_con_buffer(RTA_INICIAR_ESTRUCTURA);
    cargar_int_a_paquete(un_paquete, 1);
    enviar_paquete(un_paquete, fd_kernel);
    eliminar_paquete(un_paquete);
}

//-------------------------------------------------------------------------------------

t_list* procesar_archivo(const char* path_archivo) {
    FILE* archivo = fopen(path_archivo, "rt");
    t_list* instrucciones = list_create();

    if (archivo == NULL) {
        perror("No se encontró el archivo");
        log_error(memoria_logger, "Proceso se inicializa sin Instrucciones por error en el archivo");
        return instrucciones;
    }

    char* linea_instruccion = malloc(512 * sizeof(char)); // Tamaño suficiente para líneas largas
    while (fgets(linea_instruccion, 512, archivo)) {
        int size_linea_actual = strlen(linea_instruccion);
        if (size_linea_actual > 0 && linea_instruccion[size_linea_actual - 1] == '\n') {
            linea_instruccion[size_linea_actual - 1] = '\0'; // Eliminar el salto de línea
        }

        char** l_instrucciones = string_split(linea_instruccion, " ");
        int cantidad_parametros = 0;
        while (l_instrucciones[cantidad_parametros]) {
            cantidad_parametros++;
        }

        t_instruccion_codigo* pseudo_cod = malloc(sizeof(t_instruccion_codigo));
        pseudo_cod->pseudo_codigo = strdup(l_instrucciones[0]);
        pseudo_cod->primer_parametro = (cantidad_parametros > 1) ? strdup(l_instrucciones[1]) : NULL;
        pseudo_cod->segundo_parametro = (cantidad_parametros > 2) ? strdup(l_instrucciones[2]) : NULL;

        // Verificar y asignar hasta cinco parámetros
        if (cantidad_parametros > 3) {
            pseudo_cod->tercer_parametro = strdup(l_instrucciones[3]);
        } else {
            pseudo_cod->tercer_parametro = NULL;
        }

        if (cantidad_parametros > 4) {
            pseudo_cod->cuarto_parametro = strdup(l_instrucciones[4]);
        } else {
            pseudo_cod->cuarto_parametro = NULL;
        }

        if (cantidad_parametros > 5) {
            pseudo_cod->quinto_parametro = strdup(l_instrucciones[5]);
        } else {
            pseudo_cod->quinto_parametro = NULL;
        }

        // Concatenar todos los parámetros en una sola cadena
        char* instruccion_definitiva = string_new();
        for (int i = 0; i < cantidad_parametros; i++) {
            string_append_with_format(&instruccion_definitiva, "%s%s", l_instrucciones[i], (i < cantidad_parametros - 1) ? " " : "");
        }

        list_add(instrucciones, instruccion_definitiva);

        // Liberar memoria
        for (int i = 0; i < cantidad_parametros; i++) {
            free(l_instrucciones[i]);
        }
        free(l_instrucciones);
        free(pseudo_cod->pseudo_codigo);
        if (pseudo_cod->primer_parametro) free(pseudo_cod->primer_parametro);
        if (pseudo_cod->segundo_parametro) free(pseudo_cod->segundo_parametro);
        if (pseudo_cod->tercer_parametro) free(pseudo_cod->tercer_parametro);
        if (pseudo_cod->cuarto_parametro) free(pseudo_cod->cuarto_parametro);
        if (pseudo_cod->quinto_parametro) free(pseudo_cod->quinto_parametro);
        free(pseudo_cod);
    }

    fclose(archivo);
    free(linea_instruccion);
    return instrucciones;
}


//------------------------------------------------------------------------------------

char** dividir_cadena(const char* cadena, const char* delimitador) {
    char** resultado = NULL;
    size_t contador = 0;
    char* token = strtok((char*)cadena, delimitador);

    while (token) {
        resultado = realloc(resultado, sizeof(char*) * ++contador);
        if (!resultado) {
            fprintf(stderr, "Error al asignar memoria para dividir_cadena\n");
            exit(EXIT_FAILURE);
        }
        resultado[contador - 1] = strdup(token);
        token = strtok(NULL, delimitador);
    }
    resultado = realloc(resultado, sizeof(char*) * (contador + 1));
    resultado[contador] = NULL;
    return resultado;
}

void free_string_array(char** array) {
    if (!array)
        return;

    for (int i = 0; array[i]; ++i)
        free(array[i]);
    free(array);
}

//-------------------------------------------------------------------------------------


t_list* obtener_instrucciones_del_archivo(char* path_archivo_instrucciones){
    t_list* instrucciones = list_create();
    instrucciones = procesar_archivo(path_archivo_instrucciones);
    if (!instrucciones) {
        fprintf(stderr, "No se pudo procesar el archivo de instrucciones.\n");
        return NULL;
    }
    return instrucciones;
}

//-------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//FUNCION PRINCIPAL



void encargarse_kernel(int cliente_socket_kernel){

        int numero = 1; 
        log_info(memoria_logger, "Esperando KERNEL");
        
        while(numero){
            t_buffer* unBuffer;
            int codigo_operacion = recibir_operacion(fd_kernel);

            switch(codigo_operacion){

                case INICIAR_ESTRUCTURA: 
                unBuffer = recibir_buffer(fd_kernel);
                iniciar_estructura_proceso(unBuffer);
                destruir_buffer(unBuffer);
                log_info(memoria_logger,"Se solicitó crear proceso");
                break;

                case LIBERAR_ESTRUCTURAS:
                unBuffer = recibir_buffer(fd_kernel);
                liberar_estructura_proceso(unBuffer);
                
                destruir_buffer(unBuffer);
                break; 

            }

            free(unBuffer);
        }
    }
