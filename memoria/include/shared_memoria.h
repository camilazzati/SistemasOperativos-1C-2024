#ifndef SHARED_MEMORIA_H_
#define SHARED_MEMORIA_H_

#include "m_gestor.h"
#include "operaciones_proceso.h"
#include "../include/espacio_usuario.h"



// FUNCIONES

void retardo_respuesta();
char* resolver_solicitud_leer_bloque(t_buffer* unBuffer);
char* resolver_solicitud_escribir_bloque(t_buffer* unBuffer);


#endif