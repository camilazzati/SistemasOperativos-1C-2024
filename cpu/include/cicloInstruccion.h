#ifndef CICLOINSTRUCCION_CPU_H
#define CICLOINSTRUCCION_CPU_H

#include "c_gestor.h"

#include <sys/time.h>
#include <stdint.h>

void realizarCicloInstruccion();
void ciclo_de_instruccion_fetch();
void decodeYExecute();

uint32_t* detectar_registro(char*);
void enviarContextoAKernel(t_paquete*);
void enviarContextoAKernel_sinTiempo(t_paquete*);

bool hayQueDesalojar;
bool hay_interrupcion_quantum;
bool hay_interrupcion_exit;

struct timeval inicio, fin;

char* leer_valor_memoria(int, int);
void escribir_valor_memoria(int, char*, int);
int traducir(int);
int traducir_sin_TLB(int, int, int);
int mmu(int, int, int);
int buscar_en_TLB(int);
void agregar_entrada_TLB(int, int);
void mostrar_TLB();

char** opcode_headers;

#endif
