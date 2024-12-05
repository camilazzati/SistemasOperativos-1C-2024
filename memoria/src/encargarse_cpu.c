#include "../include/encargarse_cpu.h"



//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//FUNCIONES NECESARIAS

// Implementación de funciones de manejo de operaciones

/*
void solicitud_info_memoria(t_buffer *unBuffer) {
   unBuffer = recibir_buffer(fd_cpu);
}
*/
//------------------------------------------------------------------------------------------------------------


void enviar_tamanio_pagina(int cliente_socket){
    t_paquete* un_paquete = crear_paquete_con_buffer(SOLICITUD_INFO_MEMORIA);
    cargar_int_a_paquete(un_paquete, TAM_PAGINA);
    enviar_paquete(un_paquete, fd_cpu);
    eliminar_paquete(un_paquete);
}


//---------------------------------------------------------------------------------------------------------

void mandar_instruccion_a_cpu(char* instruccion){
    
    t_paquete* paquete = crear_paquete_con_buffer(SOLICITUD_INSTRUCCION);
    cargar_string_a_paquete(paquete, instruccion);
    //free(instruccion);
    enviar_paquete(paquete, fd_cpu);
    eliminar_paquete(paquete);    
}

void resolver_solicitud_instruccion(t_buffer *unBuffer) {

    retardo_respuesta();
    int pid = extraer_int_del_buffer(unBuffer);
    int ip  = extraer_int_del_buffer(unBuffer);

    t_proceso* proceso = obtener_proceso_por_pid(pid); 
    if (proceso == NULL) {
        log_error(memoria_logger, "No se encontró el proceso con PID: %d", pid);
        return;
    }

    char* instruccion = extraer_instruccion_por_ip(proceso, ip);
    if (instruccion == NULL) {
        log_error(memoria_logger, "No se encontró la instrucción en el IP: %d para el PID: %d", ip, pid);
        return;   
    }
    
    log_info(memoria_logger, "Proceso [PID: %d, IP: %d]: %s", pid, ip, instruccion);
    mandar_instruccion_a_cpu(instruccion);
}


//---------------------------------------------------------------------------------------------------------

void mandar_num_marco(int numero_marco){
    
    t_paquete* un_paquete = crear_paquete_con_buffer(SOLICITUD_PAGINA);
    cargar_int_a_paquete(un_paquete, numero_marco);
    enviar_paquete(un_paquete, fd_cpu);
    eliminar_paquete(un_paquete);
}


void resolver_solicitud_consulta_pagina(t_buffer *unBuffer) {
    retardo_respuesta();
    int pid = extraer_int_del_buffer(unBuffer);
    int num_pagina  = extraer_int_del_buffer(unBuffer);
  
    t_proceso* proceso = obtener_proceso_por_pid(pid);
    int num_marco_buscado = devolver_numero_de_marco(proceso,num_pagina);
    log_trace(memoria_logger, "----------------------------------------/n");
    log_trace(memoria_logger, "PRUEBAS: encontre el numero de marco pedido: %d", num_marco_buscado);
    log_trace(memoria_logger, "----------------------------------------/n");

    t_marco* marco = buscar_marco_segun_numero(num_marco_buscado);
    log_trace(memoria_logger, "----------------------------------------/n");
    log_trace(memoria_logger, "PRUEBAS: numero de base de marco es: %d", marco->base);
    log_trace(memoria_logger, "----------------------------------------/n");

    mandar_num_marco(num_marco_buscado);
}

//---------------------------------------------------------------------------------------------------------

void mandar_resultado_resize(int resultado){
   
    t_paquete* paquete = crear_paquete_con_buffer(RTA_AJUSTAR_TAMANIO);
    cargar_int_a_paquete(paquete, resultado);
    enviar_paquete(paquete, fd_cpu);
    eliminar_paquete(paquete);    
}

