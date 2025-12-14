#include "interpreter.h"

// Private Functions //

void log_ejecucion(int resultado, char* instruccion);
void enviar_paquete_a(paquete_t* paquete, int fd, char* modulo, char* instruccion);
void instruccion_simple_storage(t_instrucion* instruccion);
paquete_t* paquete_instruccion_storage(t_instrucion* instruccion);

// Public Functions //

void interpretar_CREATE(t_instrucion* instruccion) {
    
    instruccion_simple_storage(instruccion);
}

void interpretar_COMMIT(t_instrucion* instruccion) {
    
    t_instrucion* instruccion_flush = malloc(sizeof(t_instrucion));
    instruccion_flush->copi = FLUSH;
    instruccion_flush->datos = instruccion->datos;
    instruccion_flush->identificador = string_duplicate("FLUSH");
    interpretar_FLUSH(instruccion_flush);
    free(instruccion_flush->identificador);
    free(instruccion_flush);
    
    instruccion_simple_storage(instruccion);
}

void interpretar_DELETE(t_instrucion* instruccion) {

    for (int index_ft = 0; index_ft < list_size(file_tag_pages); index_ft++) {
        file_tag* ft = list_get(file_tag_pages, index_ft);

        if (string_equals_ignore_case(ft->identificador, instruccion->datos[0])) {
            // Libera los marcos que esta usando el FILE:TAG
            char **datos = string_split(instruccion->datos[0], ":");

            for (int i = 0; i < list_size(ft->tabla_paginas); i++) {

                if (ft->cantidad_paginas == 0) { break; }

                nodo_pagina* pagina = list_get(ft->tabla_paginas, i);

                if (pagina->presencia) {
                    bitarray_clean_bit(bit_map, pagina->nro_marco);
                    pagina->presencia = false;
                    ft->cantidad_paginas--;
                    log_info(logger_worker, "Query %s: Se libera el Marco: %d perteneciente al - File: %s - Tag: %s", query_id, pagina->nro_marco, datos[0], datos[1]);
                }
            }
            string_array_destroy(datos);
            // Elimina el FILE:TAG de la memoria interna
            list_remove(file_tag_pages, index_ft);
            free_file_tag(ft);
            break;
        }
    }
    
    instruccion_simple_storage(instruccion);
}

void interpretar_FLUSH(t_instrucion* instruccion) {

    // Busca el la Tabla de Paginas del FILE:TAG
    for (int i = 0; i < list_size(file_tag_pages); i++) {
        file_tag* ft = list_get(file_tag_pages, i);
        if (string_equals_ignore_case(ft->identificador, instruccion->datos[0])) {
            // Busca las paginas presentes y modificadas
            for (int j = 0; j < list_size(ft->tabla_paginas); j++) {
                nodo_pagina* pagina = list_get(ft->tabla_paginas, j);
                if (pagina->modificado && pagina->presencia) {
                    nodo_marco* n_marco = list_get(marco, pagina->nro_marco);
                    notificar_cambios(pagina, instruccion->datos[0], n_marco->puntero_marco, storage_socket);
                    pagina->modificado = false;
                }
            }
            break;
        }
    }
}

void interpretar_TRUNCATE(t_instrucion* instruccion) {

    int size = atoi(instruccion->datos[1]);
    int resto = size % tam_pagina;
    if (resto != 0)  {
        size += resto;
        log_warning(logger_worker, "El tamaño no es multiplo del tamaño del bloque, %s -> %d", instruccion->datos[1], size);
    }
    
    paquete_t* paquete = paquete_instruccion_storage(instruccion);
    agregar_a_paquete(paquete, &size, sizeof(int));                                  // TAMAÑO

    enviar_paquete_a(paquete, storage_socket, "Storage", instruccion->identificador);

    file_tag* ft = file_tag_en_memoria(instruccion->datos[0]);
    if (ceil(size/tam_pagina) != ft->cantidad_paginas) {
        actualizar_tabla(ft);
    }
}

void interpretar_TAG(t_instrucion* instruccion) {
    
    paquete_t* paquete = paquete_instruccion_storage(instruccion);
    agregar_file_tag(paquete, instruccion->datos[1]);

    enviar_paquete_a(paquete, storage_socket, "Storage", instruccion->identificador);
}

