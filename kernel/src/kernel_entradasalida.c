#include "../include/kernel_entradasalida.h"

void esperar_conexiones_entradasalida(){ // PERMITE LA ESCUCHA A TRAVÉS DE MÚLTIPLES CANALES
	fd_kernel = iniciar_servidor(PUERTO_ESCUCHA, kernel_logger, "!! Servidor ENTRADA-SALIDA iniciado !!");
	while(1){
        
        int *fd_conexion_entradasalida = malloc(sizeof(int));
        *fd_conexion_entradasalida = esperar_cliente(fd_kernel, kernel_logger, "ENTRADA-SALIDA");

        pthread_t hilo_entradasalida;
        int err = pthread_create(&hilo_entradasalida, NULL,(void*)esperar_entradasalida_kernel, (int*)fd_conexion_entradasalida);
        if (err!=0){
            perror("Fallo de creación de hilo_entradasalida(kernel)\n");
            exit(-3);
        }
        pthread_detach(hilo_entradasalida);
    }
}

void esperar_entradasalida_kernel(int* fd_conexion_entradasalida){
    int estado_while = 1;
    while (estado_while) {
		
        int cod_op = recibir_operacion(*fd_conexion_entradasalida);
		switch (cod_op) {

		case MENSAJE:
		 	recibir_mensaje_tp0(*fd_conexion_entradasalida,kernel_logger);
			break;

		case RESPUESTA_INSTRUCCION_KES: // [NOMBRE INTERFAZ][RESULTADO OPERACION]
		 	
			t_buffer* buffer = NULL;
			buffer = recibir_buffer(*fd_conexion_entradasalida);

			char* nombre_interfaz = extraer_string_del_buffer(buffer);
			int resultado_operacion = extraer_int_del_buffer(buffer);

			log_warning(kernel_logger, "Respuesta de interfaz %s recibida", nombre_interfaz);

			interfaz* una_interfaz = _obtener_interfaz_con_nombre(nombre_interfaz);
			una_interfaz->resultado_operacion_solicitada = resultado_operacion;

			sem_post(&una_interfaz->sem_instruccion_interfaz);
			
			destruir_buffer(buffer);
			
			break;

		case HANDSHAKE_K_ES: // AGREGA A LISTA DE INTERFACES CONECTADAS
			
			buffer = NULL;
			buffer = recibir_buffer(*fd_conexion_entradasalida);

			interfaz* nueva_interfaz = NULL;
			nueva_interfaz = _crear_instancia_interfaz(buffer,fd_conexion_entradasalida);

			log_info(kernel_logger,"Se ha creado la interfaz: %s", nueva_interfaz->nombre_interfaz);
			mostrar_instrucciones(nueva_interfaz);

			list_add_sync(interfaces_conectadas,nueva_interfaz,&mutex_lista_interfaces);

			int tamanio_lista = list_size(interfaces_conectadas);
			log_info(kernel_logger,"Cantidad de interfaces conectadas: %d", tamanio_lista);
			
			ejecutar_en_hilo_detach((void*)control_request_de_interfaz,nueva_interfaz);
			
			destruir_buffer(buffer);

			break;

		case -1:
		
			log_error(kernel_logger, "E/S se desconecto. Terminando servidor");
			
			limpiar_interfaz(fd_conexion_entradasalida);

			estado_while = 0;

            break;
			
		default:
			log_warning(kernel_logger,"Operacion desconocida de E/S");
			break;
		}
	}
}


interfaz* _crear_instancia_interfaz(t_buffer* buffer, int* fd_conexion_entradasalida){ // CONSULTAR: Si está bien creada la instancia 
	
	interfaz* una_interfaz = malloc(sizeof(interfaz));

	char* un_nombre = extraer_string_del_buffer(buffer);
	una_interfaz->nombre_interfaz = malloc(sizeof(char)*strlen(un_nombre) + 1);
	strcpy(una_interfaz->nombre_interfaz,un_nombre);

	una_interfaz->estado_conexion = 1;
	una_interfaz->resultado_operacion_solicitada = OK;
	una_interfaz->lista_procesos_en_cola = list_create();
	una_interfaz->instrucciones_disponibles = list_create();
	
	while(buffer->size > 0){
		instruccion_interfaz* una_instruccion = malloc(sizeof(instruccion_interfaz));
		*una_instruccion = extraer_int_del_buffer(buffer);
		list_add(una_interfaz->instrucciones_disponibles,una_instruccion);
		log_info(kernel_logger,"Agregando instrucción %d", *una_instruccion);
	} 
	
	una_interfaz->fd_conexion=fd_conexion_entradasalida;
	pthread_mutex_init(&una_interfaz->mutex_interfaz, NULL);
	sem_init(&una_interfaz->sem_interfaz,0,1);
	sem_init(&una_interfaz->sem_request_interfaz,0,0);
	sem_init(&una_interfaz->sem_instruccion_interfaz,0,0);

	return una_interfaz;
}

