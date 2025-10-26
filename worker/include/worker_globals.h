#ifndef WORKER_GLOBALS_H
#define WORKER_GLOBALS_H

#include "utils/set_instrucciones.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/temporal.h>
#include <commons/collections/list.h>

// Structs //

typedef struct t_instrucion {
    set_instrucciones copi;
    char *identificador;
    char **datos;
} t_instrucion;

typedef struct nodo_pagina {
    int nro_pagina;
    int nro_marco;
    bool modificado;
    bool uso;
    bool presencia;
} nodo_pagina;

typedef struct file_tag {
    char *identificador;  // FILE:TAG
    int cantidad_paginas; // Debe ser mayor a 0
    t_list *tabla_paginas;//Lista de nodo_pagina
} file_tag; 

typedef struct nodo_marco {
    char *identificador; // Apunta a un FILE:TAG (definido en un file_tag)
    char *puntero_marco; // Direccion en memoria
    nodo_pagina *pagina; // Apunta a la pagina asociada
    int64_t time;        // Necesario para LRU
} nodo_marco;

// Logger //

extern t_log *logger_worker;

// Config //

extern t_config *config_worker;
extern char *ALGORITMO_REEMPLAZO;
extern char *IP_MASTER;
extern char *IP_STORAGE;
extern char *PATH_SCRIPTS;
extern char *PUERTO_MASTER;
extern char *PUERTO_STORAGE;
extern int TAM_MEMORIA;
extern int RETARDO_MEMORIA;

// Utils //

extern char *query_id;
extern t_list *instrucciones;
extern int master_socket;
extern int storage_socket;

// Memoria //

extern void *memoria;
extern bool *bit_map_marco;
extern t_list *marco;
extern t_list *file_tag_pages;
extern int tam_pagina;
extern int cantidad_marcos;

// Algoritmos //

extern int (*algoritmo_reemplazo)(char*, int);
extern t_temporal *cronometro;      // Para LRU
extern nodo_marco *victima_clock;   // Para CLOCK_M

#endif