#include "interpreter.h"

// Private Functions //

void log_ejecucion(resultado_t resultado, char* instruccion);
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
                    bit_map_marco[pagina->nro_marco] = false;
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

    paquete_t* paquete = paquete_instruccion_storage(instruccion);

    // Busca el la Tabla de Paginas del FILE:TAG
    for (int i = 0; i < list_size(file_tag_pages); i++) {
        file_tag* ft = list_get(file_tag_pages, i);
        if (string_equals_ignore_case(ft->identificador, instruccion->datos[0])) {
            // Busca las paginas presentes y modificadas
            for (int j = 0; j < list_size(ft->tabla_paginas); j++) {
                nodo_pagina* pagina = list_get(ft->tabla_paginas, j);
                if (pagina->modificado && pagina->presencia) {
                    agregar_a_paquete(paquete, &(pagina->nro_pagina), sizeof(int));
                    char* info = list_get(marco, pagina->nro_marco);
                    agregar_a_paquete(paquete, info, tam_pagina);
                }
            }
            break;
        }
    }
    
    enviar_paquete_a(paquete, storage_socket, "Storage", instruccion->identificador);
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

    double nro_pagina = ceil(base / tam_pagina);
    nodo_pagina* pagina = pagina_en_Tabla(identificador, nro_pagina);
    
    escribir_pagina(pagina, identificador, base, contenido);
    log_ejecucion(OK, instruccion->identificador);
}

void interpretar_READ(t_instrucion* instruccion) {

    char* identificador = instruccion->datos[0];
    int base = atoi(instruccion->datos[1]);
    int size = atoi(instruccion->datos[2]);

    double nro_pagina = ceil(base / tam_pagina);
    nodo_pagina* pagina = pagina_en_Tabla(identificador, nro_pagina);

    leer_pagina(pagina, identificador, base, size);
    log_ejecucion(OK, instruccion->identificador);
}

void interpretar_END(t_instrucion* instruccion) {

    paquete_t* paquete = crear_paquete(INSTRUCCION_MASTER);
    agregar_a_paquete(paquete, &(instruccion->copi), sizeof(set_instrucciones));              // COPI 
    
    enviar_paquete_a(paquete, master_socket, "Master", instruccion->identificador);
}

// Private Functions //

void log_ejecucion(resultado_t resultado, char* instruccion) {
    switch (resultado) {
        case OK:
            log_info(logger_worker, "## Query %s: - Instrucción realizada: %s", query_id, instruccion);
            break;
        case ERROR:
            log_error(logger_worker, "Error al ejecutar instruccion %s", instruccion);
            exit(EXIT_FAILURE);
            break;
        default:
            log_error(logger_worker, "No se resonoce el resultado al ejecutar instruccion %s", instruccion);
            exit(EXIT_FAILURE);
            break;
    }
}

void enviar_paquete_a(paquete_t* paquete, int fd, char* modulo, char* instruccion) {
    void* paquete_s = serializar_paquete(paquete);
    if (send(fd, paquete_s, espacio_paquete_serializado(paquete), 0) <= 0) {
        log_error(logger_worker, "Error o desconeccion en %s al enviar instruccion %s", modulo, instruccion);
        destruir_paquete(paquete);
        free(paquete_s);
        exit(EXIT_FAILURE);
    }
    destruir_paquete(paquete);
    free(paquete_s);

    resultado_t resultado;
    if (recv(fd, &resultado, sizeof(resultado_t), MSG_WAITALL) <= 0) {
        log_error(logger_worker, "Error o desconeccion en %s al recibir resultado de instruccion %s", modulo, instruccion);
        exit(EXIT_FAILURE);
    }

    log_ejecucion(resultado, instruccion);
}

void instruccion_simple_storage(t_instrucion* instruccion) {
    paquete_t* paquete = paquete_instruccion_storage(instruccion);
    enviar_paquete_a(paquete, storage_socket, "Storage", instruccion->identificador);
}

paquete_t* paquete_instruccion_storage(t_instrucion* instruccion) {
    paquete_t* paquete = crear_paquete(INSTRUCCION_STORAGE);
    agregar_a_paquete(paquete, &(instruccion->copi), sizeof(set_instrucciones));     // COPI
    agregar_file_tag(paquete, instruccion->datos[0]);
    return paquete;
}