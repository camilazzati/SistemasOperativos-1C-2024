# include "../include/inicializar_memoria.h"

void inicializar_memoria(){
    inicializar_logs();
    iniciar_config();
    imprimir_config();
}


//-------------------------------------------------------------------------------------------------------
///// FUNCIONES PARA INICIAIZAR

void inicializar_logs(){
    // INICIA MEMORIA
    memoria_logger = log_create("Memoria.log", "Memoria_log", 1, LOG_LEVEL_TRACE);
    if (memoria_logger == NULL){
        perror ("Error: No se pudo crear log para la memoria");
        exit(EXIT_FAILURE);
    }

    memoria_logger_extra = log_create("Memoria_extra_log.log", "Memoria_extra_log", true, LOG_LEVEL_TRACE);
    if (memoria_logger == NULL){
        perror ("No se pudo crear log extra para la memoria");
        exit(EXIT_FAILURE);
    }
}


void iniciar_config(){

    memoria_config = config_create("/home/utnso/so-deploy/tp-2024-1c-ubuntunel/memoria/Memoria.config"); // para el labo
    //memoria_config = config_create("/home/utnso/Documents/tp-2024-1c-ubuntunel/memoria/Memoria.config"); //para casa
    if (memoria_config == NULL) {
        perror ("Error: No se pudo crear el config para la memoria");
		exit(EXIT_FAILURE);
	}

    PUERTO_ESCUCHA     = config_get_string_value(memoria_config,"PUERTO_ESCUCHA");
    TAM_MEMORIA        = config_get_int_value(memoria_config,"TAM_MEMORIA");
    TAM_PAGINA         = config_get_int_value(memoria_config,"TAM_PAGINA");
    PATH_INSTRUCCIONES = config_get_string_value(memoria_config,"PATH_INSTRUCCIONES");
    RETARDO_RESPUESTA  = config_get_int_value(memoria_config,"RETARDO_RESPUESTA");
    IP_MEMORIA         = config_get_string_value(memoria_config, "IP_MEMORIA");
}

void imprimir_config(){
    log_info(memoria_logger,"PUERTO ESCUCHA: %s",PUERTO_ESCUCHA);
}

//-------------------------------------------------------------------------------------------------------
