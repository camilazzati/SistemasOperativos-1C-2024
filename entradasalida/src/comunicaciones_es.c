# include "../include/comunicaciones_es.h"

void iterator(char* value){
	log_info(es_logger,"%s",value);
}


void esperar_kernel_es(){
    int estado_while = 1;
	sleep(5);
    while (estado_while) {
		log_trace(es_logger,"E/S: ESPERANDO MENSAJES DE KERNEL...");
        int cod_op = recibir_operacion(fd_kernel);
		switch (cod_op) {

		case IO_GEN_SLEEP:

			t_buffer* un_buffer = recibir_buffer(fd_kernel);
			char* nombre_interfaz = extraer_string_del_buffer(un_buffer);
			int pid = extraer_int_del_buffer(un_buffer);
			int un_tiempo = extraer_int_del_buffer(un_buffer);

			log_info(es_logger,"Llegó solicitud para %s de IO_GEN_SLEEP por parte de proceso con PID: %d", nombre_interfaz,pid);
			log_info(es_logger,"Tiempo solicitado: %d", un_tiempo);

			destruir_buffer(un_buffer);

			t_paquete* un_paquete = crear_paquete_con_buffer(RESPUESTA_INSTRUCCION_KES);
			cargar_string_a_paquete(un_paquete,nombre_interfaz);
			cargar_int_a_paquete(un_paquete,OK);

			sleep(un_tiempo);

			enviar_paquete(un_paquete,fd_kernel);
			destruir_paquete(un_paquete);
		
			break;

		case IO_FS_WRITE:
			break;
		case -1:
			log_error(es_logger, "KERNEL se desconecto. Terminando servidor");
			estado_while = 0;
            break;
		default:
			log_warning(es_logger,"Operacion desconocida");
			break;
		}
	}
}



void esperar_memoria_es_stdin(){
    int estado_while = 1;

    while (estado_while) {
		log_trace(es_logger,"E/S: ESPERANDO MENSAJES DE MEMORIA...");
        int cod_op = recibir_operacion(fd_memoria);
        switch (cod_op) {
		
		case ENVIO_RECURSOS_STDIN_ESM:
			
			t_buffer* un_buffer = recibir_buffer(fd_memoria);

			char* resultado = extraer_string_del_buffer(un_buffer);
			if(strcmp(resultado,"OK") == 0){
				respuesta_de_operacion_a_kernel();
			}else{
				respuesta_de_operacion_a_kernel();
			}
			destruir_buffer(un_buffer);
			break;

		case -1:
			log_error(es_logger, "MEMORIA se desconecto. Terminando servidor");
			estado_while = 0;
            break;
		default:
			log_warning(es_logger,"Operacion desconocida");
			break;
		}
	}
}


void esperar_memoria_es_stdout(){
    int estado_while = 1;

    while (estado_while) {
		log_trace(es_logger,"E/S: ESPERANDO MENSAJES DE MEMORIA...");
        int cod_op = recibir_operacion(fd_memoria);
        switch (cod_op) {
		
		case ENVIO_RECURSOS_STDOUT_ESM:
			
			t_buffer* un_buffer = recibir_buffer(fd_memoria);
			char* datos_leidos = extraer_string_del_buffer(un_buffer);

			log_info(es_logger, "recibi datos_leidos");

			if(strcmp(datos_leidos,"ERROR") == 0){
				respuesta_de_operacion_a_kernel();
			}else{
				int tamanio_direcc = strlen(datos_leidos);
				printf("El tamaño es: %d", tamanio_direcc);
				printf("Datos leidos: %s", datos_leidos);

				//mostrar_datos_leidos(datos_leidos, tamanio_direcc);
				respuesta_de_operacion_a_kernel();
			}

			destruir_buffer(un_buffer);
			
			break;

		case -1:
			log_error(es_logger, "MEMORIA se desconecto. Terminando servidor");
			estado_while = 0;
            break;
		default:
			log_warning(es_logger,"Operacion desconocida");
			break;
		}
	}
}

void esperar_memoria_es_dialfs(){
    int estado_while = 1;

    while (estado_while) {
		log_trace(es_logger,"E/S: ESPERANDO MENSAJES DE MEMORIA...");
        int cod_op = recibir_operacion(fd_memoria);
        switch (cod_op) {
		
		case ENVIO_RECURSOS_FWRITE_ESM:
			
			t_buffer* un_buffer = recibir_buffer(fd_memoria);
			datos_leidos_fwrite = extraer_string_del_buffer(un_buffer);
			tamanio_direcc_fwrite = strlen(datos_leidos_fwrite);
			printf("Me llego el valor: <%s> con el tamaño: <%d>", datos_leidos_fwrite, tamanio_direcc_fwrite);
			sem_post(&sem_pedido_lectura);
			//respuesta_de_operacion_fswrite_a_kernel(); 
			destruir_buffer(un_buffer);
			break;

		case ENVIO_RECURSOS_FREAD_ESM:
						
			un_buffer  = recibir_buffer(fd_memoria);

			char* resultado = extraer_string_del_buffer(un_buffer);
			if(strcmp(resultado,"OK") == 0){
				log_trace(es_logger, "Se escribio bien en memoria");
				//respuesta_de_operacion_a_kernel();
			}else{
				//respuesta_de_operacion_a_kernel();
				log_trace(es_logger, "No se pudo escribir correctamente en memoria");
			}
			destruir_buffer(un_buffer);
						
			break;

		case -1:
			log_error(es_logger, "MEMORIA se desconecto. Terminando servidor");
			estado_while = 0;
            break;
		default:
			log_warning(es_logger,"Operacion desconocida");
			break;
		}
	}
}
