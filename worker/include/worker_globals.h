#ifndef WORKER_GLOBALS_H
#define WORKER_GLOBALS_H

#include <utils/networking.h>
#include <utils/set_instrucciones.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>

extern t_log *logger_worker;
extern t_config *config_worker;
extern char *IP_MASTER;
extern char *PUERTO_MASTER;
extern char *IP_STORAGE;
extern char *PUERTO_STORAGE;
extern char *PATH_SCRIPTS;
extern int TAM_MEMORIA;
extern int RETARDO_MEMORIA;
extern int ALGORITMO_REEMPLAZO;
extern void *memoria;
extern int tam_pagina;
extern t_list* tabla_paginas;
extern bool* bit_map_tabla_paginas;
extern int cantidad_paginas;

typedef struct {
    set_instrucciones copi;
    char *identificador;
    char **datos;
} t_instrucion;

typedef struct {
    char* puntero;
    bool modificado;
    char* identificador;
    double nro_pagina;
} nodo_pagina;


#endif