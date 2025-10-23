#include <algoritmos_reemplazo.h>

// Private Functions //
void notificar_cambios (nodo_pagina* victima, char* identificador, void* p_marco, int fd_storage);
void check_file_tag (char* id_actual, char* id_nuevo);
int actualizar_pagina (nodo_pagina* pagina);
void mover_puntero_clock ();

// Public Functions //
// Si se ejecuta es porque todos los marcos estan opcupados por alguna pagina 
int algoritmo_LRU(int fd_storage, char* identificador_nueva_pagina) {    
    
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
        notificar_cambios(pagina_victima, victima->identificador, victima->puntero_marco, fd_storage);
    }

    int nro_marco = actualizar_pagina(pagina_victima);

    check_file_tag(victima->identificador, identificador_nueva_pagina);

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
            mover_puntero_clock();
        }

        if (find) { break; }

        for (int i = 0; i < cantidad_marcos; i++) {
            if (!victima_clock->pagina->uso && victima_clock->pagina->modificado) {
                find = true;
                break;
            }
            victima_clock->pagina->uso = false;
            mover_puntero_clock();
        }
    }
    
    nodo_pagina* pagina_victima = victima_clock->pagina;
    if (pagina_victima->modificado) {
        notificar_cambios(pagina_victima, victima_clock->identificador, victima_clock->puntero_marco, fd_storage);
    }

    int nro_marco = actualizar_pagina(pagina_victima);
    pagina_victima->uso = false;

    check_file_tag(victima_clock->identificador, identificador_nueva_pagina);

    mover_puntero_clock();

    return nro_marco;
}

// Private Functions //

void notificar_cambios(nodo_pagina* victima, char* identificador, void* p_marco, int fd_storage) {
    char **datos = string_split(identificador, ":");

    paquete_t* paquete = crear_paquete(MODIFICACIONES_STORAGE);
    agregar_a_paquete(paquete, datos[0], string_length(datos[0]) + 1);  // FILE
    agregar_a_paquete(paquete, datos[1], string_length(datos[1]) + 1);  // TAG
    agregar_a_paquete(paquete, &victima->nro_pagina, sizeof(int));      // Nro Pagina
    string_array_destroy(datos);
    agregar_a_paquete(paquete, p_marco, tam_pagina);                    // Contenido
    enviar_paquete(fd_storage, paquete, logger_worker);

    resultado_t result;
    if (recv(fd_storage, &result, sizeof(resultado_t), 0) <= 0) {
        log_error(logger_worker, "Error o desconeccion en Storage al recibir un resultado");
        exit(EXIT_FAILURE);
    }
    
    if (result == ERROR) {
        log_error(logger_worker, "Error en Storage al realizar las modificaciones en %s", identificador);
        exit(EXIT_FAILURE);
    }
}

void check_file_tag (char* id_actual, char* id_nuevo) {
    file_tag* ft = file_tag_en_memoria(id_actual);
    ft->cantidad_paginas--;
    if (ft->cantidad_paginas == 0 && !string_equals_ignore_case(id_actual, id_nuevo)) {
        free_file_tag(ft);
    }
}

int actualizar_pagina (nodo_pagina* pagina) {
    int nro_marco = pagina->nro_marco;
    bit_map_marco[nro_marco] = false;
    pagina->presencia = false;
    pagina->modificado = false;
    return nro_marco;
}

void mover_puntero_clock () {
    int nro_marco_siguiente = victima_clock->pagina->nro_marco + 1;
    if (nro_marco_siguiente == cantidad_marcos) {
        victima_clock = list_get(marco, 0);
    } else {
        victima_clock = list_get(marco, nro_marco_siguiente);
    }
}