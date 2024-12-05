#include "../include/consola.h"

void iniciar_consola(){
    char* leido;

    imprimir_comandos();
    leido = readline("> ");
    bool validacion_leido;

    while(strcmp(leido, "\0") != 0){
        validacion_leido = validacion_de_instruccion_de_consola(leido);
        while(!validacion_leido){
            log_error(kernel_logger, "MÓDULO - consola: Comando de consola no reconocido");
            free(leido);
            leido = readline("> ");
            validacion_leido = validacion_de_instruccion_de_consola(leido);
        }
        atender_instruccion(leido);
        free(leido);
        leido=readline("> ");
    }
    
}

bool validacion_de_instruccion_de_consola(char* leido){
    bool resultado_validacion = false;

    char** comando_consola = string_split(leido, " "); // Vectoriza string

    if(strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "INICIAR_PROCESO") == 0){ 
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "PROCESO_ESTADO") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "COMANDO") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "ALGORITMO_PLANIFICACION") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "LISTAR_RECURSOS") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "LISTAR_RECURSOS_SISTEMA") == 0){
        resultado_validacion = true;
    }else{
        printf("Escriba la palabra COMANDO para volver a ver los comandos disponibles \n");
    }
    string_array_destroy(comando_consola);
    return resultado_validacion;
}

