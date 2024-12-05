#include "../include/inicializar_kernel.h"

void inicializar_kernel(){
    inicializar_logs();
    inicializar_configs();
    crear_listas();
    inicializar_semaforos();
    inicializar_mutexes();
    establecer_algoritmo_seleccionado();
    inicializar_planificadores();
    establecer_recursos();
    inicializar_asistentes_de_recurso();
}

void inicializar_logs(){
    
    kernel_logger = log_create("Kernel.log", "Kernel_log", true, LOG_LEVEL_TRACE);
    if (kernel_logger == NULL){
        perror ("No se pudo crear log para el Kernel");
        exit(EXIT_FAILURE);
    }

    kernel_logger_extra = log_create("Kernel_extra.log", "Kernel_extra_log", true, LOG_LEVEL_TRACE);
    if (kernel_logger == NULL){
        perror ("No se pudo crear log extra para el Kernel");
        exit(EXIT_FAILURE);
    }
    log_trace(kernel_logger,"Logs iniciados");
}

void inicializar_configs(){
    
    kernel_config = config_create("/home/utnso/so-deploy/tp-2024-1c-ubuntunel/kernel/Kernel.config"); // para el labo
    //kernel_config = config_create("/home/utnso/Documents/tp-2024-1c-ubuntunel/kernel/Kernel.config"); //para casa

	if (kernel_config == NULL) {
        perror ("No se pudo crear el config para el módulo kernel");
		exit(EXIT_FAILURE);
	}
        PUERTO_ESCUCHA = config_get_string_value(kernel_config,"PUERTO_ESCUCHA");
        IP_MEMORIA = config_get_string_value(kernel_config,"IP_MEMORIA");
        PUERTO_MEMORIA = config_get_string_value(kernel_config,"PUERTO_MEMORIA");
        IP_CPU = config_get_string_value(kernel_config,"IP_CPU");
        PUERTO_CPU_DISPATCH = config_get_string_value(kernel_config,"PUERTO_CPU_DISPATCH");
        PUERTO_CPU_INTERRUPT = config_get_string_value(kernel_config,"PUERTO_CPU_INTERRUPT");
        ALGORITMO_PLANIFICACION = config_get_string_value(kernel_config,"ALGORITMO_PLANIFICACION");
        QUANTUM = config_get_int_value(kernel_config,"QUANTUM") * 1000; 
        log_info(kernel_logger, "QUANTUM: %d", QUANTUM);
        RECURSOS = config_get_array_value(kernel_config,"RECURSOS");
        INSTANCIAS_RECURSOS = config_get_array_value(kernel_config,"INSTANCIAS_RECURSOS");
        GRADO_MULTIPROGRAMACION = config_get_int_value(kernel_config,"GRADO_MULTIPROGRAMACION");
    
     log_trace(kernel_logger,"Config iniciada");
}

void establecer_algoritmo_seleccionado(){
    if (strcmp(ALGORITMO_PLANIFICACION, "FIFO") == 0) {
        ALGORITMO_PCP_SELECCIONADO = FIFO;
        const char* algoritmo_seleccionado= "FIFO";
        log_trace(kernel_logger,"El algoritmo seleccionado fue: %s",algoritmo_seleccionado);
    } else if (strcmp(ALGORITMO_PLANIFICACION, "RR") == 0) {
        ALGORITMO_PCP_SELECCIONADO = RR;
        const char* algoritmo_seleccionado = "RR";
        log_trace(kernel_logger,"El algoritmo seleccionado fue: %s",algoritmo_seleccionado);
    } else if (strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0) {
        ALGORITMO_PCP_SELECCIONADO = VRR;
        const char* algoritmo_seleccionado = "VRR";
        log_trace(kernel_logger,"El algoritmo seleccionado fue: %s",algoritmo_seleccionado);
    } else {
        log_error(kernel_logger_extra,"ERROR: El algoritmo seleccionado en la configuración no es válido");
    }
}


void crear_listas(){
	ready = list_create();
    ready_plus = list_create();
	execute = list_create();
	new = list_create();
	blocked = list_create();
	lista_exit = list_create();
    interfaces_conectadas = list_create();
    lista_recursos = list_create();
    log_trace(kernel_logger,"Listas creadas");
}

