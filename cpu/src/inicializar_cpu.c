#include "../include/inicializar_cpu.h"

// iniciar estructuras
void inicializar_cpu() {
    iniciar_logs();
    iniciar_config();
    imprimir_config();
    inicializar_mutexs();
    inicializar_semaforos();
    tlb = crear_TLB();
    inicializar_variables();
}

void inicializar_variables(){
    respuesta_marco_escritura = NULL;
    respuesta_marco_lectura = NULL;
}

void iniciar_logs(){
    cpu_logger = log_create("CPU.log", "CPU_log", true, LOG_LEVEL_TRACE);
    if (cpu_logger == NULL){
        perror ("No se pudo crear log para CPU");
        exit(EXIT_FAILURE);
    }

    cpu_logger_extra = log_create("CPU_extra.log", "CPU_extra_log", true, LOG_LEVEL_TRACE);
    if (cpu_logger_extra == NULL){
        perror ("No se pudo crear log extra para CPU");
        exit(EXIT_FAILURE);
    }
}

void iniciar_config(){
    cpu_config = config_create("/home/utnso/so-deploy/tp-2024-1c-ubuntunel/cpu/CPU.config");
    //cpu_config = config_create("/home/utnso/Documents/tp-2024-1c-ubuntunel/cpu/CPU.config");
	if (cpu_config == NULL) {
        perror ("No se pudo crear el config");
		exit(EXIT_FAILURE);
	}

    // SE ASIGNAN LOS VALORES A VARIABLES GLOBALES A PARTIR DE ARCHIVO DE CONFIGURACIÓN
    IP_MEMORIA = config_get_string_value(cpu_config,"IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(cpu_config,"PUERTO_MEMORIA");
    PUERTO_ESCUCHA_DISPATCH = config_get_string_value(cpu_config,"PUERTO_ESCUCHA_DISPATCH");
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(cpu_config,"PUERTO_ESCUCHA_INTERRUPT");
    CANTIDAD_ENTRADAS_TLB = config_get_int_value(cpu_config,"CANTIDAD_ENTRADAS_TLB");
    ALGORITMO_TLB = config_get_string_value(cpu_config,"ALGORITMO_TLB");
}

void imprimir_config(){
    log_info(cpu_logger,"IP MEMORIA: %s",IP_MEMORIA);
    log_info(cpu_logger,"PUERTO MEMORIA: %s",PUERTO_MEMORIA);
    // Agregar los para debugear
}

void inicializar_mutexs(){
	pthread_mutex_init(&mutex_manejo_contexto, NULL);
    pthread_mutex_init(&mutex_ordenCargaGlobal, NULL);
}

void inicializar_semaforos(){
    sem_init(&sem_pedido_tamanio_pag, 0, 0);
    sem_init(&sem_pedido_instruccion, 0, 0);
    sem_init(&sem_pedido_marco, 0, 0);
    sem_init(&sem_rta_resize, 0, 0);
    sem_init(&sem_solicitud_lectura, 0, 0);
    sem_init(&sem_solicitud_escritura, 0, 0);
}

t_tlb* crear_TLB(){
    tlb = malloc(sizeof(t_tlb));
    tlb->tamanio = CANTIDAD_ENTRADAS_TLB;
    tlb->entradas = (t_tlbEntrada*) calloc(tlb->tamanio, sizeof(t_tlbEntrada));

    // pongo todas las entradas como libres
    for (int i = 0; i < tlb->tamanio; i++){
        tlb->entradas[i].estado = LIBRE;
    }

    ordenCargaGlobal = 0;

    log_trace(cpu_logger, "Creando TLB con <%d> entradas y algoritmo de reemplazo >%s> ", tlb->tamanio, ALGORITMO_TLB);

    return tlb;

}
