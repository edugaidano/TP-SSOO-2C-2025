#include <interpreter.h>

void log_ejecucion(resultado_t resultado, char* instruccion, char* query_id) {
    switch (resultado) {
        case OK:
            log_info(logger_worker, "## Query %s: - Instrucción realizada: %s", query_id, instruccion);
            break;
        case ERROR:
            log_error(logger_worker, "Error al ejecutar instruccion %s", instruccion);
            exit(EXIT_FAILURE);
        default:
            log_error(logger_worker, "No se resonoce el resultado al ejecutar instruccion %s", instruccion);
            exit(EXIT_FAILURE);
            break;
    }
}

void enviar_paquete_a(void* paquete_serializado, int size_paquete, int fd, char* modulo, char* instruccion) {
    if (send(fd, paquete_serializado, size_paquete, 0) <= 0) {
        log_error(logger_worker, "Error o desconeccion en %s al enviar instruccion %s", modulo, instruccion);
        exit(EXIT_FAILURE);
    }
}

resultado_t resultado_instruccion(int fd, char* modulo, char* instruccion) {
    resultado_t resultado;
    if (recv(fd, &resultado, sizeof(resultado_t), MSG_WAITALL) <= 0) {
        log_error(logger_worker, "Error o desconeccion en %s al recibir resultado de instruccion %s", modulo, instruccion);
        exit(EXIT_FAILURE);
    }
    return resultado;
}

void instruccion_simple_storage(t_instrucion* instruccion, char* query_id, int fd_storage) {
    // Formato: <COPI> <NOMBRE_FILE>:<TAG>
    char **datos = string_split(instruccion->datos[0], ":");

    paquete_t* paquete = crear_paquete(INSTRUCCION_STORAGE);
    agregar_a_paquete(paquete, &(instruccion->copi), sizeof(set_instrucciones));        // COPI
    agregar_a_paquete(paquete, datos[0], string_length(datos[0]) + 1);                  // NOMBRE_FILE
    agregar_a_paquete(paquete, datos[1], string_length(datos[1]) + 1);                  // TAG
    string_array_destroy(datos);
    void* paquete_s = serializar_paquete(paquete);

    enviar_paquete_a(paquete_s, espacio_paquete_serializado(paquete), fd_storage, "Storage", instruccion->identificador);
    destruir_paquete(paquete);
    free(paquete_s);

    resultado_t resultado = resultado_instruccion(fd_storage, "Storage", instruccion->identificador);
    log_ejecucion(resultado, instruccion->identificador, query_id);
}

// ---- interpretar_instruccion ---- //

void interpretar_CREATE(t_instrucion* instruccion, char* query_id, int fd_storage) {
    /*
     * Formato: CREATE <NOMBRE_FILE>:<TAG>
     * La instrucción CREATE solicitará al módulo Storage la creación de un nuevo File con el Tag recibido por parámetro y con tamaño 0.
     */

    instruccion_simple_storage(instruccion, query_id, fd_storage);
}

void interpretar_COMMIT(t_instrucion* instruccion, char* query_id, int fd_storage) {
    /*
     * Formato: COMMIT <NOMBRE_FILE>:<TAG>
     * La instrucción COMMIT, le indicará al Storage que no se realizarán más cambios sobre el File y Tag pasados por parámetro.
     */
    t_instrucion* instruccion_flush = malloc(sizeof(t_instrucion));
    instruccion_flush->copi = FLUSH;
    instruccion_flush->datos = instruccion->datos;
    instruccion_flush->identificador = string_duplicate("FLUSH");
    interpretar_FLUSH(instruccion_flush, query_id, fd_storage);
    
    instruccion_simple_storage(instruccion, query_id, fd_storage);
}

void interpretar_DELETE(t_instrucion* instruccion, char* query_id, int fd_storage) {
    /*
     * Formato: DELETE <NOMBRE_FILE>:<TAG>
     * La instrucción DELETE solicitará al módulo Storage la eliminación del File:Tag correspondiente.
     */

    instruccion_simple_storage(instruccion, query_id, fd_storage);
}

