#include "../include/entradasalida.h"

// SERVIDOR DE: -
// CLIENTE DE: KERNEL, MEMORIA 


int main(int argc, char ** argv) {

    //char* nombre_interfaz_global;
    char* nombre_interfaz;
    char* archivo_configuracion;


    char* leido = readline("> ");
    printf("Leido: %s", leido);
    if(strcmp(leido, "fs") == 0){
        nombre_interfaz = "FS";
        nombre_interfaz_global = "FS";
        archivo_configuracion = "/home/utnso/so-deploy/tp-2024-1c-ubuntunel/entradasalida/dialfs.config"; 
        //archivo_configuracion = "/home/utnso/Documents/tp-2024-1c-ubuntunel/entradasalida/dialfs.config";
    } else {
        nombre_interfaz = argv[1];
        nombre_interfaz_global = argv[1];
        archivo_configuracion = argv[2];
    }

    /*char* nombre_interfaz = argv[1];
    nombre_interfaz_global = argv[1];
    char* archivo_configuracion = argv[2]; */
    /*char* nombre_interfaz = "FS";
    nombre_interfaz_global = "FS";
    char* archivo_configuracion = "/home/utnso/Documents/tp-2024-1c-ubuntunel/entradasalida/dialfs.config";
*/  
    printf("Nombre de la interfaz: %s\n", nombre_interfaz);
    printf("Ruta del archivo de configuración: %s\n", archivo_configuracion);

    inicializar_es(archivo_configuracion);
 
     if(strcmp(TIPO_INTERFAZ,"GENERICA")== 0){
        // CONEXION CON KERNEL
        
        log_trace(es_logger,"CREANDO CONEXION CON KERNEL... GENERICA");
        fd_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL);
        log_trace(es_logger,"CONEXION CON KERNEL EXITOSA");
        enviar_handshake_generica(nombre_interfaz);

        pthread_t hilo_kernel;
        int err = pthread_create(&hilo_kernel,NULL,(void*)atender_generica_es_kernel,NULL);
        if (err!=0){
            perror("Fallo de creación de hilo_kernel(entradasalida)\n");
            return -3;
        }
        pthread_join(hilo_kernel,NULL);

        

    }else if(strcmp(TIPO_INTERFAZ,"STDIN")== 0){
        // CONEXION CON KERNEL
        log_trace(es_logger,"CREANDO CONEXION CON KERNEL... STIDN");
        fd_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL);
        log_trace(es_logger,"CONEXION CON KERNEL EXITOSA");
        enviar_handshake_stdin(nombre_interfaz);

        //CONEXION CON MEMORIA
        log_trace(es_logger,"CREANDO CONEXION CON MEMORIA...STIDN");
        fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
        log_trace(es_logger,"CONEXION CON MEMORIA EXITOSA!");

        pthread_t hilo_memoria;
        int err = pthread_create(&hilo_memoria,NULL,(void*)esperar_memoria_es_stdin,NULL);
        if (err!=0){
            perror("Fallo de creación de hilo_memoria(entradasalida))\n");
            return -3;
        }
        pthread_detach(hilo_memoria);
    
        pthread_t hilo_kernel;
        err = pthread_create(&hilo_kernel,NULL,(void*)atender_stdin_es_kernel,NULL);
        if (err!=0){
            perror("Fallo de creación de hilo_kernel(entradasalida)\n");
            return -3;
        }
        pthread_join(hilo_kernel,NULL); 


        
    }else if(strcmp(TIPO_INTERFAZ,"STDOUT")== 0){
        // CONEXION CON KERNEL
        log_trace(es_logger,"CREANDO CONEXION CON KERNEL... STDOUT");
        fd_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL);
        log_trace(es_logger,"CONEXION CON KERNEL EXITOSA");
        enviar_handshake_stdout(nombre_interfaz);

        //CONEXION CON MEMORIA
        log_trace(es_logger,"CREANDO CONEXION CON MEMORIA...STDOUT");
        fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
        log_trace(es_logger,"CONEXION CON MEMORIA EXITOSA!");

        pthread_t hilo_memoria;
        int err = pthread_create(&hilo_memoria,NULL,(void*)esperar_memoria_es_stdout,NULL);
        if (err!=0){
            perror("Fallo de creación de hilo_memoria(entradasalida))\n");
            return -3;
        }
        pthread_detach(hilo_memoria);
        
        log_info(es_logger,"Soy yo");

        pthread_t hilo_kernel;
        err = pthread_create(&hilo_kernel,NULL,(void*)atender_stdout_es_kernel,NULL);
        if (err!=0){
            perror("Fallo de creación de hilo_kernel(entradasalida)\n");
            return -3;
        }
        pthread_join(hilo_kernel,NULL); 


    }else if(strcmp(TIPO_INTERFAZ,"DIALFS")== 0){
        // CONEXION CON KERNEL
        log_trace(es_logger,"CREANDO CONEXION CON KERNEL... DIALFS");
        fd_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL);
        log_trace(es_logger,"CONEXION CON KERNEL EXITOSA");

        enviar_handshake_dialfs(nombre_interfaz);

        //CONEXION CON MEMORIA
        log_trace(es_logger,"CREANDO CONEXION CON MEMORIA...DIALFS");
        fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
        log_trace(es_logger,"CONEXION CON MEMORIA EXITOSA!");

        iniciar_file_system();
        
        pthread_t hilo_memoria;
        int err = pthread_create(&hilo_memoria,NULL,(void*)esperar_memoria_es_dialfs,NULL);
        if (err!=0){
            perror("Fallo de creación de hilo_memoria(entradasalida))\n");
            return -3;
        }
        pthread_detach(hilo_memoria);
    
        pthread_t hilo_kernel;
        err = pthread_create(&hilo_kernel,NULL,(void*)atender_dialfs_es_kernel,NULL);
        if (err!=0){
            perror("Fallo de creación de hilo_kernel(entradasalida)\n");
            return -3;
        }
        pthread_join(hilo_kernel,NULL); 
        
        
    }
    
}	


