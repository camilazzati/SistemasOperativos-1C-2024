#ifndef PAGINACION_H_
#define PAGINACION_H_

#include "m_gestor.h"
#include "../include/espacio_usuario.h"

//FUNCIONES QUE QUIERO COMPARTIR 

void inicializar_tabla_de_paginas(t_proceso* nuevo_proceso);
t_marco* crear_frame(int tamBase, bool disponible, int numero_marco);
void liberar_frame(t_marco* un_marco);
t_marco* buscar_marco_segun_numero(int num_marco);
t_marco* obtener_frame_disponible();
int devolver_numero_de_marco(t_proceso* proceso,int num_pagina);
void asignar_marco_disponible_a_proceso_vacio(t_proceso* proceso);
void agregar_marco_pagina_a_tabla(t_proceso* proceso, int num_pagina, int nro_marco);
void inicializar_marco_para_un_proceso(t_marco* un_marco,t_proceso* proceso, int num_pagina);
int cantidad_paginas_necesarias(int tamanio);
void poner_en_disponible_frame(t_marco* un_marco);

bool obtener_frame_disponible_simulacion(t_proceso* proceso);


#endif