void atender_instruccion(char* leido){
    char** comando_consola = string_split(leido, " ");

    if(strcmp(comando_consola[0], "INICIAR_PROCESO") == 0){ 

        // [INICIAR_PROCESO] [PATH]
        // Debe crear pcb en estado NEW y agregarlo a la lista
        // Debe avisar al plp (con sem_lista_new)
        // Instrucciones a ejecutar estarán en el PATH (Deben pasarse a memoria para que las deje disponibles)

        pcb* nuevo_pcb = crear_pcb(comando_consola[1]);
        
        // CORRECCION PENDIENTE: Debería hacer free?
        list_add_pcb_sync(new,nuevo_pcb,&mutex_lista_new,NEW);
        
        log_info(kernel_logger,"MÓDULO - consola: Se ha creado proceso con PID: %d",nuevo_pcb->pid);
        log_info(kernel_logger,"MÓDULO - consola: Se ha creado proceso con path: %s",nuevo_pcb->path);
        sem_post(&sem_lista_new);

    }else if(strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0){

        // [FINALIZAR_PROCESO] [PID]
        // Debe liberar recursos, archivos y memoria
        int pid_buscado = atoi(comando_consola[1]);
        
        pcb* un_pcb = buscar_pcb_en_sistema_(pid_buscado);

        if(un_pcb == NULL){
            printf("No hay ningún proceso en el sistema que corresponda al PID \n");
        }else{
            log_info(kernel_logger,"MÓDULO - consola: Se encontró proceso con PID: %d",un_pcb->pid);
            planificar_proceso_exit_en_hilo(un_pcb);
        }

    }else if(strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0){
        
        void liberar_instruccion(char* una_instruccion){
            free(una_instruccion);
        }
        // [EJECUTAR_SCRIPT] [PATH]
        t_list* instrucciones_a_ejecutar = obtener_instrucciones_del_archivo(comando_consola[1]);
        int cantidad_instrucciones = list_size(instrucciones_a_ejecutar);

        for (int i = 0; i < cantidad_instrucciones; i++)
        {
            char* una_inst = list_get(instrucciones_a_ejecutar,i);
            atender_instruccion(una_inst);
            sleep(1);
        }

        list_destroy_and_destroy_elements(instrucciones_a_ejecutar,(void*)liberar_instruccion);

    }else if(strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0){

        // Debe pausar planificación de corto y largo plazo
        // Debe ignorar mensaje si planificación está activa
        pthread_mutex_lock(&mutex_interrupt_pcp);
        pthread_mutex_lock(&mutex_interrupt_plp);
        flag_interrupt_pcp = 0;
        flag_interrupt_plp = 0;
        pthread_mutex_unlock(&mutex_interrupt_pcp);
        pthread_mutex_unlock(&mutex_interrupt_plp);
        log_warning(kernel_logger,"Planificación detenida.");


    }else if(strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0){

        // Debe retomar planificación de corto y largo plazo
        pthread_mutex_lock(&mutex_interrupt_pcp);
        pthread_mutex_lock(&mutex_interrupt_plp);
        if(flag_interrupt_pcp == 0){
            flag_interrupt_pcp = 1;
            flag_interrupt_plp = 1;
            sem_post(&sem_interrupt_pcp);
            sem_post(&sem_interrupt_plp);
        }
        pthread_mutex_unlock(&mutex_interrupt_pcp);
        pthread_mutex_unlock(&mutex_interrupt_plp);
        log_warning(kernel_logger,"Planificación reanudada.");
        
    }else if(strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0){

        // [MULTIPROGRAMACIÓN] [VALOR]
        // Debe cambiar el grado de multiprogramación al VALOR indicado
        int* valor_solicitado = malloc(sizeof(int));
        *valor_solicitado = atoi(comando_consola[1]);
        ejecutar_en_hilo_detach((void*)modificar_grado_multiprogramacion,valor_solicitado);

        
    }else if(strcmp(comando_consola[0], "PROCESO_ESTADO") == 0){

        // Debe listar por estado, todos los procesos en ese estado
        printf("-------------------------------------------------\n");
        printf("Pocesos en NEW  \n");
        imprimir_procesos(new,&mutex_lista_new);
        printf("-------------------------------------------------\n");
        printf("Pocesos en READY \n");
        imprimir_procesos(ready,&mutex_lista_ready);
        printf("-------------------------------------------------\n");
        printf("Pocesos en READY PLUS \n");
        imprimir_procesos(ready_plus,&mutex_lista_ready_plus);
        printf("-------------------------------------------------\n");
        printf("Poceso en EXECUTE \n");
        imprimir_procesos(execute,&mutex_lista_exec);
        printf("-------------------------------------------------\n");
        printf("Pocesos en BLOCKED \n");
        imprimir_procesos(blocked,&mutex_lista_blocked);
        printf("-------------------------------------------------\n");
        printf("Pocesos en EXIT \n");
        imprimir_procesos_exit(lista_exit,&mutex_lista_exit);

    }else if(strcmp(comando_consola[0], "COMANDO") == 0){
        imprimir_comandos();
    }else if(strcmp(comando_consola[0], "LISTAR_RECURSOS") == 0){
        printf("-------------------------------------------------\n");
        printf("Pocesos en NEW  \n");
        imprimir_recursos_procesos(new,&mutex_lista_new);
        printf("-------------------------------------------------\n");
        printf("Pocesos en READY \n");
        imprimir_recursos_procesos(ready,&mutex_lista_ready);
        printf("-------------------------------------------------\n");
        printf("Pocesos en READY PLUS \n");
        imprimir_recursos_procesos(ready_plus,&mutex_lista_ready_plus);
        printf("-------------------------------------------------\n");
        printf("Poceso en EXECUTE \n");
        imprimir_recursos_procesos(execute,&mutex_lista_exec);
        printf("-------------------------------------------------\n");
        printf("Pocesos en BLOCKED \n");
        imprimir_recursos_procesos(blocked,&mutex_lista_blocked);
    }else if(strcmp(comando_consola[0], "ALGORITMO_PLANIFICACION") == 0){
        
        if(strcmp(comando_consola[1],"FIFO") == 0){

            ALGORITMO_PCP_SELECCIONADO = FIFO;
            strcpy(ALGORITMO_PLANIFICACION,"FIFO");
            log_info(kernel_logger,"Se cambió el algoritmo de planificacion a: FIFO");

        }else if(strcmp(comando_consola[1],"RR") == 0){

            ALGORITMO_PCP_SELECCIONADO = RR;
            strcpy(ALGORITMO_PLANIFICACION,"RR");
            log_info(kernel_logger,"Se cambió el algoritmo de planificacion a: RR");

        }else if(strcmp(comando_consola[1],"VRR") == 0){
            
            strcpy(ALGORITMO_PLANIFICACION,"VRR");
            ALGORITMO_PCP_SELECCIONADO = VRR;
            log_info(kernel_logger,"Se cambió el algoritmo de planificacion a: VRR");

        }else{
            log_error(kernel_logger,"No ha seleccionado un algoritmo válido");
        }
        
    }else if(strcmp(comando_consola[0], "LISTAR_RECURSOS_SISTEMA") == 0){
        int contador = 0;
        while(RECURSOS[contador] != NULL){
            printf("Recurso: %s, Instancias: %s \n",RECURSOS[contador],INSTANCIAS_RECURSOS[contador]);
            contador ++;
        }
    }else{
        log_error(kernel_logger, "Comando no reconocido"); // Con la validación no debería llegar acá
        exit(EXIT_FAILURE);
    }
    string_array_destroy(comando_consola);
    
}

void imprimir_procesos(t_list* una_lista, pthread_mutex_t* un_mutex){
    int tamanio = list_size(una_lista) - 1;
    pcb* un_pcb = NULL;
    pthread_mutex_lock(un_mutex);
    while(tamanio >= 0){
        un_pcb = list_get(una_lista,tamanio);
        tamanio --;
        printf("PID: %d \n",un_pcb->pid);
    }
    pthread_mutex_unlock(un_mutex);
}

