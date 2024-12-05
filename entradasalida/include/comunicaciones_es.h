#ifndef COMUNICACIONES_ES_H_
#define COMUNICACIONES_ES_H_

# include "es_gestor.h"
# include "realizarop_es.h"

void enviar_handshake_prueba();

void esperar_memoria_es();
void esperar_kernel_es();

void esperar_memoria_es_stdout();

void esperar_memoria_es_stdin();
void esperar_memoria_es_dialfs();



#endif