#include "../include/encargarse_es.h"




//---------------------------------------------------------------------------------------------------------

void enviar_datos_leidos_es(char* datos_leidos, int fd_conexion_entradasalida, op_code codigo_operacion){
    t_paquete* un_paquete = crear_paquete_con_buffer(codigo_operacion);
    cargar_string_a_paquete(un_paquete, datos_leidos);
    enviar_paquete(un_paquete, fd_conexion_entradasalida);
    eliminar_paquete(un_paquete);
    free(datos_leidos);
       
}

/*void resolver_solicitud_leer_bloque_es(t_buffer* unBuffer, int fd_conexion_entradasalida){

    char* datos_leidos = resolver_solicitud_leer_bloque(unBuffer);
    enviar_datos_leidos_es(datos_leidos,  fd_conexion_entradasalida);

}*/


//---------------------------------------------------------------------------------------------------------

void enviar_respuesta_escritura_en_espacio_usuario_es(char* respuesta, int fd_conexion_entradasalida,  op_code codigo_operacion){
   
    t_paquete* un_paquete = crear_paquete_con_buffer(codigo_operacion);
    cargar_string_a_paquete(un_paquete, respuesta);
    enviar_paquete(un_paquete,  fd_conexion_entradasalida);
    eliminar_paquete(un_paquete);
}

/*
void resolver_solicitud_escribir_bloque_es(t_buffer* unBuffer, int fd_conexion_entradasalida){

    char* respuesta = resolver_solicitud_escribir_bloque(unBuffer);
    enviar_respuesta_escritura_en_espacio_usuario_es(respuesta, fd_conexion_entradasalida);
}*/


//---------------------------------------------------------------------------------------------------------

void esperar_conexiones_es(){
	while(1){
        int *fd_conexion_entradasalida = malloc(sizeof(int));
        *fd_conexion_entradasalida = esperar_cliente(fd_memoria, memoria_logger, "ENTRADA-SALIDA");

        pthread_t hilo_entradasalida;
        int err = pthread_create(&hilo_entradasalida, NULL,(void*)encargarse_es, (int*)fd_conexion_entradasalida);
        if (err!=0){
            perror("Fallo de creación de hilo_entradasalida(kernel)\n");
            exit(-3);
        }
        pthread_detach(hilo_entradasalida);
    }
}

void encargarse_es(int* fd_conexion_entradasalida){

        int numero = 1; 
        log_trace(memoria_logger, "PRUEBAS - Se conectó una interfaz");
        while(numero){

            int codigo_operacion = recibir_operacion(* fd_conexion_entradasalida);

            switch(codigo_operacion){

                case ENVIO_RECURSOS_STDOUT_ESM: 
                t_buffer* unBuffer = recibir_buffer(* fd_conexion_entradasalida);
                //retardo_respuesta();
                log_warning(memoria_logger, "Pedido LEER de IO");
                char* datos_leidos = resolver_solicitud_leer_bloque(unBuffer);
                log_info(memoria_logger, "los datos leidos son: %s", datos_leidos); 
                enviar_datos_leidos_es(datos_leidos, * fd_conexion_entradasalida, ENVIO_RECURSOS_STDOUT_ESM);
                log_info(memoria_logger," datos enviados");
                destruir_buffer(unBuffer);
                break;

                case ENVIO_RECURSOS_STDIN_ESM:
                unBuffer = recibir_buffer(* fd_conexion_entradasalida);
                //retardo_respuesta();
                log_warning(memoria_logger, "Pedido ESCRIBIR de IO");
                char* respuesta = resolver_solicitud_escribir_bloque(unBuffer);
                enviar_respuesta_escritura_en_espacio_usuario_es(respuesta, * fd_conexion_entradasalida, ENVIO_RECURSOS_STDIN_ESM);
                log_info(memoria_logger," datos enviados");
                destruir_buffer(unBuffer);
                break; 

                case ENVIO_RECURSOS_FWRITE_ESM:
                unBuffer = recibir_buffer(* fd_conexion_entradasalida);
                //retardo_respuesta();
                datos_leidos = resolver_solicitud_leer_bloque(unBuffer);
                enviar_datos_leidos_es(datos_leidos, * fd_conexion_entradasalida, ENVIO_RECURSOS_FWRITE_ESM);
                log_info(memoria_logger," datos enviados");
                destruir_buffer(unBuffer);
                break;

                case ENVIO_RECURSOS_FREAD_ESM:
                unBuffer = recibir_buffer(* fd_conexion_entradasalida);
                //retardo_respuesta();
                respuesta = resolver_solicitud_escribir_bloque(unBuffer);
                enviar_respuesta_escritura_en_espacio_usuario_es(respuesta, * fd_conexion_entradasalida,  ENVIO_RECURSOS_FREAD_ESM);
                log_info(memoria_logger," datos enviados");
                destruir_buffer(unBuffer);
                break; 

            }

           
        }


    }
