#include "../include/cicloInstruccion.h"

void iniciar_tiempo(){
    gettimeofday(&inicio, NULL);
}

void calcularTiempoEjecutado(){
    gettimeofday(&fin, NULL);
    long segundos = fin.tv_sec - inicio.tv_sec;
    long microsegundos = fin.tv_usec - inicio.tv_usec;
    double tiempo_transcurrido = segundos * 1000 + (microsegundos / 1000.0);
    contexto->proceso_tiempo_ejecutado = tiempo_transcurrido;
}

void liberar_instruccion_dividida(){

    int i = 0;
    while(instruccion_dividida[i] != NULL){
        i++;
        free(instruccion_dividida[i]);
    }

}

void realizarCicloInstruccion(){
    int while_ciclo = 1;

    while(while_ciclo){

    hayQueDesalojar = false;
    hay_interrupcion_exit = false;
    hay_interrupcion_quantum = false;

    // FETCH (solicita y recibe instruccion de memoria)
    ciclo_de_instruccion_fetch();


    // esperar que llegue la instruccion
    sem_wait(&sem_pedido_instruccion);
    // DECODE Y EXECUTE
    decodeYExecute(); 
    liberar_instruccion_dividida();
    free(instruccion_dividida);

    mostrar_pcb();

    
    // CHECK INTERRUPTION
    if(hayQueDesalojar){ //llamada a IO, desaloja voluntariamente

        log_warning(cpu_logger, "Desalojando");
        
        while_ciclo = 0;

    } else if(hay_interrupcion_quantum){
        log_warning(cpu_logger, "Interrupcion Quantum");

        t_paquete* unPaquete = crear_paquete_con_buffer(INTERRUPCION_QUANTUM);

        enviarContextoAKernel_sinTiempo(unPaquete);
        
        while_ciclo = 0;

    } else if(hay_interrupcion_exit){
        log_warning(cpu_logger, "Interrupcion Exit");

        t_paquete* unPaquete = crear_paquete_con_buffer(USER_INTERRUPT);

        enviarContextoAKernel(unPaquete);
        
        while_ciclo = 0;
    }
    }
}

void ciclo_de_instruccion_fetch(){

    log_info(cpu_logger, "PID: <%d> - FETCH - Program Counter: <%d>", contexto->proceso_pid, contexto->proceso_pc);
    t_paquete* un_paquete = crear_paquete_con_buffer(SOLICITUD_INSTRUCCION);
    cargar_int_a_paquete(un_paquete, contexto->proceso_pid);
    cargar_int_a_paquete(un_paquete, contexto->proceso_pc);
    enviar_paquete(un_paquete, fd_memoria);
    eliminar_paquete(un_paquete);
}

