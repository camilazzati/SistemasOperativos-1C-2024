#ifndef KERNEL_ENTRADASALIDA_H_
#define KERNEL_ENTRADASALIDA_H_

#include "../include/k_gestor.h"
#include "../include/shared_kernel.h"
#include "../include/control_procesos.h"
#include "../include/manejo_interfaces.h"

void esperar_entradasalida_kernel(int *fd_conexion_entradasalida);
void esperar_conexiones_entradasalida();
interfaz* _crear_instancia_interfaz(t_buffer* buffer, int* fd_conexion_entradasalida);
int solicitar_instruccion_a_interfaz(pcb* un_pcb, interfaz* una_interfaz);
void cargar_datos_auxiliares_en_paquete(instruccion_interfaz instruccion, pcb* un_pcb, t_paquete* un_paquete);
void limpiar_interfaz(int *fd_interfaz);
interfaz* _obtener_interfaz_con_nombre(char* nombre_interfaz);
void mostrar_instrucciones(interfaz* una_interfaz);

#endif /* KERNEL_ENTRADASALIDA_H_ */