void imprimir_procesos_exit(t_list* una_lista, pthread_mutex_t* un_mutex){
    int tamanio = list_size(una_lista) - 1;
    int* un_pid = NULL;

    pthread_mutex_lock(un_mutex);
    while(tamanio >= 0){
        un_pid = list_get(una_lista,tamanio);
        tamanio --;
        printf("PID: %d \n",*un_pid);
    }
    pthread_mutex_unlock(un_mutex);
}

t_list* procesar_archivo(const char* path_archivo){
    
    FILE* archivo = fopen(path_archivo, "rt");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    t_list* instrucciones = list_create();
    char linea_instruccion[256];
    while (fgets(linea_instruccion, sizeof(linea_instruccion), archivo)) {
        int size_linea_actual = strlen(linea_instruccion);
        if (size_linea_actual > 0 && linea_instruccion[size_linea_actual - 1] == '\n') {
            linea_instruccion[size_linea_actual - 1] = '\0'; 
        }

        char* instruccion = strdup(linea_instruccion);
        
        list_add(instrucciones, instruccion);
  }
  
  fclose(archivo); 
  return instrucciones;
}

t_list* obtener_instrucciones_del_archivo(char* path_archivo_instrucciones){
    t_list* instrucciones = list_create();
    instrucciones = procesar_archivo(path_archivo_instrucciones);
    if (!instrucciones) {
        fprintf(stderr, "No se pudo procesar el archivo de instrucciones.\n");
        return NULL;
    }
    return instrucciones;
}

void imprimir_comandos(){
    printf("Ingrese alguno de los siguientes comandos disponibles: \n");
    printf("EJECUTAR_SCRIPT         + [PATH] \n");
    printf("INICIAR_PROCESO         + [PATH] \n");
    printf("FINALIZAR_PROCESO       + [PID] \n");
    printf("DETENER_PLANIFICACIÓN \n");
    printf("INICIAR_PLANIFICACIÓN \n");
    printf("MULTIPROGRAMACIÓN       + [VALOR] \n");
    printf("PROCESO_ESTADO \n");
    printf("LISTAR_RECURSOS\n");
    printf("ALGORITMO_PLANIFICACION + [ALGORITMO]\n");
    printf("LISTAR_RECURSOS_SISTEMA\n");
}

void imprimir_recursos_procesos(t_list* una_lista, pthread_mutex_t* un_mutex){
    int tamanio = list_size(una_lista) - 1;
    pcb* un_pcb = NULL;
    pthread_mutex_lock(un_mutex);
    while(tamanio >= 0){
        un_pcb = list_get(una_lista,tamanio);
        tamanio --;
        int tamanio_lista_recursos = list_size(un_pcb->recursos_en_uso);
        if(tamanio_lista_recursos>0){
            printf("PID: %d ************************************\n",un_pcb->pid);
            for (int i = 0; i < tamanio_lista_recursos; i++)
            {
                instancia_recurso_pcb* un_recurso = list_get(un_pcb->recursos_en_uso,i);
                printf("RECURSO: %s \n",un_recurso->nombre_recurso);
                printf("Cantidad de instancias: %d \n",un_recurso->instancias_recurso);
            }
            printf("*******************************************\n");
        }
    }
    pthread_mutex_unlock(un_mutex);
}

void modificar_grado_multiprogramacion(int* valor_solicitado){

    int diferencia;
    // DEBERÍA AGREGAR ALGO PARA CHEQUEAR QUE SE ESTÁ CAMBIANDO GRADO DE MULTIPROGRAMACION
    if(*valor_solicitado>GRADO_MULTIPROGRAMACION){

        diferencia = *valor_solicitado - GRADO_MULTIPROGRAMACION;
        while(diferencia > 0){
            sem_post(&sem_multiprogramacion);
            diferencia--;
        }
        GRADO_MULTIPROGRAMACION = *valor_solicitado;
        printf("El grado de multiprogramación se ha modificado a: %d",GRADO_MULTIPROGRAMACION);

    }else if(*valor_solicitado<GRADO_MULTIPROGRAMACION && *valor_solicitado>0){

        diferencia = GRADO_MULTIPROGRAMACION - *valor_solicitado;
        while(diferencia > 0){
            sem_wait(&sem_multiprogramacion);
            diferencia--;
        }
        GRADO_MULTIPROGRAMACION = *valor_solicitado;
        printf("El grado de multiprogramación se ha modificado a: %d",GRADO_MULTIPROGRAMACION);

    }else{
        printf("El grado de multiprogramación no se ha moidificado: %d",GRADO_MULTIPROGRAMACION);
    }

    free(valor_solicitado);
}