void enviar_handshake_generica(char* nombre_interfaz) {
    t_paquete* un_paquete = crear_paquete_con_buffer(HANDSHAKE_K_ES);
    cargar_string_a_paquete(un_paquete, nombre_interfaz);
    cargar_int_a_paquete(un_paquete, IO_GEN_SLEEP);

    enviar_paquete(un_paquete, fd_kernel);

    log_info(es_logger, "Handshake enviado GEN");

    eliminar_paquete(un_paquete);
}

void enviar_handshake_stdin(char* nombre_interfaz) {
    t_paquete* un_paquete = crear_paquete_con_buffer(HANDSHAKE_K_ES);
    cargar_string_a_paquete(un_paquete, nombre_interfaz);
    cargar_int_a_paquete(un_paquete, IO_STDIN_READ);

    enviar_paquete(un_paquete, fd_kernel);

    log_info(es_logger, "Handshake enviado STDIN");

    eliminar_paquete(un_paquete);
}

void enviar_handshake_stdout(char* nombre_interfaz) {
    t_paquete* un_paquete = crear_paquete_con_buffer(HANDSHAKE_K_ES);
    cargar_string_a_paquete(un_paquete, nombre_interfaz);
    cargar_int_a_paquete(un_paquete, IO_STDOUT_WRITE);

    enviar_paquete(un_paquete, fd_kernel);

    log_info(es_logger, "Handshake enviado STDOUT");

    eliminar_paquete(un_paquete);
}

void enviar_handshake_dialfs(char* nombre_interfaz) {
    t_paquete* un_paquete = crear_paquete_con_buffer(HANDSHAKE_K_ES);
    cargar_string_a_paquete(un_paquete, nombre_interfaz);
    cargar_int_a_paquete(un_paquete, IO_FS_CREATE);
    cargar_int_a_paquete(un_paquete, IO_FS_READ);
    cargar_int_a_paquete(un_paquete, IO_FS_WRITE);
    cargar_int_a_paquete(un_paquete, IO_FS_TRUNCATE);
    cargar_int_a_paquete(un_paquete, IO_FS_DELETE);

    enviar_paquete(un_paquete, fd_kernel);
    
    log_info(es_logger, "Handshake enviado DIALFS");

    eliminar_paquete(un_paquete);
}
