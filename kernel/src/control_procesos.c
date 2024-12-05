#include "../include/control_procesos.h"
// FUNCIONALIDADES PCB
pcb *crear_pcb(char *path)
{

	pcb *nuevo_PCB = malloc(sizeof(pcb));

	nuevo_PCB->pid = asignar_pid();

	nuevo_PCB->program_counter = 0;
	nuevo_PCB->quantum = QUANTUM; 
	nuevo_PCB->tiempo_ejecutado = 0;

	nuevo_PCB->ticket = -1; // Inicializa en -1 porque el valor del primer ticket global es 0

	nuevo_PCB->path = malloc(sizeof(char)*strlen(path) + 1); 

	if (nuevo_PCB->path != NULL) {
    strcpy(nuevo_PCB->path, path);
	}else{
		exit(EXIT_FAILURE);
	}

	nuevo_PCB->registros_CPU = malloc(sizeof(registrosCPU));
	nuevo_PCB->registros_CPU->AX = 0;
	nuevo_PCB->registros_CPU->BX = 0;
	nuevo_PCB->registros_CPU->CX = 0;
	nuevo_PCB->registros_CPU->DX = 0;
	nuevo_PCB->registros_CPU->EAX = 0;
	nuevo_PCB->registros_CPU->EBX = 0;
	nuevo_PCB->registros_CPU->ECX = 0;
	nuevo_PCB->registros_CPU->EDX = 0;
	nuevo_PCB->registros_CPU->SI = 0;
	nuevo_PCB->registros_CPU->DI = 0;

	nuevo_PCB->estado = NEW;

	// INTERFACES
	
	nuevo_PCB->motivo_bloqueo = BLOQUEO_NO_DEFINIDO;
	nuevo_PCB->pedido_a_interfaz = malloc(sizeof(pedido_interfaz));
	nuevo_PCB->pedido_a_interfaz->nombre_interfaz = NULL;
	nuevo_PCB->pedido_a_interfaz->instruccion_a_interfaz = INSTRUCCION_IO_NO_DEFINIDA;
	nuevo_PCB->pedido_a_interfaz->datos_auxiliares_interfaz = list_create();
	
	// RECURSOS
	nuevo_PCB->pedido_recurso = NULL; 
	nuevo_PCB->recursos_en_uso = list_create();
	
	return nuevo_PCB;
}

// Semáforos que necesita para funcionar:
// - sem_lista_new

// PLANIFICADOR LARGO PLAZO
// Agrega a ready cuando proceso llega a new (Solamente por consola)
void planificador_largo_plazo()
{
	log_trace(kernel_logger,"PLP inicializado");

	while (1)
	{

		_check_interrupt_plp();

		// Es señalizado por:
		// - Consola
		// - Propio PLP
		sem_wait(&sem_lista_new);
		log_info(kernel_logger,"PLP: Planificando");

		pcb *un_pcb = NULL;

		pthread_mutex_lock(&mutex_lista_new);
		un_pcb = list_remove(new, 0);
		pthread_mutex_unlock(&mutex_lista_new);

		if (un_pcb != NULL)
		{

			// Envía la orden de iniciar estructura a memoria y espera con semáforo a que memoria la cree
			iniciar_estructura_en_memoria(un_pcb);
			log_info(kernel_logger,"PLP: Planificando");
			if (flag_respuesta_creacion_proceso == 0) 
			{

				log_info(kernel_logger,"No se pudo asignar memoria para el proceso con PID: %d",un_pcb->pid);
				
				// Reinicio la bendera
				flag_respuesta_creacion_proceso = 1; // Asumo que no necesito mutex porque plp es el único que accede a este flag y son ejecuciones secuenciales

				// Ejecuto while nuevamente
				continue;
			}

			list_add_pcb_sync(ready,un_pcb,&mutex_lista_ready,READY);
			log_info(kernel_logger,"Cola Ready:");
			mostrar_contenido(ready,&mutex_lista_ready);

			log_info(kernel_logger, " PID: %d - SET: READY", un_pcb->pid);

			sem_post(&sem_pcp);
			
			sem_wait(&sem_multiprogramacion);
		}
		else
		{
			log_warning(kernel_logger, "Lista NEW vacía");
		}
	}
}

/*





















*/

// PLANIFICADOR CORTO PLAZO
// Método de planificación: FIFO, RR, VRR.

