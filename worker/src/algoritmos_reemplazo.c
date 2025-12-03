#include "algoritmos_reemplazo.h"

// Private Functions //

int actualizar_pagina (nodo_pagina* pagina, char* identificador);
void check_file_tag (char* id_actual, char* id_nuevo);
void log_reemplazo (char* identificador1,  char* identificador2, int nro_pagina1, int nro_pagina2);
void mover_puntero_clock (int nro_marco);

// Public Functions //

int algoritmo_LRU(char* id_nueva_pagina, int nro_nueva_pagina) {    
    
    nodo_marco* victima = list_get(marco, 0);
    // Selecciona el marco que mas tiempo lleva sin usarse
    for (int i = 1; i < cantidad_marcos; i++) {
        nodo_marco* marco_i = list_get(marco, i);
        if (marco_i->time < victima->time) {
            victima = marco_i;
        }
    }

    nodo_pagina* pagina_victima = victima->pagina;
    if (pagina_victima->modificado) {
        notificar_cambios(pagina_victima, victima->identificador, victima->puntero_marco, storage_socket);
    }

    int nro_marco = actualizar_pagina(pagina_victima, victima->identificador);

    log_reemplazo(victima->identificador, id_nueva_pagina, pagina_victima->nro_pagina, nro_nueva_pagina);

    check_file_tag(victima->identificador, id_nueva_pagina);

    return nro_marco;
}

int algoritmo_CLOCK_M(char* id_nueva_pagina, int nro_nueva_pagina) {
    bool find = false;
    int nro_marco_siguiente;
    while (!find) {
        for (int i = 0; i < cantidad_marcos; i++) {
            if (!victima_clock->pagina->uso && !victima_clock->pagina->modificado) {
                find = true;
                break;
            }
            nro_marco_siguiente = victima_clock->pagina->nro_marco + 1;
            mover_puntero_clock(nro_marco_siguiente);
        }

        if (find) { break; }

        for (int i = 0; i < cantidad_marcos; i++) {
            if (!victima_clock->pagina->uso && victima_clock->pagina->modificado) {
                find = true;
                break;
            }
            victima_clock->pagina->uso = false;
            nro_marco_siguiente = victima_clock->pagina->nro_marco + 1;
            mover_puntero_clock(nro_marco_siguiente);
        }
    }
    
    nodo_pagina* pagina_victima = victima_clock->pagina;
    if (pagina_victima->modificado) {
        notificar_cambios(pagina_victima, victima_clock->identificador, victima_clock->puntero_marco, storage_socket);
    }

    int nro_marco = actualizar_pagina(pagina_victima, victima_clock->identificador);
    pagina_victima->uso = false;

    log_reemplazo(victima_clock->identificador, id_nueva_pagina, pagina_victima->nro_pagina, nro_nueva_pagina);
    
    check_file_tag(victima_clock->identificador, id_nueva_pagina);

    mover_puntero_clock(nro_marco + 1);

    return nro_marco;
}

void notificar_cambios(nodo_pagina* victima, char* identificador, void* p_marco, int fd_storage) {

    paquete_t* paquete = crear_paquete(MODIFICACIONES_STORAGE);
    agregar_file_tag(paquete, identificador);
    agregar_a_paquete(paquete, &victima->nro_pagina, sizeof(int));      // Nro Pagina
    agregar_a_paquete(paquete, p_marco, tam_pagina);                    // Contenido
    agregar_a_paquete(paquete, query_id, string_length(query_id) + 1);  // Query ID

    enviar_paquete(fd_storage, paquete, logger_worker);

    rta_storage result;
    if (recv(fd_storage, &result, sizeof(rta_storage), 0) <= 0) {
        log_error(logger_worker, "Error o desconeccion en Storage al recibir un resultado");
        exit(EXIT_FAILURE);
    }
    
    if (result != OP_EXITOSA) {
        log_error(logger_worker, "Error en Storage al realizar las modificaciones en %s (%d)", identificador, result);
        notif_query_error(result);
        //exit(EXIT_FAILURE);
    }
}

// Private Functions //

void check_file_tag (char* id_actual, char* id_nuevo) {
    file_tag* ft = file_tag_en_memoria(id_actual);
    ft->cantidad_paginas--;
    if (ft->cantidad_paginas == 0 && !string_equals_ignore_case(id_actual, id_nuevo)) {
        free_file_tag(ft);
    }
}

int actualizar_pagina (nodo_pagina* pagina, char* ft) {
    int nro_marco = pagina->nro_marco;
    bitarray_clean_bit(bit_map, nro_marco);
    pagina->presencia = false;
    pagina->modificado = false;
    char **datos = string_split(ft, ":");
    log_info(logger_worker, "Query %s: Se libera el Marco: %d perteneciente al - File: %s - Tag: %s", query_id, nro_marco, datos[0], datos[1]);
    string_array_destroy(datos);
    return nro_marco;
}

void mover_puntero_clock (int nro_marco) {
    if (nro_marco == cantidad_marcos) {
        victima_clock = list_get(marco, 0);
    } else {
        victima_clock = list_get(marco, nro_marco);
    }
}

void log_reemplazo (char* identificador1,  char* identificador2, int nro_pagina1, int nro_pagina2) {
    log_info(logger_worker, "## Query %s: Se reemplaza la página %s/%d por la %s/%d"
        , query_id, identificador1, nro_pagina1, identificador2, nro_pagina2);
}