void interpretar_WRITE(t_instrucion* instruccion) {

    char* identificador = instruccion->datos[0];
    int base = atoi(instruccion->datos[1]);
    char* contenido = instruccion->datos[2];

    int nro_pagina = base / tam_pagina;
    
    nodo_pagina* pagina = pagina_en_Tabla(identificador, nro_pagina);
    if (!pagina) { 
        /*
         * Que no exista la pagina en tabla es equivalente a decir que se esta intentado acceder fuera de los limites del archivo, 
         * cuando busca la pagina en tabla si esta por fuera de los limites de la misma, retorna NULL
         */
        log_ejecucion(ERR_FUERA_LIMITE, instruccion->identificador);
        return;
    }

    escribir_pagina(pagina, identificador, base, contenido);
    log_ejecucion(OK, instruccion->identificador);
}

void interpretar_READ(t_instrucion* instruccion) {

    char* identificador = instruccion->datos[0];
    int base = atoi(instruccion->datos[1]);
    int size = atoi(instruccion->datos[2]);

    double nro_pagina = ceil(base / tam_pagina);
    nodo_pagina* pagina = pagina_en_Tabla(identificador, nro_pagina);
    if (!pagina) { 
        /*
         * Que no exista la pagina en tabla es equivalente a decir que se esta intentado acceder fuera de los limites del archivo, 
         * cuando busca la pagina en tabla si esta por fuera de los limites de la misma, retorna NULL
         */
        log_ejecucion(ERR_FUERA_LIMITE, instruccion->identificador);
        return;
    }

    leer_pagina(pagina, identificador, base, size);
    log_ejecucion(OK, instruccion->identificador);
}

void interpretar_END(t_instrucion* instruccion) {

    paquete_t* paquete = crear_paquete(INSTRUCCION_MASTER);
    agregar_a_paquete(paquete, &(instruccion->copi), sizeof(set_instrucciones));              // COPI 
    
    enviar_paquete_a(paquete, master_socket, "Master", instruccion->identificador);
}

// Private Functions //

void log_ejecucion(int resultado, char* instruccion) {
    switch (resultado) {
        case OK:
            log_info(logger_worker, "## Query %s: - Instrucción realizada: %s", query_id, instruccion);
            break;
        case ERROR:
        case ERR_INEXISTENCIA:
        case ERR_PREEXISTENCIA:
        case ERR_ESP_INSUFICIENTE:
        case ERR_WRITE_COMMITED:
        case ERR_FUERA_LIMITE:
        {
            log_error(logger_worker, "Error al ejecutar instruccion %s (%d)", instruccion, resultado);
            notif_query_error(resultado);
            break;
        }
        default:
            log_error(logger_worker, "No se reconoce el resultado al ejecutar instruccion %s (R = %d)", instruccion, resultado);
            exit(EXIT_FAILURE);
            break;
    }
}

void enviar_paquete_a(paquete_t* paquete, int fd, char* modulo, char* instruccion) {
    void* paquete_s = serializar_paquete(paquete);
    int result_send = send(fd, paquete_s, espacio_paquete_serializado(paquete), 0);
    destruir_paquete(paquete);
    free(paquete_s);
    if (result_send <= 0) {
        log_error(logger_worker, "Error o desconeccion en %s al enviar instruccion %s", modulo, instruccion);
        exit(EXIT_FAILURE);
    }

    int nro_resultado;
    if (string_equals_ignore_case(modulo, "Master")) {
        resultado_t resultado;
        if (recv(fd, &resultado, sizeof(resultado_t), MSG_WAITALL) <= 0) {
            log_error(logger_worker, "Error o desconeccion en %s al recibir resultado de instruccion %s", modulo, instruccion);
            exit(EXIT_FAILURE);
        }
        nro_resultado = (int)resultado;

    } else { // case "Storage"
        rta_storage resultado;
        if (recv(fd, &resultado, sizeof(rta_storage), MSG_WAITALL) <= 0) {
            log_error(logger_worker, "Error o desconeccion en %s al recibir resultado de instruccion %s", modulo, instruccion);
            exit(EXIT_FAILURE);
        }
        nro_resultado = (int)resultado;
    }
    
    log_ejecucion(nro_resultado, instruccion);
}

void instruccion_simple_storage(t_instrucion* instruccion) {
    paquete_t* paquete = paquete_instruccion_storage(instruccion);
    enviar_paquete_a(paquete, storage_socket, "Storage", instruccion->identificador);
}

paquete_t* paquete_instruccion_storage(t_instrucion* instruccion) {
    paquete_t* paquete = crear_paquete(INSTRUCCION_STORAGE);
    agregar_a_paquete(paquete, &(instruccion->copi), sizeof(set_instrucciones));     // COPI
    agregar_file_tag(paquete, instruccion->datos[0]);
    agregar_a_paquete(paquete, query_id, string_length(query_id) + 1);               // Query ID
    return paquete;
}