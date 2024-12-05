#include "../include/kernel_memoria.h"

void esperar_conexiones_memoria(){
	
	fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(kernel_logger, "Conexion con MEMORIA exitosa.");

	pthread_t hilo_memoria;
    int err = pthread_create(&hilo_memoria, NULL, (void*)esperar_memoria_kernel, NULL);
    if (err!=0){
        perror("Fallo de creación de hilo_memoria(kernel)\n");
    }
    pthread_detach(hilo_memoria);
}


void esperar_memoria_kernel(){
    bool control_key = 1;
    while(control_key){
        
        int cod_op = recibir_operacion(fd_memoria);
        switch (cod_op){

            case MENSAJE:
		 	recibir_mensaje_tp0(fd_memoria,kernel_logger);
			break;
            
            case RTA_INICIAR_ESTRUCTURA:
                
                t_buffer* un_buffer = recibir_buffer(fd_memoria);
                // Si me devuelve el flag con valor igual a 0, no pudo crearse
                flag_respuesta_creacion_proceso = extraer_int_del_buffer(un_buffer);
                
                log_info(kernel_logger,"Me llegó respuesta de memoria con FLAG: %d", flag_respuesta_creacion_proceso);
                sem_post(&sem_estructura_iniciada_en_memoria);

                destruir_buffer(un_buffer);
                
			break;

            case -1:
                log_error(kernel_logger, "MEMORIA se desconecto. Terminando servidor");
                control_key = 0;
                //exit(EXIT_FAILURE);
            break;

            default:
                log_warning(kernel_logger, "Operacion desconocida de MEMORIA - Cod. op: %d",cod_op);
            break;
            
        }
    }
}

void iniciar_estructura_en_memoria(pcb* un_pcb){

    t_paquete* paquete = NULL;
    paquete = crear_paquete_con_buffer(INICIAR_ESTRUCTURA);

    cargar_string_a_paquete(paquete,un_pcb->path);
    cargar_int_a_paquete(paquete,un_pcb->pid);
 
    log_info(kernel_logger, "Solicitud de creación de proceso con PID: %d enviada a memoria",un_pcb->pid);
    log_info(kernel_logger, "Solicitud de creación de proceso con PATH: %s enviada a memoria",un_pcb->path);
    log_info(kernel_logger,"Tamaño paquete: %d",paquete->buffer->size);
    
    enviar_paquete(paquete,fd_memoria);
    destruir_paquete(paquete);

    // Espero a la respuesta de memoria
    sem_wait(&sem_estructura_iniciada_en_memoria);
    log_info(kernel_logger,"Sigo con la ejecución del PLP");
    
}

void liberar_memoria(pcb* un_pcb){
    
    t_paquete* paquete = NULL;
    paquete = crear_paquete_con_buffer(LIBERAR_ESTRUCTURAS);
    
    log_info(kernel_logger,"Liberando memoria de proceso con PID: %d",un_pcb->pid);
    cargar_int_a_paquete(paquete,un_pcb->pid);
    
    enviar_paquete(paquete,fd_memoria);

    destruir_paquete(paquete);

}