void decodeYExecute(){ 
    // strcmp compara cadenas
   
    if(strcmp(instruccion_dividida[0], "SET") == 0) { //SET(registro, valor)
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2]);
       
        contexto->proceso_pc++; // aumenta PC

        uint32_t* registro = detectar_registro(instruccion_dividida[1]); //registro
        *registro = atoi(instruccion_dividida[2]);
        
    } else if(strcmp(instruccion_dividida[0], "SUM") == 0){ //SUM(registroDestino, registroOrigen)
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2]);
       
        contexto->proceso_pc++; // aumenta PC

        uint32_t* registro_destino = detectar_registro(instruccion_dividida[1]); // registro destino
        uint32_t* registro_origen = detectar_registro(instruccion_dividida[2]); // registro origen

        *registro_destino = *registro_destino + *registro_origen;
        
    } else if(strcmp(instruccion_dividida[0], "SUB") == 0){ //SUB(registroDestino, registroOrigen)
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2]);
       
        contexto->proceso_pc++; // aumenta PC
       
        uint32_t* registro_destino = detectar_registro(instruccion_dividida[1]); // registro destino
        uint32_t* registro_origen = detectar_registro(instruccion_dividida[2]); // registro origen

        *registro_destino = *registro_destino - *registro_origen;

    } else if(strcmp(instruccion_dividida[0], "JNZ") == 0){ //JNZ(registro, instruccion)
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2]);
       
        uint32_t* registro = detectar_registro(instruccion_dividida[1]); //registro
        
        int valorNuevoPC = atoi(instruccion_dividida[2]);

         if(*registro != 0){
            contexto->proceso_pc = valorNuevoPC;
        }

    } else if(strcmp(instruccion_dividida[0], "IO_GEN_SLEEP") == 0){ //IO_GEN_SLEEP(interfaz, unidades de trabajo)
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2]);
       
        contexto->proceso_pc++; // aumenta PC
        
        int instruccion_interfaz = 0;
        
        t_paquete* unPaquete = crear_paquete_con_buffer(ATENDER_INSTRUCCION_CPU);

        cargar_int_a_paquete(unPaquete, instruccion_interfaz); // instruccion
        cargar_string_a_paquete(unPaquete, instruccion_dividida[1]); // interfaz 
        cargar_int_a_paquete(unPaquete, atoi(instruccion_dividida[2])); // unidades de tiempo

        enviarContextoAKernel(unPaquete);

        hayQueDesalojar = true;

    } else if(strcmp(instruccion_dividida[0], "MOV_IN") == 0){ //MOV_IN(registro datos, registro direccion)
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2]);

        contexto->proceso_pc++; // aumenta PC   
       
        uint32_t* registro_datos = detectar_registro(instruccion_dividida[1]); 
        uint32_t* registro_direccion = detectar_registro(instruccion_dividida[2]); 

        int direccion_logica = *registro_direccion;

        char* valor = leer_valor_memoria(direccion_logica, sizeof(*registro_datos));
        
        if(strcmp(valor, "ERROR") != 0){ // si no hubo PF
            int un_valor = atoi(valor);
            
            *registro_datos = (uint32_t)un_valor;
        } 
        free(valor);

    } else if (strcmp(instruccion_dividida[0], "MOV_OUT") == 0){ // MOV_OUT(registro direccion, registro datos)
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2]);

        contexto->proceso_pc++; // aumenta PC  

        uint32_t* registro_datos = detectar_registro(instruccion_dividida[2]); 
        uint32_t* registro_direccion = detectar_registro(instruccion_dividida[1]); 

        int direccion_logica = *registro_direccion;

        int un_valor = *registro_datos;

        char* valor_a_escribir = malloc(sizeof(char)*2);
        valor_a_escribir[0] = (char)un_valor;
        valor_a_escribir[1] = '\0';

        escribir_valor_memoria(direccion_logica, valor_a_escribir, strlen(valor_a_escribir)); // se fija aca adentro si hubo PF

        free(valor_a_escribir);

    } else if (strcmp(instruccion_dividida[0], "RESIZE") == 0){ // RESIZE(tamaño)
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1]);

        contexto->proceso_pc++; // aumenta PC
        
        int tamanio = atoi(instruccion_dividida[1]); 

        t_paquete* unPaquete = crear_paquete_con_buffer(AJUSTAR_TAMANIO);
        cargar_int_a_paquete(unPaquete, contexto->proceso_pid); //para que memoria sepa el proceso
        cargar_int_a_paquete(unPaquete, tamanio);

        enviar_paquete(unPaquete, fd_memoria); 
        eliminar_paquete(unPaquete);

        sem_wait(&sem_rta_resize);
        if(resultado == -1){ // si el resize dio out of memory
            log_trace(cpu_logger, "Rta de memoria: Out of Memory");

            t_paquete* unPaquete = crear_paquete_con_buffer(OUT_OF_MEMORY);

            enviarContextoAKernel(unPaquete);

            hayQueDesalojar = true;
        } 

    } else if (strcmp(instruccion_dividida[0], "COPY_STRING") == 0){ // COPY_STRING(tamaño)
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1]);

        contexto->proceso_pc++; // aumenta PC
        
        int tamanio = atoi(instruccion_dividida[1]);

        int direccion_logica_SI = (int)contexto->SI; // el registro contiene la direccion logica del string que quiero copiar
        char* valor = leer_valor_memoria(direccion_logica_SI, tamanio); // lee la direccion (tendria que ser un string pero leer devuelve un int como resuelvo?)
        
        if(strcmp(valor, "ERROR") != 0){
            int direccion_logica_DI = (int)contexto->DI; // registro en el que quiero escribir
            escribir_valor_memoria(direccion_logica_DI, valor, strlen(valor)); // escribe en la direccion de DI el valor de la direccion SI
        }
        free(valor);

    } else if (strcmp(instruccion_dividida[0], "IO_STDIN_READ") == 0){ // IO_STDIN_READ(interfaz, registro direccion, registro tamanio )
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s> <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2], instruccion_dividida[3]);

        contexto->proceso_pc++; // aumenta PC
       
        int instruccion_interfaz = 1;

        uint32_t* registro_tamanio = detectar_registro(instruccion_dividida[3]); 
        uint32_t* registro_direccion = detectar_registro(instruccion_dividida[2]); 

        int direccion_logica = *registro_direccion;
        int tamanio = *registro_tamanio;   

        int direccion_fisica = traducir(direccion_logica); 

        if(direccion_fisica != -1){

            t_paquete* unPaquete = crear_paquete_con_buffer(ATENDER_INSTRUCCION_CPU);

            cargar_int_a_paquete(unPaquete, instruccion_interfaz); // instruccion
            cargar_string_a_paquete(unPaquete, instruccion_dividida[1]); // interfaz
            cargar_int_a_paquete(unPaquete, direccion_fisica); // direccion
            cargar_int_a_paquete(unPaquete, tamanio); // tamanio

            enviarContextoAKernel(unPaquete);

            hayQueDesalojar = true;
        } else {
            log_error(cpu_logger, "No se pudo encontrar la Direccion Fisica");
            exit(EXIT_FAILURE);
        }

    } else if (strcmp(instruccion_dividida[0], "IO_STDOUT_WRITE") == 0){ // IO_STDOUT_WRITE(interfaz, registro direccion, registro tamanio )
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s> <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2], instruccion_dividida[3]);

        contexto->proceso_pc++; // aumenta PC
       
        int instruccion_interfaz = 2;

        uint32_t* registro_tamanio = detectar_registro(instruccion_dividida[3]); 
        uint32_t* registro_direccion = detectar_registro(instruccion_dividida[2]); 

        int direccion_logica = *registro_direccion;
        int tamanio = *registro_tamanio; 

        int direccion_fisica = traducir(direccion_logica); 

        if(direccion_fisica != -1){

            t_paquete* unPaquete = crear_paquete_con_buffer(ATENDER_INSTRUCCION_CPU);

            cargar_int_a_paquete(unPaquete, instruccion_interfaz); // instruccion
            cargar_string_a_paquete(unPaquete, instruccion_dividida[1]); // interfaz
            cargar_int_a_paquete(unPaquete, direccion_fisica); // direccion
            cargar_int_a_paquete(unPaquete, tamanio); // tamanio

            enviarContextoAKernel(unPaquete);

            hayQueDesalojar = true;
        } else {
            log_error(cpu_logger, "No se pudo encontrar la Direccion Fisica");
            exit(EXIT_FAILURE);
        }

    }  else if (strcmp(instruccion_dividida[0], "IO_FS_CREATE") == 0){ // IO_FS_CREATE(Interfaz, Nombre Archivo)
            log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s> ", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2]);

            contexto->proceso_pc++; // aumenta PC
            
            int instruccion_interfaz = 3;
            
            t_paquete* unPaquete = crear_paquete_con_buffer(ATENDER_INSTRUCCION_CPU);

            cargar_int_a_paquete(unPaquete, instruccion_interfaz); // instruccion
            cargar_string_a_paquete(unPaquete, instruccion_dividida[1]); // interfaz
            cargar_string_a_paquete(unPaquete, instruccion_dividida[2]); // nombre archivo

            enviarContextoAKernel(unPaquete);

            hayQueDesalojar = true;
   
    }  else if (strcmp(instruccion_dividida[0], "IO_FS_DELETE") == 0){ // IO_FS_DELETE(Interfaz, Nombre Archivo)
            log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s> ", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2]);

            contexto->proceso_pc++; // aumenta PC
            
            int instruccion_interfaz = 4;
           
            t_paquete* unPaquete = crear_paquete_con_buffer(ATENDER_INSTRUCCION_CPU);

            cargar_int_a_paquete(unPaquete, instruccion_interfaz); // instruccion
            cargar_string_a_paquete(unPaquete, instruccion_dividida[1]); // interfaz
            cargar_string_a_paquete(unPaquete, instruccion_dividida[2]); // nombre archivo

            enviarContextoAKernel(unPaquete);

            hayQueDesalojar = true;
    
    } else if (strcmp(instruccion_dividida[0], "IO_FS_TRUNCATE") == 0){ // IO_FS_TRUNCATE(Interfaz, Nombre Archivo, Registro tamanio)
            log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s> <%s> ", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2], instruccion_dividida[3]);

            contexto->proceso_pc++; // aumenta PC
            
            int instruccion_interfaz = 5;
           
            uint32_t* registro_tamanio = detectar_registro(instruccion_dividida[3]); 
        
           int tamanio = *registro_tamanio;

            t_paquete* unPaquete = crear_paquete_con_buffer(ATENDER_INSTRUCCION_CPU);

            cargar_int_a_paquete(unPaquete, instruccion_interfaz); // instruccion
            cargar_string_a_paquete(unPaquete, instruccion_dividida[1]); // interfaz
            cargar_string_a_paquete(unPaquete, instruccion_dividida[2]); // nombre archivo
            cargar_int_a_paquete(unPaquete, tamanio); // valor del registro tamanio

            enviarContextoAKernel(unPaquete);

            hayQueDesalojar = true;
    
    } else if (strcmp(instruccion_dividida[0], "IO_FS_WRITE") == 0){ // IO_FS_WRITE(Interfaz, Nombre Archivo, Registro direccion, registro tamanio, registro puntero archivo)
            log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s> <%s> <%s> <%s> ", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2], instruccion_dividida[3], instruccion_dividida[4], instruccion_dividida[5]);

            contexto->proceso_pc++; // aumenta PC
            
            int instruccion_interfaz = 6;

            uint32_t* registro_direccion = detectar_registro(instruccion_dividida[3]); 

            int direccion_logica = *registro_direccion;

            int direccion_fisica = traducir(direccion_logica); 

            uint32_t* registro_tamanio = detectar_registro(instruccion_dividida[4]); 
        
           int tamanio = *registro_tamanio;

            uint32_t* registro_puntero = detectar_registro(instruccion_dividida[5]); 
        
            int puntero = *registro_puntero;

            if(direccion_fisica != 1){
                t_paquete* unPaquete = crear_paquete_con_buffer(ATENDER_INSTRUCCION_CPU);

                cargar_int_a_paquete(unPaquete, instruccion_interfaz); // instruccion
                cargar_string_a_paquete(unPaquete, instruccion_dividida[1]); // interfaz
                cargar_string_a_paquete(unPaquete, instruccion_dividida[2]); // nombre archivo
                cargar_int_a_paquete(unPaquete, direccion_fisica); // valor registro direccion
                cargar_int_a_paquete(unPaquete, tamanio); // valor del registro tamanio
                cargar_int_a_paquete(unPaquete, puntero); // valor del registro puntero archivo

                enviarContextoAKernel(unPaquete);

                hayQueDesalojar = true;
            } else {
            log_error(cpu_logger, "No se pudo encontrar la Direccion Fisica");
            exit(EXIT_FAILURE);
        }
            
    } else if (strcmp(instruccion_dividida[0], "IO_FS_READ") == 0){ // IO_FS_READ(Interfaz, Nombre Archivo, Registro direccion, registro tamanio, registro puntero archivo)
            log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s> <%s> <%s> <%s> <%s> ", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1], instruccion_dividida[2], instruccion_dividida[3], instruccion_dividida[4], instruccion_dividida[5]);

            contexto->proceso_pc++; // aumenta PC
           
            int instruccion_interfaz = 7;
           
            uint32_t* registro_direccion = detectar_registro(instruccion_dividida[3]); 

            int  direccion_logica = *registro_direccion;
           
            int direccion_fisica = traducir(direccion_logica); 

            uint32_t* registro_tamanio = detectar_registro(instruccion_dividida[4]); 
        
            int tamanio = *registro_tamanio;

            uint32_t* registro_puntero = detectar_registro(instruccion_dividida[5]); 
        
            int puntero = *registro_puntero;

            if(direccion_fisica != 1){
                t_paquete* unPaquete = crear_paquete_con_buffer(ATENDER_INSTRUCCION_CPU);

                cargar_int_a_paquete(unPaquete, instruccion_interfaz); // instruccion
                cargar_string_a_paquete(unPaquete, instruccion_dividida[1]); // interfaz
                cargar_string_a_paquete(unPaquete, instruccion_dividida[2]); // nombre archivo
                cargar_int_a_paquete(unPaquete, direccion_fisica); // valor registro direccion
                cargar_int_a_paquete(unPaquete, tamanio); // valor del registro tamanio
                cargar_int_a_paquete(unPaquete, puntero); // valor del registro puntero archivo

                enviarContextoAKernel(unPaquete);

                hayQueDesalojar = true;
            } else {
            log_error(cpu_logger, "No se pudo encontrar la Direccion Fisica");
            exit(EXIT_FAILURE);
        }
            
    } else if(strcmp(instruccion_dividida[0], "WAIT") == 0){ //WAIT(recurso)
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1]);
      
        contexto->proceso_pc++; // aumenta PC
       
        t_paquete* unPaquete = crear_paquete_con_buffer(WAIT_KCPU);

        cargar_string_a_paquete(unPaquete, instruccion_dividida[1]); // recurso 

        enviarContextoAKernel(unPaquete);

        hayQueDesalojar = true;

    } else if(strcmp(instruccion_dividida[0], "SIGNAL") == 0){ //SIGNAL(recurso)
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s> - <%s>", contexto->proceso_pid, instruccion_dividida[0], instruccion_dividida[1]);
        
        contexto->proceso_pc++; // aumenta PC
        
        t_paquete* unPaquete = crear_paquete_con_buffer(SIGNAL_KCPU);

        cargar_string_a_paquete(unPaquete, instruccion_dividida[1]); // recurso 

        enviarContextoAKernel(unPaquete);

    }
    else if(strcmp(instruccion_dividida[0], "EXIT") == 0){
        log_info(cpu_logger, "PID: <%d>, Ejecutando: <%s>", contexto->proceso_pid, instruccion_dividida[0]);

        t_paquete* unPaquete = crear_paquete_con_buffer(EXIT_PROCESS);
        
        enviarContextoAKernel(unPaquete);

        hayQueDesalojar = true;

    } else {
        log_error(cpu_logger, "Instruccion no valida");
        exit(EXIT_FAILURE);
    }
}


