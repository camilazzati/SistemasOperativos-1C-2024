#include "../include/comunicaciones_cpu.h"

void iterator(char *value)
{
	log_info(cpu_logger, "%s", value);
}

void esperar_kernel_cpu_interrupt()
{
	int estado_while = 1;
	while (estado_while)
	{
		log_trace(cpu_logger, "CPU INTERRUPT: ESPERANDO MENSAJES DE KERNEL...");
		int cod_op = recibir_operacion(fd_kernel_interrupt);
		switch (cod_op)
		{
		case INTERRUPCION_QUANTUM:
			log_trace(cpu_logger, "Me llego una interrupción por Quantum");		
			t_buffer *unBuffer = recibir_buffer(fd_kernel_interrupt);
			atender_interrupcion_quantum(unBuffer);
			destruir_buffer(unBuffer);
			break;
		case EXIT_PROCESS:
			log_trace(cpu_logger, "Me llego una interrupción Exit");
			unBuffer = recibir_buffer(fd_kernel_interrupt);
			atender_interrupcion_exit(unBuffer);
			destruir_buffer(unBuffer);
			break;
		case -1:
			log_error(cpu_logger, "KERNEL se desconecto de cpu interrupt. Terminando servidor");
			estado_while = 0;
			break;
		default:
			log_warning(cpu_logger, "Operacion desconocida de KERNEL (Interrupt)");
			break;
		}
	}
}

void esperar_kernel_cpu_dispatch()
{
	int estado_while = 1;
	while (estado_while)
	{
		log_trace(cpu_logger, "CPU DISPATCH: ESPERANDO MENSAJES DE KERNEL...");
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		switch (cod_op){
		case EJECUTAR_PROCESO_KCPU:
			log_trace(cpu_logger, "Me llego un proceso");
			t_buffer *unBuffer = recibir_buffer(fd_kernel_dispatch);
			recibir_pcb_del_kernel(unBuffer);
			destruir_buffer(unBuffer);
			break;
		case -1:
			log_error(cpu_logger, "KERNEL se desconecto de cpu dispatch. Terminando servidor");
			estado_while = 0;
			break;
		default:
			log_warning(cpu_logger, "Operacion desconocida de KERNEL (Distaptch)");
			break;
		}
	}
}

void esperar_memoria_cpu()
{
	int estado_while = 1;
	while (estado_while)
	{
		log_trace(cpu_logger, "CPU: ESPERANDO MENSAJES DE MEMORIA");
		int cod_op = recibir_operacion(fd_memoria);
		switch (cod_op)
		{
		case SOLICITUD_INSTRUCCION:
			log_trace(cpu_logger, "Me llego una instruccion");
			t_buffer *buffer = recibir_buffer(fd_memoria);
			recibir_instruccion(buffer);
			destruir_buffer(buffer);
			sem_post(&sem_pedido_instruccion);
			break;
		case SOLICITUD_PAGINA:
			log_trace(cpu_logger, "Me llego un marco");
			buffer = recibir_buffer(fd_memoria);
			marco = extraer_int_del_buffer(buffer);
			destruir_buffer(buffer);
			sem_post(&sem_pedido_marco);
			break;
		case SOLICITUD_INFO_MEMORIA:
			log_trace(cpu_logger, "Me llego el tamaño de página");
			buffer = recibir_buffer(fd_memoria);
			tamanio_pagina = extraer_int_del_buffer(buffer);
			destruir_buffer(buffer);
			sem_post(&sem_pedido_tamanio_pag);
			break;
		case RTA_AJUSTAR_TAMANIO:
			log_trace(cpu_logger, "Me llego la rta a resize");
			buffer = recibir_buffer(fd_memoria);
			resultado = extraer_int_del_buffer(buffer);
			destruir_buffer(buffer);
			sem_post(&sem_rta_resize);
			break;
		case SOLICITUD_LECTURA_MEMORIA_BLOQUE:
			log_trace(cpu_logger, "Me llego lectura");
			buffer = recibir_buffer(fd_memoria);
			char *literal;
			literal = extraer_string_del_buffer(buffer);
			atender_lectura(literal);
			destruir_buffer(buffer);
			sem_post(&sem_solicitud_lectura);
			break;
		case SOLICITUD_ESCRITURA_MEMORIA_BLOQUE:
			log_trace(cpu_logger, "Me llego escritura");
			buffer = recibir_buffer(fd_memoria);
			char *literal2;
			literal2 = extraer_string_del_buffer(buffer);
			atender_escritura(literal2);
			destruir_buffer(buffer);
			sem_post(&sem_solicitud_escritura);
			break;
		case -1:
			log_error(cpu_logger, "MEMORIA se desconecto. Terminando servidor");
			estado_while = 0;
			break;
		default:
			log_warning(cpu_logger, "Operacion desconocida de MEMORIA");
			break;
		}
	}
}

