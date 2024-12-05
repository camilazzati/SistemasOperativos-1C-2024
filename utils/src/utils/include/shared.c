#include "shared.h"

/////////////////////////////// CONEXIONES  ///////////////////////////////
// LÓGICA CONEXIÓN SERVIDOR - CLIENTE 

int crear_conexion(char *ip, char* puerto){
	struct addrinfo hints; 
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int err = getaddrinfo(ip, puerto, &hints, &server_info);

	if (err != 0){
		printf("Error al crear conexion");
		exit(-2);
	}

	int socket_cliente = socket(server_info->ai_family,
		server_info->ai_socktype,
		server_info->ai_protocol);
		
	if(setsockopt(socket_cliente,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int))<0){
		perror("setsockopt(SO_REUSEADDR) failed");
	}

	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen); //conecta socket

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_servidor(char* puerto,t_log* logger,char* mensaje) //Crea socket servidor, lo bindea y lo deja escuchando
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo); //De acá saca el puerto a donde se bindea el socket

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(
		servinfo->ai_family,
        servinfo->ai_socktype,
        servinfo->ai_protocol);

	if(setsockopt(socket_servidor,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int))<0){
		perror("setsockopt(SO_REUSEADDR) failed");
	}

	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	// Escuchamos las conexiones entrantes
	listen(socket_servidor,SOMAXCONN);

	freeaddrinfo(servinfo);
	log_trace(logger, "SERVER: %s",mensaje);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor,t_log* logger,char* mensaje) 
{	
	// Aceptamos un nuevo cliente
	int socket_cliente;
	
	log_info(logger, "Esperando a %s", mensaje);

	socket_cliente = accept(socket_servidor,NULL,NULL); //qieda esperando hasta que se conecte cliente

	log_info(logger, "!! Se conecto %s !!", mensaje);

	return socket_cliente;
}

/////////////////////////////// COMUNICACIÓN  ///////////////////////////////

// CADA UNO SE FIJA COMO LO EXTRAE, YA QUE DEPENDERÁ DEL CODIGO DE OPERACIÓN


// BUFFER 				|-> STREAM Y SIZE
// PAQUETE 				|-> CODIGO DE OPERACION Y BUFFER (STREAM Y SIZE)

// ======== Serializar:

// PAQUETE SERIALZIADO 	|-> CODIGO DE OPERACION - SIZE - STREAM(TAM - MSJE - TAM2 - MESAJE2)


// COMUNICACIÓN PRUEBA

void recibir_mensaje_tp0(int socket_cliente,t_log* logger)
{
	int size;
	char* buffer = recibir_buffer_tp0(&size, socket_cliente); // PARA QUE SE USA EL SIZE? No sería mejor declararlo en recibir_buffer??
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}


void enviar_mensaje(char* mensaje, int socket_cliente) //Esta función manda string como mensaje
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;  //Acá se le da la info al while de que tiene que hacer
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

// ========== LÓGICA DE CREACIÓN DE MENSAJES


t_buffer* crear_buffer() { // CREA BUFFER PARA QUE LLEVE DISTINTOS MENSAJES
	t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = 0;
	buffer->stream = NULL;
	return buffer;
}

t_paquete* crear_paquete_con_buffer(op_code codigo_operacion){ // EL PAQUETE ADEMAS DE LLEVAR INFORMACION (COMO EL BUFFER) LLEVA EL CODIGO DE OPERACION
	t_paquete* nuevo_paquete = malloc(sizeof(t_paquete));
	
	// Agrego código de operacion
	nuevo_paquete -> codigo_operacion = codigo_operacion;
	
	// Creo buffer en paquete
	crear_buffer_en_paquete(nuevo_paquete);
	
	return nuevo_paquete;
}