uint32_t* detectar_registro(char* registro){
    if(strcmp(registro, "AX") == 0){
        return &(contexto->AX);
    }
    else if(strcmp(registro, "BX") == 0){
        return &(contexto->BX);
    }
    else if(strcmp(registro, "CX") == 0){
        return &(contexto->CX);
    }
    else if(strcmp(registro, "DX") == 0){
        return &(contexto->DX);
    }
    if(strcmp(registro, "EAX") == 0){
        return &(contexto->EAX);
    } 
    else if(strcmp(registro, "EBX") == 0){
        return &(contexto->EBX);
    } 
    else if(strcmp(registro, "ECX") == 0){
        return &(contexto->ECX);
    } 
    else if(strcmp(registro, "EDX") == 0){
        return &(contexto->EDX);
    } 
    else if(strcmp(registro, "SI") == 0){
        return &(contexto->SI);
    }
    else if(strcmp(registro, "DI") == 0){
        return &(contexto->DI);
    } 
    else if(strcmp(registro, "PC") == 0){
        return &(contexto->proceso_pc);
    } else {
        log_error(cpu_logger, "Registro no valido");
        exit(EXIT_FAILURE);
        return NULL;
    }
}

void enviarContextoAKernel(t_paquete* unPaquete){

    cargar_int_a_paquete(unPaquete, contexto->proceso_pid);    
    cargar_int_a_paquete(unPaquete, contexto->proceso_pc);

    cargar_uint32_a_paquete(unPaquete, contexto->AX);  
    cargar_uint32_a_paquete(unPaquete, contexto->BX);
    cargar_uint32_a_paquete(unPaquete, contexto->CX);
    cargar_uint32_a_paquete(unPaquete, contexto->DX);
    cargar_uint32_a_paquete(unPaquete, contexto->EAX);
    cargar_uint32_a_paquete(unPaquete, contexto->EBX);
    cargar_uint32_a_paquete(unPaquete, contexto->ECX);
    cargar_uint32_a_paquete(unPaquete, contexto->EDX);
    cargar_uint32_a_paquete(unPaquete, contexto->SI);
    cargar_uint32_a_paquete(unPaquete, contexto->DI);
    
    calcularTiempoEjecutado();

    cargar_int_a_paquete(unPaquete, contexto->proceso_tiempo_ejecutado);
    
    enviar_paquete(unPaquete, fd_kernel_dispatch); 
    eliminar_paquete(unPaquete);

}

