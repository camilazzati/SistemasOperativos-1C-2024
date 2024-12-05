#include "../include/operaciones_proceso.h"


//--------------------------------------------------------------------------------------------

t_proceso* obtener_proceso_por_pid(int pid){
	
	bool _buscar_el_pid(t_proceso* proceso){
		return (proceso->pid_proceso) == pid;
	}
	
	pthread_mutex_lock(&mutex_lista_procesos);
	t_proceso* un_proceso = list_find(lista_procesos, (void*)_buscar_el_pid);
	pthread_mutex_unlock(&mutex_lista_procesos);

	if(un_proceso == NULL){
		log_error(memoria_logger, "PID<%d> No encontrado en la lista de procesos", pid);
		return NULL;
	}
	log_trace(memoria_logger, "PRUEBAS: encontre el procceso por pid");
	return un_proceso;
}
//--------------------------------------------------------------------------------------------


char*  extraer_instruccion_por_ip(t_proceso* proceso, int ip_proceso){

    if (ip_proceso < 0 ||ip_proceso >= list_size(proceso->lista_de_instrucciones)) {
        log_error(memoria_logger, "PID: <%d> - Índice de instrucción  <%d> NO VALIDO", proceso->pid_proceso, ip_proceso);
        return NULL;
    }

    char* instruccion = list_get(proceso->lista_de_instrucciones, ip_proceso);

	log_trace(memoria_logger, "PRUEBAS: encontre la instruccion por ip");

    return instruccion;
}
//---------------------------------------------------------------------------------------

void crear_proceso_nuevo(int pid, char* path){

	t_proceso* nuevo_proceso = malloc(sizeof(t_proceso));
    nuevo_proceso->pid_proceso = pid;
    nuevo_proceso->size = 0;
    nuevo_proceso->pathInstrucciones = path;
    nuevo_proceso->tabla_paginas = list_create();
	pthread_mutex_init(&(nuevo_proceso->mutex_tabla_paginas), NULL);
    nuevo_proceso->lista_de_instrucciones = obtener_instrucciones_del_archivo(nuevo_proceso->pathInstrucciones);
    
	inicializar_tabla_de_paginas(nuevo_proceso);

	pthread_mutex_lock(&mutex_lista_procesos);
    list_add(lista_procesos, nuevo_proceso);
	pthread_mutex_unlock(&mutex_lista_procesos);
    
	//deberia ser 0
    int cantidad_paginas = list_size(nuevo_proceso->tabla_paginas);
    log_info(memoria_logger,"Se creo un proceso: PID: <%d>- Tamaño: <%d>",nuevo_proceso->pid_proceso,cantidad_paginas);

}


//---------------------------------------------------------------------------------------

void destruir_tabla_de_paginas(t_proceso* proceso){
	int pid = proceso->pid_proceso;
	int cantidad_paginas = list_size(proceso->tabla_paginas);
	
	void _liberar_paginas(t_tabla_de_pagina* pagina){
		t_marco* marco = buscar_marco_segun_numero(pagina->num_marco);
		poner_en_disponible_frame(marco);
		free(pagina);
	}
    
	pthread_mutex_lock(&(proceso->mutex_tabla_paginas));
	list_destroy_and_destroy_elements(proceso->tabla_paginas, (void*)_liberar_paginas);
	pthread_mutex_unlock(&(proceso->mutex_tabla_paginas));

	log_info(memoria_logger, "Destruccion tabla de Paginas - PID: <%d> - Tamaño: <%d>", pid, cantidad_paginas);
}


// 1) libero el path
// 2) elimino cada instruccion
// 3) elimino tablas 
// 4) elimino proceso

void destruir_proceso(t_proceso* proceso){
	free(proceso->pathInstrucciones);

	void _liberar_instrucciones(char* instruccion){
		free(instruccion);
	}

	list_destroy_and_destroy_elements(proceso->lista_de_instrucciones,(void*)_liberar_instrucciones);

    destruir_tabla_de_paginas(proceso);

	pthread_mutex_destroy(&(proceso->mutex_tabla_paginas));
	

	pthread_mutex_lock(&mutex_lista_procesos);
	list_remove_element(lista_procesos, proceso);
	pthread_mutex_unlock(&mutex_lista_procesos);
	
	free(proceso);
	log_trace(memoria_logger,"PRUEBAS: Se libero el proceso");

}