void interpretar_FLUSH(t_instrucion* instruccion, char* query_id, int fd_storage) {
    /*
     * Formato: FLUSH <NOMBRE_FILE>:<TAG>
     * Persistirá todas las modificaciones realizadas en Memoria Interna de un File:Tag en el Storage.
     */
    char **datos = string_split(instruccion->datos[0], ":");

    paquete_t* paquete = crear_paquete(INSTRUCCION_STORAGE);
    agregar_a_paquete(paquete, &(instruccion->copi), sizeof(set_instrucciones));    // COPI
    agregar_a_paquete(paquete, datos[0], string_length(datos[0]) + 1);              // NOMBRE_FILE
    agregar_a_paquete(paquete, datos[1], string_length(datos[1]) + 1);              // TAG
    string_array_destroy(datos);
    for (int i = 0; i < list_size(tabla_paginas); i++) {
        nodo_pagina *pagina = list_get(tabla_paginas, i);
        if (pagina->modificado && string_equals_ignore_case(pagina->identificador, instruccion->datos[0])) {
            agregar_a_paquete(paquete, &(pagina->nro_pagina), sizeof(int));
            agregar_a_paquete(paquete, pagina->puntero, tam_pagina);
            reset_pagina(pagina, i, query_id);
        }
    }
    void* paquete_s = serializar_paquete(paquete);
    
    enviar_paquete_a(paquete_s, espacio_paquete_serializado(paquete), fd_storage, "Storage", instruccion->identificador);
    destruir_paquete(paquete);
    free(paquete_s);

    resultado_t resultado = resultado_instruccion(fd_storage, "Storage", instruccion->identificador);
    log_ejecucion(resultado, instruccion->identificador, query_id);    
}

void interpretar_TRUNCATE(t_instrucion* instruccion, char* query_id, int fd_storage) {
    /*
     * Formato: TRUNCATE <NOMBRE_FILE>:<TAG> <TAMAÑO>
     * La instrucción TRUNCATE solicitará al módulo Storage la modificación del tamaño del File y Tag indicados, 
     * asignando el tamaño recibido por parámetro (deberá ser múltiplo del tamaño de bloque).
     */
    char **datos = string_split(instruccion->datos[0], ":");
    int size = atoi(instruccion->datos[1]);
    int resto = size % tam_pagina;
    if (resto != 0)  {
        size += resto;
        log_warning(logger_worker, "El tamaño no es multiplo del tamaño del bloque, %s -> %d", instruccion->datos[1], size);
    }
    
    paquete_t* paquete = crear_paquete(INSTRUCCION_STORAGE);
    agregar_a_paquete(paquete, &(instruccion->copi), sizeof(set_instrucciones));     // COPI
    agregar_a_paquete(paquete, datos[0], string_length(datos[0]) + 1);               // NOMBRE_FILE
    agregar_a_paquete(paquete, datos[0], string_length(datos[1]) + 1);               // TAG
    string_array_destroy(datos);
    agregar_a_paquete(paquete, &size, sizeof(int));                                  // TAMAÑO
    void* paquete_s = serializar_paquete(paquete);

    enviar_paquete_a(paquete_s, espacio_paquete_serializado(paquete), fd_storage, "Storage", instruccion->identificador);
    destruir_paquete(paquete);
    free(paquete_s);

    resultado_t resultado = resultado_instruccion(fd_storage, "Storage", instruccion->identificador);
    log_ejecucion(resultado, instruccion->identificador, query_id);
}