void enviarContextoAKernel_sinTiempo(t_paquete* unPaquete){

    cargar_int_a_paquete(unPaquete, contexto->proceso_pid);    
    cargar_int_a_paquete(unPaquete, contexto->proceso_pc);

    cargar_uint32_a_paquete(unPaquete, contexto->AX);  
    cargar_uint32_a_paquete(unPaquete, contexto->BX);
    cargar_uint32_a_paquete(unPaquete, contexto->CX);
    cargar_uint32_a_paquete(unPaquete, contexto->DX);
    cargar_uint32_a_paquete(unPaquete, contexto->EAX);
    cargar_uint32_a_paquete(unPaquete, contexto->EBX);
    cargar_uint32_a_paquete(unPaquete, contexto->ECX);
    cargar_uint32_a_paquete(unPaquete, contexto->EDX);
    cargar_uint32_a_paquete(unPaquete, contexto->SI);
    cargar_uint32_a_paquete(unPaquete, contexto->DI);
    
    //calcularTiempoEjecutado();
    
    cargar_int_a_paquete(unPaquete, 0);
    
    enviar_paquete(unPaquete, fd_kernel_dispatch); 
    eliminar_paquete(unPaquete);

}

// ----------------------- MEMORIA ----------------------------

char* leer_valor_memoria(int direccion_logica, int tamanio){
    int direccion_fisica = traducir(direccion_logica);

    if(direccion_fisica == -1){
       
        log_warning(cpu_logger, "Direccion no válida");
        return "ERROR";

    } else {
        t_paquete* unPaquete = crear_paquete_con_buffer(SOLICITUD_LECTURA_MEMORIA_BLOQUE);
        cargar_int_a_paquete(unPaquete, contexto->proceso_pid);
        cargar_int_a_paquete(unPaquete, direccion_fisica);
        cargar_int_a_paquete(unPaquete, tamanio);
        enviar_paquete(unPaquete, fd_memoria);
        eliminar_paquete(unPaquete);

        sem_wait(&sem_solicitud_lectura);
        if (strcmp(respuesta_marco_lectura, "ERROR") != 0) {
            log_info(cpu_logger, "PID: <%d> - Accion: LEER - Direccion: <%d> - Valor: <%s>", contexto->proceso_pid, direccion_fisica, respuesta_marco_lectura);
        
            char* valor = respuesta_marco_lectura;
            //free(respuesta_marco_lectura);
            return valor;
            //free(valor);
        } else {
            log_warning(cpu_logger, "No se pudo leer");
            return "ERROR";
        }

    }
}