void inicializar_semaforos(){
	sem_init(&sem_enviar_interrupcion, 0, 0);
    sem_init(&sem_interrupt_pcp, 0, 0);
    sem_init(&sem_interrupt_plp, 0, 0);
    sem_init(&sem_estructura_iniciada_en_memoria,0,0);
    sem_init(&sem_multiprogramacion,0,GRADO_MULTIPROGRAMACION - 1);
    sem_init(&sem_listas_ready,0,0);
    sem_init(&sem_lista_new,0,0);
    sem_init(&sem_lista_execute,0,1);
    sem_init(&sem_solicitud_interfaz,0,0);
    sem_init(&sem_pcp,0,0);
    sem_init(&sem_cpu_libre,0,1);
    log_trace(kernel_logger,"Semáforos inicializados");
}

void inicializar_mutexes(){
	pthread_mutex_init(&mutex_lista_ready, NULL);
    pthread_mutex_init(&mutex_lista_ready_plus, NULL);
	pthread_mutex_init(&mutex_lista_exec, NULL);
    pthread_mutex_init(&mutex_lista_new, NULL);
    pthread_mutex_init(&mutex_lista_blocked, NULL);
    pthread_mutex_init(&mutex_lista_exit, NULL);
    pthread_mutex_init(&mutex_procesos_en_core, NULL);
    pthread_mutex_init(&mutex_lista_interfaces, NULL);
    pthread_mutex_init(&mutex_lista_recursos, NULL);

	pthread_mutex_init(&mutex_ticket, NULL);
    pthread_mutex_init(&mutex_pid, NULL);
    pthread_mutex_init(&mutex_interrupt_pcp,NULL);
    pthread_mutex_init(&mutex_interrupt_plp,NULL);
    log_trace(kernel_logger,"Mutexes inicializados");
}

 void establecer_recursos(){

    int contador = 0;
    while(INSTANCIAS_RECURSOS[contador] != NULL){
        
        instancia_recurso* un_recurso = malloc(sizeof(instancia_recurso));
        

        char* nombre = RECURSOS[contador];
        un_recurso->nombre_recurso = malloc(sizeof(char)*strlen(nombre) + 1);
        un_recurso->cantidad_instancias = atoi(INSTANCIAS_RECURSOS[contador]);
        strcpy(un_recurso->nombre_recurso,nombre);
        sem_init(&un_recurso->semaforo_recurso,0,atoi(INSTANCIAS_RECURSOS[contador]));
        sem_init(&un_recurso->semaforo_request_recurso,0,0);
        pthread_mutex_init(&un_recurso->mutex_lista_procesos_en_cola, NULL);

        un_recurso->lista_procesos_en_cola = list_create();
        
        list_add(lista_recursos,un_recurso);

        instancia_recurso* recurso = list_get(lista_recursos,contador);
        log_info(kernel_logger,"MÓDULO - inicializar_kernel: Se agregó recurso %s con %d instancias",recurso->nombre_recurso,un_recurso->cantidad_instancias);
        
        contador += 1;
    }
    int tamanio = list_size(lista_recursos);
    log_trace(kernel_logger,"Recursos establecidos: %d", tamanio);
 }

void inicializar_planificadores(){
    
    ejecutar_en_hilo_detach((void*)planificador_largo_plazo,NULL);
    ejecutar_en_hilo_detach((void*)planificador_corto_plazo,NULL);

}

void inicializar_asistentes_de_recurso(){

    int tamanio_lista_recursos = list_size(lista_recursos) - 1;
    
    pthread_mutex_lock(&mutex_lista_recursos);
    while(tamanio_lista_recursos >= 0){

        instancia_recurso* un_recurso = list_get(lista_recursos,tamanio_lista_recursos);
        ejecutar_en_hilo_detach((void*)control_request_de_recursos,un_recurso);
        tamanio_lista_recursos--;

    }
    pthread_mutex_unlock(&mutex_lista_recursos);
}