void interpretar_TAG(t_instrucion* instruccion, char* query_id, int fd_storage) {
    /*
     * Formato: TAG <NOMBRE_FILE_ORIGEN>:<TAG_ORIGEN> <NOMBRE_FILE_DESTINO>:<TAG_DESTINO>
     * La instrucción TAG solicitará al módulo Storage la creación un nuevo File:Tag a partir del File y Tag origen pasados por parámetro.
     */
    char **datos0 = string_split(instruccion->datos[0], ":");
    char **datos1 = string_split(instruccion->datos[1], ":");

    paquete_t* paquete = crear_paquete(INSTRUCCION_STORAGE);
    agregar_a_paquete(paquete, &(instruccion->copi), sizeof(set_instrucciones));                // COPI
    agregar_a_paquete(paquete, datos0[0], string_length(datos0[0]));                             // NOMBRE_FILE_ORIGEN
    agregar_a_paquete(paquete, datos0[0], string_length(datos0[1]));                             // TAG_ORIGEN
    string_array_destroy(datos0);
    agregar_a_paquete(paquete, datos1[0], string_length(datos1[0]));                             // NOMBRE_FILE_DESTINO
    agregar_a_paquete(paquete, datos1[0], string_length(datos1[1]));                             // TAG_DESTINO
    string_array_destroy(datos1);
    void* paquete_s = serializar_paquete(paquete);

    enviar_paquete_a(paquete_s, espacio_paquete_serializado(paquete), fd_storage, "Storage", instruccion->identificador);
    destruir_paquete(paquete);
    free(paquete_s);

    resultado_t resultado = resultado_instruccion(fd_storage, "Storage", instruccion->identificador);
    log_ejecucion(resultado, instruccion->identificador, query_id);
}

void interpretar_WRITE(t_instrucion* instruccion, char* query_id, int fd_storage) {
    /*
     * Formato: WRITE <NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <CONTENIDO>
     * La instrucción WRITE escribirá en la Memoria Interna los bytes correspondientes a partir de la dirección base del File:Tag. 
     * En caso de que la Memoria Interna no cuente con todas las páginas necesarias para satisfacer la operación, 
     * deberá solicitar el contenido faltante al módulo Storage.
     */

    // Suponiendo que la BASE es 0 para cada FILE
    double nro_pagina = ceil(atoi(instruccion->datos[1]) / tam_pagina);
    nodo_pagina* pagina = pagina_en_Tabla(instruccion->datos[0], nro_pagina);
    if (pagina == NULL) {
        pagina = solicitar_pagina(instruccion->datos[0], nro_pagina, fd_storage, query_id);
    }
    
    escribir_pagina(pagina, atoi(instruccion->datos[1]), instruccion->datos[2], fd_storage,query_id);
    log_ejecucion(OK, instruccion->identificador, query_id);
}

void interpretar_READ(t_instrucion* instruccion, char* query_id, int fd_storage, int fd_master) {
    /*
     * Formato: READ <NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <TAMAÑO>
     * La instrucción READ leerá de la Memoria Interna los bytes correspondientes a partir de la dirección base del File y Tag 
     * pasados por parámetro, y deberá enviar dicha información al módulo Master.
     * En caso de que la Memoria Interna no cuente con todas las páginas necesarias para satisfacer la operación,
     * deberá solicitar el contenido faltante al módulo Storage.
     */

    // Suponiendo que la BASE es 0 para cada FILE
    double nro_pagina = ceil(atoi(instruccion->datos[1]) / tam_pagina);
    nodo_pagina* pagina = pagina_en_Tabla(instruccion->datos[0], nro_pagina);
    if (pagina == NULL) {
        pagina = solicitar_pagina(instruccion->datos[0], nro_pagina, fd_storage, query_id);
    }

    leer_pagina(pagina, atoi(instruccion->datos[1]), atoi(instruccion->datos[2]), fd_storage, fd_master, query_id);
    log_ejecucion(OK, instruccion->identificador, query_id);
}

void interpretar_END(t_instrucion* instruccion, char* query_id, int fd_master) {
    /*
     * Formato: END
     * Esta instrucción da por finalizada la Query y le informa al módulo Master el fin de la misma.
     */

    paquete_t* paquete = crear_paquete(INSTRUCCION_MASTER);
    agregar_a_paquete(paquete, &(instruccion->copi), sizeof(set_instrucciones)); 
    void* paquete_s = serializar_paquete(paquete);

    enviar_paquete_a(paquete_s, espacio_paquete_serializado(paquete), fd_master, "Master", instruccion->identificador);
    destruir_paquete(paquete);
    free(paquete_s);

    resultado_t resultado = resultado_instruccion(fd_master, "Master", instruccion->identificador);
    log_ejecucion(resultado, instruccion->identificador, query_id);
}