void escribir_valor_memoria(int direccion_logica, char* valor, int tamanio){
    int direccion_fisica = traducir(direccion_logica);

    if(direccion_fisica != -1){

        t_paquete* unPaquete = crear_paquete_con_buffer(SOLICITUD_ESCRITURA_MEMORIA_BLOQUE);
        cargar_int_a_paquete(unPaquete, contexto->proceso_pid);
        cargar_int_a_paquete(unPaquete, direccion_fisica);
        cargar_int_a_paquete(unPaquete, tamanio);
        cargar_string_a_paquete(unPaquete, valor); 
        enviar_paquete(unPaquete, fd_memoria);
        log_info(cpu_logger,"Mando a memorira: %s",valor);
        eliminar_paquete(unPaquete);

        sem_wait(&sem_solicitud_escritura);
        if (strcmp(respuesta_marco_escritura, "ERROR") != 0) {
            log_info(cpu_logger, "PID: <%d> - Accion: ESCRIBIR - Direccion: <%d> - Valor: <%s>", contexto->proceso_pid, direccion_fisica, valor);
        } else {
            log_warning(cpu_logger, "No se pudo escribir");
        }
        //free(respuesta_marco_escritura);
        //free(valor);
    }
}

// ----------------------- TRADUCCION ----------------------------

