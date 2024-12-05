#include "../include/paginacion.h"

double ceil(double x) {
    int int_part = (int)x;
    if (x > (double)int_part) {
        return (double)(int_part + 1);
    } else {
        return (double)int_part;
    }
}

t_marco* buscar_marco_segun_numero(int num_marco){
    t_marco* un_marco;

    pthread_mutex_lock(&mutex_lista_marcos);
    un_marco = list_get(lista_marcos, num_marco);
    pthread_mutex_unlock(&mutex_lista_marcos);
    log_trace(memoria_logger, "PRUEBAS: encontre marco");
    return un_marco;
}

int devolver_numero_de_marco(t_proceso* proceso,int num_pagina){
    pthread_mutex_lock(&(proceso->mutex_tabla_paginas));
    int cantidad_paginas= list_size(proceso->tabla_paginas);
    if(num_pagina < cantidad_paginas){
        t_tabla_de_pagina* fila = list_get(proceso->tabla_paginas, num_pagina);
        pthread_mutex_unlock(&(proceso->mutex_tabla_paginas));
        log_trace(memoria_logger, "PRUEBAS: encontre numero de marco");
        return fila->num_marco;
    }else{
        pthread_mutex_unlock(&(proceso->mutex_tabla_paginas));
        log_error(memoria_logger,"El proceso no contiene la pagina <%d>", num_pagina);
        return -1;
    }
     
    
}


//-------------------------------------------------------------------------------------------------------
// TABLA DE PAGINAS

void inicializar_tabla_de_paginas(t_proceso* nuevo_proceso){
    int cant_paginas =(int)ceil(nuevo_proceso->size/TAM_PAGINA);

    for(int i=0; i < cant_paginas ;i++){
        t_pagina* pagina_nueva = malloc(sizeof(t_pagina));
        pagina_nueva->nro_pagina = i;
        pagina_nueva->nro_marco = 0;

        pthread_mutex_lock(&(nuevo_proceso->mutex_tabla_paginas));
        list_add(nuevo_proceso->tabla_paginas,pagina_nueva);
        pthread_mutex_unlock(&(nuevo_proceso->mutex_tabla_paginas));
    }
}

//---------------------------------------------------------------

int cantidad_paginas_necesarias(int tamanio){
    int cantida_necesarias = (int)ceil((double)tamanio / TAM_PAGINA);
    return cantida_necesarias;
}

//-------------------------------------------------------------------------------------------------------
// FRAMES

t_marco* crear_frame(int tamBase, bool disponible, int numero_marco){
    t_marco *marco = malloc(sizeof(t_marco));
	marco->nro_marco = numero_marco;
	marco->base = tamBase;
	marco->disponible = disponible;
    marco->proceso = NULL;
    marco->num_pagina = 0;
    marco->cantidad_usado =0;
    marco->queda_lugar_disponible = true;	

    log_trace(memoria_logger, "PRUEBAS: se creo un marco");
	return marco;
}

void poner_en_disponible_frame(t_marco* un_marco){
    un_marco->disponible = true;
    un_marco->proceso = NULL;
    un_marco->cantidad_usado =0;
    un_marco->queda_lugar_disponible = true;
    log_trace(memoria_logger, "PRUEBAS: se puso en disonible un  marco");
}

void free_marco(t_marco* marco){
    free(marco);
}

// TENGO Q USAR SEMAFOROS??

//-------------------------------------------------------------------------------------

void inicializar_marco_para_un_proceso(t_marco* un_marco,t_proceso* proceso, int num_pagina){
    
    un_marco->num_pagina = num_pagina;
    un_marco->proceso = proceso;
    un_marco->cantidad_usado = 0;
    un_marco->queda_lugar_disponible = true; 
}

//-------------------------------------------------------------------------------------

void agregar_marco_pagina_a_tabla(t_proceso* proceso, int num_pagina, int nro_marco){

    t_tabla_de_pagina* pagina = malloc(sizeof(t_tabla_de_pagina));
    pagina->num_pagina = num_pagina;
    pagina->num_marco = nro_marco;

    pthread_mutex_lock(&(proceso->mutex_tabla_paginas));
    list_add(proceso->tabla_paginas, pagina);
    pthread_mutex_unlock(&(proceso->mutex_tabla_paginas));

    log_trace(memoria_logger, "PRUEBAS: se agrego pagina a la tabla de pagina");
}

//-----------------------------------------
void asignar_marco_disponible_a_proceso_vacio(t_proceso* proceso){
    t_marco* un_marco = obtener_frame_disponible();
    int num_pagina = 0;

    inicializar_marco_para_un_proceso(un_marco,proceso, num_pagina);
    agregar_marco_pagina_a_tabla(proceso, num_pagina, (un_marco->nro_marco));
}

//-------------------------------------------------------------------------------------------------------
t_marco* obtener_frame_disponible(){
	t_marco* un_marco = NULL;

	bool _marco_libre(t_marco* un_marco){
		return un_marco->disponible;
	}
	pthread_mutex_lock(&mutex_lista_marcos);
	un_marco = list_find(lista_marcos, (void*)_marco_libre);
	
	if(un_marco == NULL){
		//ERROR OUT OF MEMORY
        log_error(memoria_logger,"Error: out of memory");
        fprintf(stderr, "Error: Out Of Memory\n");
        exit(EXIT_FAILURE);
	}
		
	un_marco->disponible = false;
	pthread_mutex_unlock(&mutex_lista_marcos);

	return un_marco;
}




//-------------------------------------------------------------------------------------------------------

int obtener_num_marco_a_partir_de_dir_fisica(int dir_fisica){
	int nro_marco = dir_fisica / TAM_PAGINA;
	return nro_marco;
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//---------------------------------------------------- VIEJO
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
/*
bool obtener_frame_disponible_simulacion(t_proceso* proceso){
    t_marco* un_marco = NULL;

	bool _marco_libre(t_marco* un_marco){
		return un_marco->disponible;
	}
	pthread_mutex_lock(&mutex_lista_marcos);
	un_marco = list_find(lista_marcos, (void*)_marco_libre);
	
	if(un_marco == NULL){
		return false;
	}
		
	un_marco->disponible = false;
    un_marco->proceso = proceso;
	pthread_mutex_unlock(&mutex_lista_marcos);

	return true;
}*/
