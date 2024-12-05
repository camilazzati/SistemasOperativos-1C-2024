#include "../include/kernel_cpu_interrupt.h"

void esperar_conexiones_cpu_interrupt(){
	
	fd_cpu_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT);
    log_info(kernel_logger, "Conexion con CPU INTERRUPT exitosa.");

	pthread_t hilo_cpu_interrupt;
	int err = pthread_create(&hilo_cpu_interrupt, NULL,(void*)esperar_cpu_interrupt_kernel, NULL);
	if (err!=0){
		perror("Fallo de creación de hilo_cpu_interrupt(kernel)\n");
		exit(-3);
	}
	pthread_detach(hilo_cpu_interrupt);
}

void esperar_cpu_interrupt_kernel(){
    int estado_while = 1;
    while (estado_while) {
		
        int cod_op = recibir_operacion(fd_cpu_interrupt);
		switch (cod_op) {
		case MENSAJE:
		 	recibir_mensaje_tp0(fd_cpu_interrupt,kernel_logger);
			break;
		case PAQUETE:
			break;
		case -1:
			log_error(kernel_logger, "CPU INTERRUPT se desconecto. Terminando servidor");
			estado_while = 0;
            break;
		default:
			log_warning(kernel_logger,"Operacion desconocida de CPU INTERRUPT");
			break;
		}
	}
}


// ENVÍA SEÑAL PARA DESALOJAR PROCESO, LUEGO SACA PCB DE LISTA EXEC Y LO RETORNA
void _gestionar_interrupcion(pcb* un_pcb, interrupcion motivo_interrupcion){
		
		t_paquete* paquete = NULL;
		
		paquete = crear_paquete_con_buffer(motivo_interrupcion);
		cargar_int_a_paquete(paquete,un_pcb->pid);

		enviar_paquete(paquete,fd_cpu_interrupt);
    	destruir_paquete(paquete);

}