// En que casos debería planificar?
// 	- Llegada a READY y lista EXEC vacía
// 	- Salida por I/O				(Osea salida de exec)
// 	- Salida por fin de proceso		(Osea salida de exec)
// 	- Salida por fin de quantum		(Osea salida de exec)
//  - Salida por interrupción desde consola
//	- => CUANDO EXEC ESTÁ VACÍA

void planificador_corto_plazo()
{ // Controla todo el tiempo la lista ready y ready_plus en caso de VRR
	log_trace(kernel_logger,"PCP inicializado");
	while (1)
	{

		_check_interrupt_pcp();
		// Espero a que se agregue algo a ready

		// Es señalizado por:
		// - PLP
		// - Fin de instrucción IO
		// - Volver de pedido de recurso
		// - Volver por fin de Q
		log_info(kernel_logger,"PCP: Esperando señal para planificar");
		sem_wait(&sem_pcp);
		log_info(kernel_logger,"PCP: Esperando a que la CPU esté libre");

		// Es señalizado por:
		// - Salida por fin de Q
		// - Salida por fin de proceso (Consola o natural)
		// - Salida por pedido de instrucción IO
		// - Salida por WAIT
		sem_wait(&sem_cpu_libre);
		log_info(kernel_logger,"PCP: Planificando");

		planificar_corto_plazo();

	}
}

void planificar_corto_plazo()
{

	// BLOQUEO LISTA EXEC HASTA QUE TERMINE EL PCP
	pthread_mutex_lock(&mutex_lista_exec);

	if (list_is_empty(execute))
	{

		pthread_mutex_unlock(&mutex_lista_exec);
		pcb *un_pcb = NULL;

		pthread_mutex_lock(&mutex_lista_ready);
		pthread_mutex_lock(&mutex_lista_ready_plus);

		if (!list_is_empty(ready_plus))
		{
			un_pcb = list_remove(ready_plus, 0);
		}
		else if (!list_is_empty(ready))
		{
			un_pcb = list_remove(ready, 0); // Me trae el primer elemento de la lista ready
		}
		pthread_mutex_unlock(&mutex_lista_ready);
		pthread_mutex_unlock(&mutex_lista_ready_plus);

		_poner_en_ejecucion(un_pcb);

	}
	else{
		
		pthread_mutex_unlock(&mutex_lista_exec);

	}
}

void _poner_en_ejecucion(pcb *un_pcb)
{

	if (un_pcb != NULL)
	{
		
		list_add_pcb_sync(execute,un_pcb,&mutex_lista_exec,EXEC);
		log_info(kernel_logger, " PID: %d - SET: EXEC", un_pcb->pid);

		un_pcb->ticket = generar_ticket();

		enviar_pcb_CPU_dispatch(un_pcb);

		if (strcmp(ALGORITMO_PLANIFICACION, "RR") == 0)
		{
			ejecutar_en_hilo_detach((void *)_programar_interrupcion_por_quantum_RR, un_pcb);
		}
		else if (strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0)
		{
			ejecutar_en_hilo_detach((void *)_programar_interrupcion_por_quantum_VRR, un_pcb);
		}
	}
	else
	{
		log_warning(kernel_logger, "Se intento _poner_en_ejecucion pero la lista READY está vacía");
		sem_post(&sem_cpu_libre);
	}
}

void _programar_interrupcion_por_quantum_RR(pcb *un_pcb)
{

	int ticket_referencia = un_pcb->ticket;
	log_warning(kernel_logger,"MÓDULO: control_procesos - Esperando %d milisegundos",un_pcb->quantum/1000);
	usleep(QUANTUM);

	pthread_mutex_lock(&mutex_ticket);
	if (ticket_referencia == ticket_actual)
	{
		log_warning(kernel_logger,"MÓDULO: control_procesos - Enviando interrupción a CPU por fin de Quantum");
		_gestionar_interrupcion(un_pcb, INTERRUPCION_QUANTUM);
	}
	pthread_mutex_unlock(&mutex_ticket);
	
}