int solicitar_instruccion_a_interfaz(pcb* un_pcb, interfaz* una_interfaz){

	t_paquete* paquete = NULL;

	paquete = crear_paquete_con_buffer(un_pcb->pedido_a_interfaz->instruccion_a_interfaz);
	
	cargar_datos_auxiliares_en_paquete(un_pcb->pedido_a_interfaz->instruccion_a_interfaz,un_pcb,paquete);
	
	enviar_paquete(paquete,*(una_interfaz->fd_conexion));

	destruir_paquete(paquete);
	
	sem_wait(&una_interfaz->sem_instruccion_interfaz);
	
	switch (una_interfaz->resultado_operacion_solicitada)
	{
	case OK:
		sem_post(&una_interfaz->sem_interfaz);  
		break;
	case ERROR:
		break;
	default:
		log_error(kernel_logger,"Una interfaz devolvió un resultado inválido");
		break;
	}

	return una_interfaz->resultado_operacion_solicitada;
	
}

void cargar_datos_auxiliares_en_paquete(instruccion_interfaz instruccion, pcb* un_pcb, t_paquete* un_paquete){
	
	t_list* datos_auxiliares = un_pcb->pedido_a_interfaz->datos_auxiliares_interfaz;
	switch (instruccion)
	{
		case IO_GEN_SLEEP:

			int* un_tiempo = list_remove(datos_auxiliares,0);
			
			cargar_string_a_paquete(un_paquete,un_pcb->pedido_a_interfaz->nombre_interfaz);
			cargar_int_a_paquete(un_paquete,un_pcb->pid);
			cargar_int_a_paquete(un_paquete,*un_tiempo);

			break;
		
		case IO_STDIN_READ:

			cargar_string_a_paquete(un_paquete,un_pcb->pedido_a_interfaz->nombre_interfaz);
			cargar_int_a_paquete(un_paquete,un_pcb->pid);
			// Registro dirección
			uint32_t* parametro = list_remove(datos_auxiliares,0);
			cargar_uint32_a_paquete(un_paquete,*parametro);
			// Registro tamaño
			parametro = list_remove(datos_auxiliares,0);
			cargar_uint32_a_paquete(un_paquete,*parametro);
			break;

		case IO_STDOUT_WRITE:

			cargar_string_a_paquete(un_paquete,un_pcb->pedido_a_interfaz->nombre_interfaz);
			cargar_int_a_paquete(un_paquete,un_pcb->pid);
			log_info(kernel_logger, "pid a es: %d", un_pcb->pid);
			// Registro dirección
			int* un_parametro = list_remove(datos_auxiliares,0);
			cargar_int_a_paquete(un_paquete,*un_parametro);
			log_info(kernel_logger,"MÓDULO: kernel_entradasalida - Registro dirección: %d",*un_parametro);
			// Registro tamaño
			un_parametro = list_remove(datos_auxiliares,0);
			log_info(kernel_logger,"MÓDULO: kernel_entradasalida - Registro tamaño: %d",*un_parametro);
			cargar_int_a_paquete(un_paquete,*un_parametro);
			break;

		case IO_FS_CREATE:

			cargar_int_a_paquete(un_paquete,un_pcb->pid);
			char* nombre_archivo = list_remove(datos_auxiliares,0);
			cargar_string_a_paquete(un_paquete,nombre_archivo);
			break;
      
		case IO_FS_DELETE:

			nombre_archivo = list_remove(datos_auxiliares,0);
			cargar_string_a_paquete(un_paquete, nombre_archivo);
			cargar_int_a_paquete(un_paquete,un_pcb->pid);
			break;

		case IO_FS_TRUNCATE:

			nombre_archivo = list_remove(datos_auxiliares,0);
			cargar_string_a_paquete(un_paquete, nombre_archivo);
			cargar_int_a_paquete(un_paquete,un_pcb->pid);
			//tamanio
			parametro = list_remove(datos_auxiliares,0);
			cargar_int_a_paquete(un_paquete,*parametro);
			break;
			
		case IO_FS_WRITE:

			nombre_archivo = list_remove(datos_auxiliares,0);
			cargar_string_a_paquete(un_paquete, nombre_archivo);
			cargar_int_a_paquete(un_paquete,un_pcb->pid);
			// Registro dirección
			parametro = list_remove(datos_auxiliares,0);
			cargar_int_a_paquete(un_paquete,*parametro);
			// Registro tamaño
			parametro = list_remove(datos_auxiliares,0);
			cargar_int_a_paquete(un_paquete,*parametro);
			// Registro puntero archivo
			parametro = list_remove(datos_auxiliares,0);
			cargar_int_a_paquete(un_paquete,*parametro);
			break;
			
		case IO_FS_READ:

			nombre_archivo = list_remove(datos_auxiliares,0);
			cargar_string_a_paquete(un_paquete, nombre_archivo);
			cargar_int_a_paquete(un_paquete,un_pcb->pid);
			// Registro dirección
			parametro = list_remove(datos_auxiliares,0);
			cargar_int_a_paquete(un_paquete,*parametro);
			// Registro tamaño
			parametro = list_remove(datos_auxiliares,0);
			cargar_int_a_paquete(un_paquete,*parametro);
			// Registro puntero archivo
			parametro = list_remove(datos_auxiliares,0);
			cargar_int_a_paquete(un_paquete,*parametro);

			break;

		default:
			log_error(kernel_logger,"ERROR: Se recibió instrucción de E/S que no está disponible");
			exit(EXIT_FAILURE);
			break;
	}
}