int traducir(int direccion_logica){

    int numero_pagina = floor(direccion_logica/tamanio_pagina);
    int direccion_fisica;
    if(CANTIDAD_ENTRADAS_TLB == 0){
        direccion_fisica = traducir_sin_TLB(direccion_logica, numero_pagina, tamanio_pagina);
        return direccion_fisica;
    }else {
        int marco_TLB = buscar_en_TLB(numero_pagina);

        if(marco_TLB == -1){ // No la encuentra en la tlb
            log_info(cpu_logger, "PID: <%d> - TLB MISS - Pagina: <%d>", contexto->proceso_pid, numero_pagina);

            direccion_fisica = mmu(direccion_logica, numero_pagina, tamanio_pagina);

            return direccion_fisica;

        } else { // La encuentra en la tlb
            log_info(cpu_logger, "PID: <%d> - TLB HIT - Pagina: <%d>", contexto->proceso_pid, numero_pagina);

            int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;
            direccion_fisica = marco_TLB * tamanio_pagina + desplazamiento;

            return direccion_fisica;
    }
    }
}

int traducir_sin_TLB(int direccion_logica, int numero_pagina, int tamanio_pagina){
    int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;

    // le pido la pag a memoria, si la tiene devuelve el marco y sino PF (-1)
    t_paquete* unPaquete = crear_paquete_con_buffer(SOLICITUD_PAGINA);
    cargar_int_a_paquete(unPaquete, contexto->proceso_pid);
    cargar_int_a_paquete(unPaquete, numero_pagina);
    enviar_paquete(unPaquete, fd_memoria);
    eliminar_paquete(unPaquete);

    sem_wait(&sem_pedido_marco);
    if(marco != -1){ // no hay PF
        log_info(cpu_logger, "PID: <%d> - OBTENER MARCO - Pagina: <%d> - Marco: <%d>", contexto->proceso_pid, numero_pagina, marco);

        int direccion_fisica = marco * tamanio_pagina + desplazamiento;
        
        return direccion_fisica;

    } else { // hay PF
        log_info(cpu_logger, "PID: <%d> - PAGE FAULT - Pagina: <%d>", contexto->proceso_pid, numero_pagina);

        t_paquete* unPaquete = crear_paquete_con_buffer(EXIT_PROCESS);

        enviarContextoAKernel(unPaquete);

        hayQueDesalojar = true;

        return -1;
    }
}

