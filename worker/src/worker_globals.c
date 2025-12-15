#include "worker_globals.h"

t_log *logger_worker;

t_config *config_worker;
char *ALGORITMO_REEMPLAZO;
char *IP_MASTER;
char *IP_STORAGE;
char *LOG_LEVEL;
char *PATH_SCRIPTS;
char *PUERTO_MASTER;
char *PUERTO_STORAGE;
int TAM_MEMORIA;
int RETARDO_MEMORIA;

char *query_id;
t_list *instrucciones;
int master_socket = -1;
int storage_socket = -1;

void *memoria;
void *bit_array;
t_bitarray *bit_map;
t_list *marco;
t_list *file_tag_pages;
int tam_pagina;
int cantidad_marcos;

int (*algoritmo_reemplazo)(char*, int);
t_temporal *cronometro;
nodo_marco *victima_clock;