void limpiar_interfaz(int *fd_interfaz){
	
	bool _coincide_fd (interfaz* una_interfaz){
		return *(una_interfaz->fd_conexion) == *fd_interfaz;
	}

	interfaz* una_interfaz = NULL;
	pthread_mutex_lock(&mutex_lista_interfaces);
	una_interfaz = list_find(interfaces_conectadas,(void *)_coincide_fd);
	pthread_mutex_unlock(&mutex_lista_interfaces);

	if(una_interfaz != NULL){
		
		pthread_mutex_lock(&una_interfaz->mutex_interfaz);
		free(una_interfaz->nombre_interfaz);
		una_interfaz->nombre_interfaz = malloc(sizeof(char)*strlen("DESCONECTADA")+1);
		strcpy(una_interfaz->nombre_interfaz,"DESCONECTADA");
		una_interfaz->resultado_operacion_solicitada = ERROR;
		int cantidad_de_procesos_en_cola = list_size(una_interfaz->lista_procesos_en_cola);

		for(int i = 0;i<cantidad_de_procesos_en_cola;i++){
			log_info(kernel_logger,"PROCESOS EN INTERFAZ: %d",list_size(una_interfaz->lista_procesos_en_cola));
			pcb* un_pcb = list_remove(una_interfaz->lista_procesos_en_cola,0);
			un_pcb->pedido_a_interfaz->instruccion_a_interfaz = INSTRUCCION_IO_NO_DEFINIDA;
			free(un_pcb->pedido_a_interfaz->nombre_interfaz);
			un_pcb->pedido_a_interfaz->nombre_interfaz = NULL;
			planificar_proceso_exit_en_hilo(un_pcb);
		}

		una_interfaz->estado_conexion = 0;
		sem_post(&una_interfaz->sem_instruccion_interfaz);
		pthread_mutex_unlock(&una_interfaz->mutex_interfaz);
	}
}


interfaz* _obtener_interfaz_con_nombre(char* nombre_interfaz){
	
	bool _buscar_interfaz(interfaz* una_interfaz){
		
		char* nombre_encontrado = una_interfaz->nombre_interfaz;		
		return strcmp(nombre_encontrado,nombre_interfaz)==0;

	}

	interfaz* una_interfaz = NULL;
	if(list_any_satisfy(interfaces_conectadas,(void*)_buscar_interfaz)){
		return una_interfaz = list_find(interfaces_conectadas,(void*)_buscar_interfaz); // Por qué no me la reconoce?
	}
	else{
		log_error(kernel_logger,"Se solicitó interfaz que ya no se encuentra conectada");
		// En este caso retorna NULL
		return NULL;
	}
}

void mostrar_instrucciones(interfaz* una_interfaz){
	
	instruccion_interfaz* instruccion = NULL;
	int tamanio_lista = list_size(una_interfaz->instrucciones_disponibles);
	log_info(kernel_logger,"Mostrando instrucciones de la siguiente interfaz: %s", una_interfaz->nombre_interfaz);
	for (int i = 0; i < tamanio_lista; i++)
	{	
		instruccion = (instruccion_interfaz*)list_get(una_interfaz->instrucciones_disponibles,i);
		switch (*instruccion)
		{
			
			case IO_GEN_SLEEP:
				log_info(kernel_logger,"IO_GEN_SLEEP");
				break;
			
			case IO_STDIN_READ:
				log_info(kernel_logger,"IO_STDIN_READ");
				break;

			case IO_STDOUT_WRITE:
				log_info(kernel_logger,"IO_STDOUT_WRITE");
				break;

			case IO_FS_CREATE:
				log_info(kernel_logger,"IO_FS_CREATE");
				break;

			case IO_FS_DELETE:
				log_info(kernel_logger,"IO_FS_DELETE");
				break;

			case IO_FS_TRUNCATE:
				log_info(kernel_logger,"IO_FS_TRUNCATE");
				break;	

			case IO_FS_WRITE:
				log_info(kernel_logger,"IO_FS_WRITE");
				break;

			case IO_FS_READ:
				log_info(kernel_logger,"IO_FS_READ");
				break;

			default:
				log_info(kernel_logger,"INSTRUCCIÓN DESCONOCIDA");
				break;
		}
	}
}