int mmu(int direccion_logica, int numero_pagina, int tamanio_pagina){
     
    int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;

    // le pido la pag a memoria, si la tiene devuelve el marco y sino PF (-1)
    t_paquete* unPaquete = crear_paquete_con_buffer(SOLICITUD_PAGINA);
    cargar_int_a_paquete(unPaquete, contexto->proceso_pid);
    cargar_int_a_paquete(unPaquete, numero_pagina);
    enviar_paquete(unPaquete, fd_memoria);
    eliminar_paquete(unPaquete);

    sem_wait(&sem_pedido_marco);
    if(marco != -1){ // no hay PF
        log_info(cpu_logger, "PID: <%d> - OBTENER MARCO - Pagina: <%d> - Marco: <%d>", contexto->proceso_pid, numero_pagina, marco);

        int direccion_fisica = marco * tamanio_pagina + desplazamiento;
        
        agregar_entrada_TLB(numero_pagina, marco);

        return direccion_fisica;

    } else { // hay PF
        log_info(cpu_logger, "PID: <%d> - PAGE FAULT - Pagina: <%d>", contexto->proceso_pid, numero_pagina);

        t_paquete* unPaquete = crear_paquete_con_buffer(EXIT_PROCESS);

        enviarContextoAKernel(unPaquete);

        hayQueDesalojar = true;

        return -1;
    }
}

// ----------------------- TLB ----------------------------

int buscar_en_TLB(int numero_pagina){
    int marco = -1;

    for(int i = 0; i < tlb->tamanio; i++){
        if(tlb->entradas[i].pid == contexto->proceso_pid && tlb->entradas[i].pagina == numero_pagina && tlb->entradas[i].estado == OCUPADA){
            marco = tlb->entradas[i].marco;
            temporal_destroy(tlb->entradas[i].ultimo_uso);
            tlb->entradas[i].ultimo_uso = temporal_create();
        }
    }

    log_trace(cpu_logger, "busqueda en tlb, marco: <%d>", marco);

    return marco;
}