// atender memoria

void atender_lectura(char* literal){
	//if (respuesta_marco_lectura != NULL){
	//	free(respuesta_marco_lectura);
	//}
	respuesta_marco_lectura = (char *)malloc((strlen(literal) + 1) * sizeof(char));
	if(respuesta_marco_lectura == NULL){
		log_error(cpu_logger, "Error de asignacion de memoria\n");
		exit(EXIT_FAILURE);
	}
	strcpy(respuesta_marco_lectura, literal);
	//free(literal);
}

void atender_escritura(char* literal2){
	if (respuesta_marco_escritura != NULL){
		free(respuesta_marco_escritura);
	}
	respuesta_marco_escritura = (char *)malloc((strlen(literal2) + 1) * sizeof(char));
	if(respuesta_marco_escritura == NULL){
		log_error(cpu_logger, "Error de asignacion de memoria\n");
		exit(EXIT_FAILURE);
	}
	strcpy(respuesta_marco_escritura, literal2);
	free(literal2);
}

void recibir_instruccion(t_buffer *unBuffer)
{
	char *delimitador = " ";
	char *instruccion = extraer_string_del_buffer(unBuffer);
	instruccion_dividida = string_split(instruccion, delimitador);
	free(instruccion);
}

// KERNEL

void mostrar_pcb()
{
	log_trace(cpu_logger, "[PID: %d] [PC: %d] [TIEMPO EJECUTADO: %u] [REGISTROS: %u|%u|%u|%u|%u|%u|%u|%u|%u|%u]",
				contexto->proceso_pid,
				contexto->proceso_pc,
				contexto->proceso_tiempo_ejecutado,
				contexto->AX,
				contexto->BX,
				contexto->CX,
				contexto->DX,
				contexto->EAX,
				contexto->EBX,
				contexto->ECX,
				contexto->EDX,
				contexto->SI,
				contexto->DI);
}

void recibir_pcb_del_kernel(t_buffer *unBuffer)
{
	pthread_mutex_lock(&mutex_manejo_contexto);
	if(primer_pcb){
		contexto = malloc(sizeof(t_contexto)); //iniciar_estructuras_para_recibir_pcb_por_primera_vez(unBuffer);
		primer_pcb = false;
	} 
	iniciar_estructuras_para_recibir_pcb(unBuffer);
	pthread_mutex_unlock(&mutex_manejo_contexto);

	mostrar_pcb();

	if(tamanio_pagina == 0){
		sem_wait(&sem_pedido_tamanio_pag);
	}

	iniciar_tiempo();

	realizarCicloInstruccion();
}

void iniciar_estructuras_para_recibir_pcb(t_buffer *unBuffer)
{
	contexto->proceso_pid = extraer_int_del_buffer(unBuffer);
	contexto->proceso_pc = extraer_int_del_buffer(unBuffer);
	contexto->proceso_tiempo_ejecutado = extraer_int_del_buffer(unBuffer);

	contexto->AX = extraer_uint32_del_buffer(unBuffer);
	contexto->BX = extraer_uint32_del_buffer(unBuffer);
	contexto->CX = extraer_uint32_del_buffer(unBuffer);
	contexto->DX = extraer_uint32_del_buffer(unBuffer);
	contexto->EAX = extraer_uint32_del_buffer(unBuffer);
	contexto->EBX = extraer_uint32_del_buffer(unBuffer);
	contexto->ECX = extraer_uint32_del_buffer(unBuffer);
	contexto->EDX = extraer_uint32_del_buffer(unBuffer);
	contexto->SI = extraer_uint32_del_buffer(unBuffer);
	contexto->DI = extraer_uint32_del_buffer(unBuffer);
}

void atender_interrupcion_quantum(t_buffer *unBuffer)
{
	// verifico que el pid del proceso interrumpido sea el mismo que el del proceso actual, si lo es lo interrumpo y sino no hago nada
	int pid_interrumpido = extraer_int_del_buffer(unBuffer);
	//int motivo_interrupcion = extraer_int_del_buffer(unBuffer);

	if (pid_interrumpido == contexto->proceso_pid){
		
		hay_interrupcion_quantum = true;
	}
}

void atender_interrupcion_exit(t_buffer *unBuffer)
{
	// verifico que el pid del proceso interrumpido sea el mismo que el del proceso actual, si lo es lo interrumpo y sino no hago nada
	int pid_interrumpido = extraer_int_del_buffer(unBuffer);
	//int motivo_interrupcion = extraer_int_del_buffer(unBuffer);

	if (pid_interrumpido == contexto->proceso_pid){
		
		hay_interrupcion_exit = true;
	}
}