void crear_buffer_en_paquete(t_paquete* paquete){
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void cargar_mensaje_a_buffer(t_buffer* buffer, void* mensaje, int tam_mensaje) { // AGREGA EL TAM. DE MSJE + MSJE UN BUFFER EXISTENTE (Puede ya tener algo cargado o no)
	
	if(buffer -> size == 0){ // EValúo si está vacío el buffer
		
		// Reservo memoria para el tam. y el msje
		buffer -> stream = malloc(sizeof(int) + tam_mensaje); 
		
		// Copio tamaño de mensaje 
		memcpy(buffer -> stream,&tam_mensaje,sizeof(int));
		
		// Copio el mensaje
		memcpy(buffer -> stream + sizeof(int),mensaje,tam_mensaje);

		// Stream -> SIZE + MSJE
	}
	else {

		// Aumenta tamaño de memoria de stream en tiempo de ejecución
   		buffer->stream = realloc(buffer->stream, buffer->size + sizeof(int) + tam_mensaje); // realloc() permite aumentar en tiempo de ejecución el tamaño de memoria dinámica ya pedido
    	
		// Agrego datos a partir de donde termina ultimo msje
		memcpy(buffer->stream + buffer->size, &tam_mensaje, sizeof(int));
    	memcpy(buffer->stream + buffer->size + sizeof(int), mensaje, tam_mensaje);

		// Stream -> SIZE + MSJE + SIZE_NUEVO + MSJE_NUEVO ...
	}

	// Actualizo tamaño del buffer
    buffer->size += sizeof(int);
    buffer->size += tam_mensaje;
}

void cargar_int_a_buffer(t_buffer* buffer, int valor) {  
    cargar_mensaje_a_buffer(buffer, &valor, sizeof(int));
}

void cargar_uint32_a_buffer(t_buffer* buffer, uint32_t valor) {
    cargar_mensaje_a_buffer(buffer, &valor, sizeof(uint32_t));
}

void cargar_uint8_a_buffer(t_buffer* buffer, uint8_t valor){
	cargar_mensaje_a_buffer(buffer, &valor, sizeof(uint8_t));
}

void cargar_string_a_buffer(t_buffer* buffer, char* string) {
    cargar_mensaje_a_buffer(buffer, string, strlen(string)+1);
}

void cargar_int_a_paquete(t_paquete* paquete, int valor) {  
    cargar_mensaje_a_buffer(paquete -> buffer, &valor, sizeof(int));
}

void cargar_uint32_a_paquete(t_paquete* paquete, uint32_t valor) {
    cargar_mensaje_a_buffer(paquete->buffer, &valor, sizeof(uint32_t));
}

void cargar_uint8_a_paquete(t_paquete* paquete, uint8_t valor){
	cargar_mensaje_a_buffer(paquete->buffer, &valor, sizeof(uint8_t));
}

void cargar_string_a_paquete(t_paquete* paquete, char* string) {
    cargar_mensaje_a_buffer(paquete -> buffer, string, strlen(string)+1);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void enviar_paquete(t_paquete* paquete, int fd){ // RECIBE PAQUETE Y FILE DESCRIPTOR DE CONEXION
	
	void* a_enviar = serializar_paquete(paquete);

	int bytes = paquete->buffer->size + 2*sizeof(int); // LE AGREGA *2 int POR LOS QUE LE AGREGO AL SERIALZIAR
	send(fd, a_enviar, bytes, 0);

	free(a_enviar);
}

void* serializar_paquete(t_paquete* paquete){
	
	int tam_puntero = paquete->buffer->size + 2*sizeof(int); // POR LOS 2 sizeof(int) QUE AGREGO MÁS ABAJO
	
	void* puntero = malloc(tam_puntero);
	int desplazamiento = 0;

	// Copia en destino (puntero) lo que hay en source (paquete)
	memcpy(puntero+desplazamiento,&(paquete->codigo_operacion),sizeof(int)); // COPIO EL CODIGO DE OPERACION
	desplazamiento += sizeof(int);
	memcpy(puntero+desplazamiento,&(paquete->buffer->size),sizeof(int)); // COPIO EL TAMAÑO DEL "BUFFER"
	desplazamiento += sizeof(int);
	memcpy(puntero+desplazamiento,paquete->buffer->stream,paquete->buffer->size); // COPIO EL STREAM

	return puntero; 
	// puntero -> COD_OP + TAM. + STREAM 
}

// ========== LÓGICA DE EXTRACCIÓN DE MENSAJES


int recibir_operacion(int socket_cliente) // DEVUELVE SOLAMENTE EL CODIGO DE OPERACION
{
	int cod_op;

	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

// TAM. + STREAM 

void* recibir_buffer_tp0(int* size, int socket_cliente) // RECIBE TODO EL STREAM DEL PAQUETE SERIALIZADO
{
	void * buffer;

	// Recibo TAM. y lo guardo en size (nos sirve para después)
	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	
	// Reservo memoria para el puntero del tamaño TAM.
	buffer = malloc(*size);

	// Recibo STREAM en buffer
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer; 
}

// STREAM

t_buffer* recibir_buffer(int socket_cliente) // BUFFERIALIZO
{
	int size;
	void* buffer;
	t_buffer* nuevo_buffer = crear_buffer();
	
	// Recibo todo el stream
	buffer = recibir_buffer_tp0(&size, socket_cliente);
	
	// Guardo el tamaño del buffer total
	nuevo_buffer->size = size;

	// Agrego todo el stream al stream del buffer
	nuevo_buffer->stream = buffer;
	// Devuelvo el nuevo buffer
	return nuevo_buffer;
}

void* extraer_mensaje_de_buffer(t_buffer* buffer){ //EXTRAE BUFFER DE A PARTES
	
	// Evalúo posibles errores
	if(buffer -> size == 0){
		printf("ERROR: Se intento extraer_mensaje_de_buffer de buffer vacío");
		exit(EXIT_FAILURE);
	}

	if(buffer -> size < 0){
		printf("ERROR: Se intento extraer_mensaje_de_buffer pero el tamaño del mensaje es negativo");
		exit(EXIT_FAILURE);
	}

	int tam_mensaje;

	// Saco el tamaño de la próxima porción del stream a sacar
	memcpy(&tam_mensaje,buffer->stream,sizeof(int));
	// En tam_mensaje tengo el tamaño de lo que voy a sacar después 

	// Reservo memoria con el tamaño de la porción a sacar
	void* mensaje = malloc(tam_mensaje);

	// Copio em mensaje lo que hay en el stream en la posición 0 + sizeof(int), del tamaño que guardé antes
	memcpy(mensaje,buffer->stream + sizeof(int),tam_mensaje);

	// Guardo el tamaño nuevo (sizeof(int) + tamaño del mensaje que saqué)
	int tam_nuevo = buffer->size - sizeof(int) - tam_mensaje;
	
	if (tam_nuevo == 0){
		buffer->size = tam_nuevo;
		return mensaje;
	}

	if (tam_nuevo < 0){
		printf("ERROR: Buffer de tamaño negativo");
		exit(EXIT_FAILURE);
	}

	// Creo puntero con el nuevo tamaño del STREAM
	void* nuevo_stream = malloc(tam_nuevo);

	// Copio el STREAM viejo sin lo que ya saqué
	memcpy(nuevo_stream,buffer->stream+sizeof(int)+tam_mensaje,tam_nuevo);
	
	// TAM + ALGO + TAM_ALGONUEVO + ALGO_NUEVO

	// Le reasigno el nuevo STREAM al buffer y actualizo el tamaño
	free(buffer->stream);
	buffer->size = tam_nuevo;
	buffer->stream = nuevo_stream;

	// Devuelvo mensaje a través de puntero de tipo void*
	return mensaje;
}

int extraer_int_del_buffer(t_buffer* buffer){ 

	// Creo puntero del tipo de dato que espero
	int* un_entero = extraer_mensaje_de_buffer(buffer);

	// Se lo asigno a la variable 
	int valor_del_int = *un_entero;

	free(un_entero);

	// Retorno valor
	return valor_del_int; 
}

char* extraer_string_del_buffer(t_buffer* buffer){
	char* string = extraer_mensaje_de_buffer(buffer);
	return string;
}

uint32_t extraer_uint32_del_buffer(t_buffer* buffer){
	uint32_t* un_entero = extraer_mensaje_de_buffer(buffer);
	uint32_t valor_del_uint32_t = *un_entero;
	free(un_entero);
	return valor_del_uint32_t; 
}

uint8_t extraer_uint8_del_buffer(t_buffer* buffer){
	uint8_t* un_entero = extraer_mensaje_de_buffer(buffer);
	uint8_t valor_del_uint8_t = *un_entero;
	free(un_entero);
	return valor_del_uint8_t; 
}

int32_t extraer_int32_del_buffer(t_buffer* buffer){
	int32_t* un_entero = extraer_mensaje_de_buffer(buffer);
	int32_t valor_del_int32_t = *un_entero;
	free(un_entero);
	return valor_del_int32_t; 
}

// DESTRUIR BUFFER

void destruir_buffer(t_buffer* buffer) { // DESTRUYE BUFFER PARA QUE NO HAYA !!!!MEMORY LEAKS!!!!
    if (buffer->stream != NULL) {
        free(buffer->stream);
        buffer->stream = NULL;
    }
    free(buffer);
}

// DESTRUIR PAQUETE CON SU BUFFER:

void destruir_paquete(t_paquete* paquete){
	destruir_buffer(paquete->buffer);
	free(paquete);
}

// FUNCIONES THREADS

void ejecutar_en_hilo_detach(void (*una_funcion)(void*) ,void* struct_argumento){
	pthread_t thread;
	pthread_create(&thread, NULL, (void*)una_funcion, struct_argumento);
	pthread_detach(thread);
}

void ejecutar_en_un_hilo_nuevo_join(void (*f)(void*) ,void* struct_arg){
	pthread_t thread;
	pthread_create(&thread, NULL, (void*)f, struct_arg);
	pthread_join(thread, NULL);
}


t_buffer* recibir_un_paquete(int conexion){
	t_buffer* unBuffer = malloc(sizeof(t_buffer));
	int size;
	unBuffer->stream =  recibir_buffer_tp0(&size, conexion);
	unBuffer->size = size;
	return unBuffer;
}


t_list* recibir_paquete(int socket_cliente)
{
	int posición = 0;
	int size;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer_tp0(&size, socket_cliente);

	while(posición < size)
	{
		memcpy(&tamanio, buffer + posición, sizeof(int));
		posición+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+posición, tamanio);
		posición+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

void int_to_string(int number, char* buffer) {
    int is_negative = 0;
    int i = 0, j, len;
    char temp;

    // Manejar el caso del número negativo
    if (number < 0) {
        is_negative = 1;
        number = -number;
    }

    // Convertir cada dígito del número a carácter
    do {
        buffer[i++] = (number % 10) + '0';
        number /= 10;
    } while (number > 0);

    // Añadir el signo negativo si es necesario
    if (is_negative) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0'; // Añadir el carácter nulo al final

    // Invertir la cadena resultante
    len = i;
    for (j = 0; j < len / 2; j++) {
        temp = buffer[j];
        buffer[j] = buffer[len - j - 1];
        buffer[len - j - 1] = temp;
    }
}