void resolver_ajustar_tamanio(t_buffer* unBuffer){
    
    retardo_respuesta();
    int pid = extraer_int_del_buffer(unBuffer);
    int tamanio_nuevo = extraer_int_del_buffer(unBuffer);
    int resultado;

    t_proceso* proceso = obtener_proceso_por_pid(pid);

    if(tamanio_nuevo > proceso->size){
        resultado = ampliar_tamanio_proceso(tamanio_nuevo,proceso);
    }
    else{
        resultado = reducir_tamanio_proceso(tamanio_nuevo,proceso);
    }
    
    mandar_resultado_resize(resultado);
}

//---------------------------------------------------------------------------------------------------------

void enviar_datos_leidos_cpu(char* datos_leidos){
    
    t_paquete* un_paquete = crear_paquete_con_buffer(SOLICITUD_LECTURA_MEMORIA_BLOQUE);
    cargar_string_a_paquete(un_paquete, datos_leidos);
    enviar_paquete(un_paquete, fd_cpu);
    eliminar_paquete(un_paquete);
    free(datos_leidos);
       
}

void resolver_solicitud_leer_bloque_cpu(t_buffer* unBuffer){
    //retardo_respuesta();
    log_warning(memoria_logger, "Pedido LEER de CPU");
    char* datos_leidos = resolver_solicitud_leer_bloque(unBuffer);
    enviar_datos_leidos_cpu(datos_leidos);
}

//---------------------------------------------------------------------------------------------------------

void enviar_respuesta_escritura_en_espacio_usuario_cpu(char* respuesta){
    
    t_paquete* un_paquete = crear_paquete_con_buffer(SOLICITUD_ESCRITURA_MEMORIA_BLOQUE);
    cargar_string_a_paquete(un_paquete, respuesta);
    enviar_paquete(un_paquete, fd_cpu);
    eliminar_paquete(un_paquete);
}

void resolver_solicitud_escribir_bloque_cpu(t_buffer* unBuffer){
    //retardo_respuesta();
    log_warning(memoria_logger, "Pedido ESCRIBIR de CPU");
    char* respuesta = resolver_solicitud_escribir_bloque(unBuffer);
    enviar_respuesta_escritura_en_espacio_usuario_cpu(respuesta);
   // free(respuesta);
}

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//FUNCIONES PRINCIPAL 


void encargarse_cpu(){
        
        enviar_tamanio_pagina(fd_cpu);
        int numero =1;

        while(numero){
		    int codigo_operacion = recibir_operacion(fd_cpu);

            switch(codigo_operacion){
                case SOLICITUD_INSTRUCCION:
                    t_buffer* unBuffer1 = recibir_buffer(fd_cpu);
					resolver_solicitud_instruccion(unBuffer1);
                    destruir_buffer(unBuffer1);
                    break;

                case SOLICITUD_PAGINA:
                    t_buffer* unBuffer2 = recibir_buffer(fd_cpu);
					resolver_solicitud_consulta_pagina(unBuffer2);
                    destruir_buffer(unBuffer2);
                    break;

                case SOLICITUD_ESCRITURA_MEMORIA_BLOQUE:
                    t_buffer* unBuffer3 = recibir_buffer(fd_cpu);
                    resolver_solicitud_escribir_bloque_cpu(unBuffer3);
                    destruir_buffer(unBuffer3);
                    break;

                case SOLICITUD_LECTURA_MEMORIA_BLOQUE:
                    t_buffer* unBuffer4 = recibir_buffer(fd_cpu);
                    resolver_solicitud_leer_bloque_cpu(unBuffer4);
                    destruir_buffer(unBuffer4);
                    break;

                case AJUSTAR_TAMANIO:
                    t_buffer* unBuffer = recibir_buffer(fd_cpu);
                    resolver_ajustar_tamanio(unBuffer);
                    destruir_buffer(unBuffer);
                    break; 


                case -1:
				log_error(memoria_logger, "SE DESCONECTO CPU");
				numero= 0;
                return;

                default:
				log_error(memoria_logger, "NO ES UNA OPERACION");
				break;
			}
		
        }
}
