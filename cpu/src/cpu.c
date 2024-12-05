#include "../include/cpu.h"

// SERVIDOR DE: KERNEL (x2)
// CLIENTE DE:  MEMORIA

//INICIAR_PROCESO /home/utnso/Documents/scripts/c-comenta-pruebas/scripts_memoria/IO_A

int main(void){
    
    // INICIAR CPU
    inicializar_cpu();

    primer_pcb = true;

    // CONEXIONES CPU
  
    //conectarse como cliente con memoria
    fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(cpu_logger, "Conexion con MEMORIA exitosa");
  
    // iniciar server cpu-dispatch
    fd_cpu_dispatch = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH, cpu_logger, "!! Servidor CPU-DISPATCH iniciado !!");
    //iniciar server cpu-interrupt
    fd_cpu_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT, cpu_logger, "!! Servidor CPU-DISPATCH iniciado !!");

    //esperar al cliente kernel-dispatch
    fd_kernel_dispatch = esperar_cliente(fd_cpu_dispatch, cpu_logger, "KERNEL - Dispatch");
    //esperar al cliente kernel-interrupt
    fd_kernel_interrupt = esperar_cliente(fd_cpu_interrupt, cpu_logger, "KERNEL - Interrupt");


    // COMUNICACIÓN


    //atender los mensajes de kernel-dispatch
    pthread_t hilo_k_dispatch;
    int err = pthread_create(&hilo_k_dispatch,NULL,(void*)esperar_kernel_cpu_dispatch,NULL);
    if (err!=0){
        perror("Fallo de creación de hilo_k_dispatch(cpu)\n");
        return -3;
    }
    pthread_detach(hilo_k_dispatch);

    // atender los mensajes de kernel-interrupt
    pthread_t hilo_k_interrupt;
    err = pthread_create(&hilo_k_interrupt,NULL,(void*)esperar_kernel_cpu_interrupt,NULL);
    if (err!=0){
        perror("Fallo de creación de hilo_k_interrupt(cpu)\n");
        return -3;
    }
    pthread_detach(hilo_k_interrupt);

    pthread_t hilo_memoria;
    err = pthread_create(&hilo_memoria,NULL,(void*)esperar_memoria_cpu,NULL);
    if (err!=0){
        perror("Fallo de creación de hilo_memoria(cpu)\n");
        return -3;
    }
    pthread_join(hilo_memoria,NULL);

    
	return EXIT_SUCCESS;
}

