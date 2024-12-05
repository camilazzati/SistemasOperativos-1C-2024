#include "../include/memoria.h"

void inicializar_mutex();
int servidor_escucha();

// SERVIDOR DE: CPU, KERNEL, ENTRADASALIDA
// CLIENTE DE: - 

// MENSAJES DE PRUEBA
void mandar_mensajes(){
    sleep(10);
    enviar_mensaje("Hola KERNEL, soy memoria", fd_kernel);
    enviar_mensaje("Hola CPU, soy memoria", fd_cpu);
    enviar_mensaje("Hola E/S, soy memoria", fd_es);   
}

int main(int argc, char* argv[]) {
    
    // 1) Inicializar estructuras de memoria
   //  2) Iniciar servidor de memoria
  //   3) Concectar con clientes
	
    inicializar_memoria();
	
    fd_memoria = iniciar_servidor(PUERTO_ESCUCHA,memoria_logger,"Iniciado servidor: Memoria");  
    fd_cpu     = esperar_cliente(fd_memoria, memoria_logger,"Cliente: CPU");
    fd_kernel  = esperar_cliente(fd_memoria, memoria_logger,"Cliente: Kernel");
//    fd_es      = esperar_cliente(fd_memoria, memoria_logger,"Cliente: E/S");

//-------------------------------------------------------------------------------------------------------
// Procesos
       // Creacion Espacio Usuario 
    
    lista_procesos = list_create();
    iniciar_espacio_usuario(); 
    inicializar_mutex();

//-------------------------------------------------------------------------------------------------------
// Hilos 

    pthread_t hilo_cpu;
    int err = pthread_create(&hilo_cpu,NULL,(void*)encargarse_cpu,NULL);
    if (err!=0){
        perror("Fallo de creación de hilo_cpu(memoria))\n");
        return -3;
    }
    pthread_detach(hilo_cpu);
/*
    pthread_t hilo_es;
    err = pthread_create(&hilo_es,NULL,(void*)encargarse_es,NULL);
    if (err!=0){
        perror("Fallo de creación de hilo_es(memoria))\n");
        return -3;
    }
    pthread_detach(hilo_es);
*/
    ejecutar_en_hilo_detach((void*)esperar_conexiones_es,NULL);

    pthread_t hilo_kernel;
    err = pthread_create(&hilo_kernel,NULL,(void*)esperar_kernel_memoria,NULL);
    if (err!=0){
        perror("Fallo de creación de hilo_kernel(memoria))\n");
        return -3;
    }
    pthread_join(hilo_kernel,NULL);


   

//-------------------------------------------------------------------------------------------------------

    

    //servidor_fd_memoria = iniciar_servidor(PUERTO_ESCUCHA,memoria_logger, IP_MEMORIA);
   //while(servidor_escucha())

    finalizar_memoria();

    return 0;
}


///// COMUNICACIONES CON LOS MODULOS -> ATENDER LOS PEDIDOS 

    
    void cliente_segun_modulo(int conexion, t_buffer* unBuffer){
        int identificador_modulo = extraer_int_del_buffer(unBuffer);

        switch (identificador_modulo) {
		case CPU:
			fd_cpu = conexion;
			log_info(memoria_logger, "CPU se conecto correctamente");
			encargarse_cpu(fd_cpu);
			break; 

        case KERNEL:
            fd_kernel = conexion;
            log_info(memoria_logger, "Kernel se conecto correctamente");
            encargarse_kernel(fd_kernel);    
            break;

        case ENTRADASALIDA:
        fd_es = conexion;
        log_info(memoria_logger, "E/S se conecto correctamente");
        encargarse_es(fd_es);
        break;

        }
    }

static void gestionar_conexion(void *void_args){
    int *args = (int*) void_args;
    int cliente_socket = *args;
    int codigo_operacion = recibir_operacion(cliente_socket);
    t_buffer* buffer;

    switch (codigo_operacion)
    {
    case IDENTIFICACION:
        buffer = recibir_buffer(cliente_socket);
        cliente_segun_modulo(cliente_socket, buffer);
        break;
    
    case -1:
        log_error(memoria_logger, "Error: Se desconceto cliente en  IDENTIFICACION");
        break;

        default:
        log_error(memoria_logger, "Error: Operacion desconocida en IDENTIFICACION");
        break;
    }
}

void saludar_cliente(void *void_args){
    int* conexion = (int*) void_args;
    int codigo_operacion = recibir_operacion(*conexion);
    switch(codigo_operacion){

        case HANDSHAKE:
        int resultado = 1;
        void* dato_para_enviar = malloc(sizeof(int));
        memcpy(dato_para_enviar, &resultado, sizeof(int));
        send(*conexion, dato_para_enviar, sizeof(int), 0);
        free(dato_para_enviar);
        gestionar_conexion(conexion);
        break;

        case -1:
        log_error(memoria_logger, "Error: Se desconceto cliente en HANDSHAKE");
        break;

        default:
        log_error(memoria_logger, "Error: Operacion desconocida en HANDSHAKE");
        break;
    }
}


int servidor_escucha(){

    servidor_memoria_nombre = "Memoria";
    log_info(memoria_logger, "%s servidor comenzando", servidor_memoria_nombre);
    int numero =1;
    while(numero){
        int cliente_socket = esperar_cliente(servidor_fd_memoria,memoria_logger, servidor_memoria_nombre);
        if(cliente_socket != -1){
            pthread_t hilo_cliente;
            int *args = malloc(sizeof(int));
            *args = cliente_socket;
            pthread_create(&hilo_cliente, NULL, (void*)saludar_cliente, args);
            log_info(memoria_logger, "[THREAD] Estableciendo hilo para soporte");
            pthread_detach(hilo_cliente);
        }
        
    }
    return EXIT_SUCCESS;
}



//-------------------------------------------------------------------------

void inicializar_mutex(){
    pthread_mutex_init(&mutex_lista_procesos, NULL);
    pthread_mutex_init(&mutex_lista_marcos, NULL);
    pthread_mutex_init(&mutex_tablas, NULL);
    pthread_mutex_init(&mutex_espacio_usuario, NULL);
}