void agregar_entrada_TLB(int numero_pagina, int marco){
    int hay_libre = 0; // si hay entrada libre (no ocupada)

    for(int i = 0; i < tlb->tamanio && hay_libre == 0; i++){
        if(tlb->entradas[i].estado == LIBRE) {
            tlb->entradas[i].pid = contexto->proceso_pid;
            tlb->entradas[i].pagina = numero_pagina;
            tlb->entradas[i].marco = marco;
            tlb->entradas[i].estado = OCUPADA;

            pthread_mutex_lock(&mutex_ordenCargaGlobal);
            tlb->entradas[i].orden_carga = ordenCargaGlobal;
            ordenCargaGlobal++;
            pthread_mutex_unlock(&mutex_ordenCargaGlobal);
            temporal_destroy(tlb->entradas[i].ultimo_uso);
            tlb->entradas[i].ultimo_uso = temporal_create();
        

            hay_libre = 1; // encontro entrada libre y la ocupo

            log_trace(cpu_logger, "encontro entrada libre");

            break;
        }
    }

    if(hay_libre == 0){ //no encontro ninguna entrada libre, hay que reemplazar alguna segun el algoritmo
      
        if(strcmp(ALGORITMO_TLB, "FIFO") == 0){ // FIFO
            log_trace(cpu_logger, "FIFO");
            int menorIndice = 0;
           for(int i = 1; i < tlb->tamanio; i++){ // busco la entrada con el menor orden de carga (la que esta hace mas tiempo) y la reemplazo por la nueva
                if(tlb->entradas[i].orden_carga < tlb->entradas[menorIndice].orden_carga){
                    menorIndice = i;
                }
           }

            log_trace(cpu_logger, "Reemplazando entrada <%d> del proceso <%d> con numero_pagina <%d> y marco <%d>", menorIndice, tlb->entradas[menorIndice].pid, tlb->entradas[menorIndice].pagina, tlb->entradas[menorIndice].marco);

            tlb->entradas[menorIndice].pid = contexto->proceso_pid;
            tlb->entradas[menorIndice].pagina = numero_pagina;
            tlb->entradas[menorIndice].marco = marco;
            tlb->entradas[menorIndice].estado = OCUPADA;

            pthread_mutex_lock(&mutex_ordenCargaGlobal);
            tlb->entradas[menorIndice].orden_carga = ordenCargaGlobal;
            ordenCargaGlobal++;
            pthread_mutex_unlock(&mutex_ordenCargaGlobal);
            temporal_destroy(tlb->entradas[menorIndice].ultimo_uso);
            tlb->entradas[menorIndice].ultimo_uso = temporal_create();
           


        }else if (strcmp(ALGORITMO_TLB, "LRU") == 0){ //LRU
           log_trace(cpu_logger, "LRU");
            int mayorIndice = 0;
           for(int i = 1; i < tlb->tamanio; i++){ // busco la entrada con el mayor ultimo_uso (el mas antiguo) y la reemplazo por la nueva
                if(temporal_gettime(tlb->entradas[i].ultimo_uso) > temporal_gettime(tlb->entradas[mayorIndice].ultimo_uso)){
                    mayorIndice = i;
                }
           }

            log_trace(cpu_logger, "Reemplazando entrada <%d> del proceso <%d> con numero_pagina <%d> y marco <%d>", mayorIndice, tlb->entradas[mayorIndice].pid, tlb->entradas[mayorIndice].pagina, tlb->entradas[mayorIndice].marco);

            tlb->entradas[mayorIndice].pid = contexto->proceso_pid;
            tlb->entradas[mayorIndice].pagina = numero_pagina;
            tlb->entradas[mayorIndice].marco = marco;
            tlb->entradas[mayorIndice].estado = OCUPADA;

            pthread_mutex_lock(&mutex_ordenCargaGlobal);
            tlb->entradas[mayorIndice].orden_carga = ordenCargaGlobal;
            ordenCargaGlobal++;
            pthread_mutex_unlock(&mutex_ordenCargaGlobal);
            temporal_destroy(tlb->entradas[mayorIndice].ultimo_uso);
            tlb->entradas[mayorIndice].ultimo_uso = temporal_create();

        }else{
            log_error(cpu_logger, "Algoritmo de reemplazo no valido");
            exit(EXIT_FAILURE);
        }
    }

     mostrar_TLB();
}

void mostrar_TLB(){
    for(int i = 0; i < tlb->tamanio; i++){
        log_trace(cpu_logger, "PID: <%d> | PAGINA: <%d> | MARCO: <%d> ", tlb->entradas[i].pid, tlb->entradas[i].pagina, tlb->entradas[i].marco);
    }
}