void _programar_interrupcion_por_quantum_VRR(pcb *un_pcb)
{
	log_info(kernel_logger,"Programando interrupción por VRR");
	int ticket_referencia = un_pcb->ticket;
	int tiempo_restante = un_pcb->quantum - un_pcb->tiempo_ejecutado; 	// Consultar si está ok
	log_warning(kernel_logger,"MÓDULO: control_procesos - Tiempo quantum %d",un_pcb->quantum/1000);
	log_warning(kernel_logger,"MÓDULO: control_procesos - Tiempo ejecutado %d",un_pcb->tiempo_ejecutado/1000);															// El tiempo debe ser calculado en base a microsegundos
	if(tiempo_restante<=0){
		log_warning(kernel_logger,"MÓDULO: control_procesos - Esperando %d milisegundos para volver a ejecutar (<0)",QUANTUM/1000);
		usleep(QUANTUM);
		pthread_mutex_lock(&mutex_ticket);
		if (ticket_referencia == ticket_actual)
		{
			_gestionar_interrupcion(un_pcb, INTERRUPCION_QUANTUM);
		}
		pthread_mutex_unlock(&mutex_ticket);
	}else{
		log_warning(kernel_logger,"MÓDULO: control_procesos - Esperando %d milisegundos para volver a ejecutar (>0)",tiempo_restante/1000);
		usleep(tiempo_restante);
		pthread_mutex_lock(&mutex_ticket);
		if (ticket_referencia == ticket_actual)
		{
			log_warning(kernel_logger,"MÓDULO: control_procesos - Enviando interrupción a CPU por fin de Quantum (VRR)");
			_gestionar_interrupcion(un_pcb, INTERRUPCION_QUANTUM);
		}
		pthread_mutex_unlock(&mutex_ticket);	
	}
										  
	
}

// En teoría lo anterior está ok porque cuando un proceso vuelva porque se interrumpió su quantum
// la CPU debería haberme devuelto el proceso con su contexto de ejecución

void _check_interrupt_pcp()
{
	pthread_mutex_lock(&mutex_interrupt_pcp);
	switch (flag_interrupt_pcp)
	{
	case 0:
		pthread_mutex_unlock(&mutex_interrupt_pcp);
		sem_wait(&sem_interrupt_pcp); // Espera hasta que el semáforo sea señalizado
		break;

	default:
		pthread_mutex_unlock(&mutex_interrupt_pcp);
		break;
	}
	
}

void _check_interrupt_plp()
{
	pthread_mutex_lock(&mutex_interrupt_plp);
	switch (flag_interrupt_plp)
	{
	case 0:
		pthread_mutex_unlock(&mutex_interrupt_plp);
		sem_wait(&sem_interrupt_plp); // Espera hasta que el semáforo sea señalizado
		break;

	default:
		pthread_mutex_unlock(&mutex_interrupt_plp);
		break;
	}
	
}

/*





















*/

// MANEJA PROCESOS QUE SE BLOQUEAN EN CPU
void manejar_bloqueo_de_proceso(pcb *un_pcb)
{
	log_trace(kernel_logger,"MÓDULO - control_procesos: Manejando bloqueo deproceso con PID: %d",un_pcb->pid);
	// Cuando CPU me pide que lo bloquee tengo que sacarlo de exec si cumple con las siguientes condiciones
	// 		- La interfaz existe y se encuentra conectada (Tengo que tener lista de interfaces con su nombre como índice y
	//		  luego socket correspondiente)
	//		- La interfaz admite la operacion solicitada (En la lista anterior tambiém instrucciones que puede manejar?)
	//				-> Base de datos con nombres, sockets (dinámico) y instrucciones que pueden aceptar?

	// =========================================================================================================================

	switch (un_pcb->motivo_bloqueo)
	{
	case PEDIDO_A_INTERFAZ:
		
		log_info(kernel_logger,"PID: <%d> - Bloqueado por pedido a interfaz: %s",un_pcb->pid,un_pcb->pedido_a_interfaz->nombre_interfaz);
		ejecutar_en_hilo_detach((void *)manejar_pedido_a_interfaz, un_pcb);

	break;

	case WAIT:
		
		log_info(kernel_logger,"PID: <%d> - Bloqueado por recurso: %s",un_pcb->pid,un_pcb->pedido_recurso);
		ejecutar_en_hilo_detach((void *)manejar_pedido_de_recurso, un_pcb);
	
	break;

	case SIGNAL:

		log_trace(kernel_logger,"PID: <%d> - Bloqueado por recurso: %s",un_pcb->pid,un_pcb->pedido_recurso);
		ejecutar_en_hilo_detach((void *)manejar_signal_de_recurso, un_pcb);

	break;

	default:
		break;
	}
}
