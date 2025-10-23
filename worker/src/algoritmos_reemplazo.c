#include <algoritmos_reemplazo.h>

// Si se ejecuta es porque todos los marcos estan opcupados por alguna pagina 
int algoritmo_LRU(int fd_storage, char* identificador_nueva_pagina) {    
    
    nodo_marco* victima = list_get(marco, 0);
    for (int i = 1; i < cantidad_marcos; i++) {
        nodo_marco* marco_i = list_get(marco, i);
        if (marco_i->time < victima->time) { // Selecciona el marco que mas tiempo lleva sin usarse
            victima = marco_i;
        }
    }
    nodo_pagina* pagina_victima = victima->pagina;
    if (pagina_victima->modificado) {
        char **datos = string_split(victima->identificador, ":");

        paquete_t* paquete = crear_paquete(MODIFICACIONES_STORAGE);
        agregar_a_paquete(paquete, datos[0], string_length(datos[0]) + 1);  // FILE
        agregar_a_paquete(paquete, datos[1], string_length(datos[1]) + 1);  // TAG
        agregar_a_paquete(paquete, pagina_victima->nro_pagina, sizeof(int));// Nro Pagina
        string_array_destroy(datos);
        agregar_a_paquete(paquete, victima->puntero_marco, tam_pagina);     // Contenido
        enviar_paquete(fd_storage, paquete, logger_worker);

        resultado_t result;
        if (recv(fd_storage, &result, sizeof(resultado_t), 0) <= 0) {
            log_error(logger_worker, "Error o desconeccion en Storage al recibir un resultado");
            exit(EXIT_FAILURE);
        }
        
        if (result == ERROR) {
            log_error(logger_worker, "Error en Storage al realizar las modificaciones en %s", victima->identificador);
            exit(EXIT_FAILURE);
        }
    }
    int nro_marco = pagina_victima->nro_marco;
    bit_map_marco[nro_marco] = false;
    pagina_victima->presencia = false;
    pagina_victima->modificado = false;

    file_tag* ft = file_tag_en_memoria(victima->identificador);
    ft->cantidad_paginas--;
    // Si el file_tag no tiene paginas en memoria y no se utilizara en este momento, entonces se libera
    if (ft->cantidad_paginas == 0 && !string_equals_ignore_case(victima->identificador, identificador_nueva_pagina)) {
        free_file_tag(ft);
    }

    return nro_marco;
}

int algoritmo_CLOCK_M(int fd_storage, char* identificador_nueva_pagina) {
    bool find = false;
    while (!find) {
        for (int i = 0; i < cantidad_marcos; i++) {
            if (!victima_clock->pagina->uso && !victima_clock->pagina->modificado) {
                find = true;
                break;
            }
            int nro_marco_siguiente = victima_clock->pagina->nro_marco + 1;
            if (nro_marco_siguiente == cantidad_marcos) {
                victima_clock = list_get(marco, 0);
            } else {
                victima_clock = list_get(marco, nro_marco_siguiente);
            }
        }

        if (find) { break; }

        for (int i = 0; i < cantidad_marcos; i++) {
            if (!victima_clock->pagina->uso && victima_clock->pagina->modificado) {
                find = true;
                break;
            }
            victima_clock->pagina->uso = false;
            int nro_marco_siguiente = victima_clock->pagina->nro_marco + 1;
            if (nro_marco_siguiente == cantidad_marcos) {
                victima_clock = list_get(marco, 0);
            } else {
                victima_clock = list_get(marco, nro_marco_siguiente);
            }
        }
    }
    
    if (victima_clock->pagina->modificado) {
        char **datos = string_split(victima_clock->identificador, ":");

        paquete_t* paquete = crear_paquete(MODIFICACIONES_STORAGE);
        agregar_a_paquete(paquete, datos[0], string_length(datos[0]) + 1);   // FILE
        agregar_a_paquete(paquete, datos[1], string_length(datos[1]) + 1);   // TAG
        string_array_destroy(datos);        
        agregar_a_paquete(paquete, &victima_clock->pagina->nro_pagina, sizeof(int));// Nro Pagina
        agregar_a_paquete(paquete, victima_clock->puntero_marco, tam_pagina);// Contenido
        enviar_paquete(fd_storage, paquete, logger_worker);

        resultado_t result;
        if (recv(fd_storage, &result, sizeof(resultado_t), 0) <= 0) {
            log_error(logger_worker, "Error o desconeccion en Storage al recibir un resultado");
            exit(EXIT_FAILURE);
        }
        
        if (result == ERROR) {
            log_error(logger_worker, "Error en Storage al realizar las modificaciones en %s", victima_clock->identificador);
            exit(EXIT_FAILURE);
        }
    }

    int nro_marco = victima_clock->pagina->nro_marco;
    bit_map_marco[nro_marco] = false;
    victima_clock->pagina->presencia = false;
    victima_clock->pagina->modificado = false;
    victima_clock->pagina->uso = false;

    file_tag* ft = file_tag_en_memoria(victima_clock->identificador);
    ft->cantidad_paginas--;
    // Si el file_tag no tiene paginas en memoria y no se utilizara en este momento, entonces se libera
    if (ft->cantidad_paginas == 0 && !string_equals_ignore_case(victima_clock->identificador, identificador_nueva_pagina)) {
        free_file_tag(ft);
    }

    int nro_marco_siguiente = victima_clock->pagina->nro_marco + 1;
    if (nro_marco_siguiente == cantidad_marcos) {
        victima_clock = list_get(marco, 0);
    } else {
        victima_clock = list_get(marco, nro_marco_siguiente);
    }

    return nro_marco;
}