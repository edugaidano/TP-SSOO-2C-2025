#include <memoria.h>

void init_memoria() {
    // malloc memoria
    memoria = malloc(TAM_MEMORIA);

    // Tabla de paginas
    cantidad_paginas = TAM_MEMORIA/tam_pagina;
    tabla_paginas = list_create();
    for (int i = 0; i < cantidad_paginas; i++) {
        nodo_pagina* pagina = malloc(sizeof(nodo_pagina));
        pagina->puntero = (char*)memoria + i * tam_pagina;
        pagina->modificado = false;
        pagina->nro_pagina = -1;
        pagina->identificador = string_new(); 
        list_add(tabla_paginas, pagina);
    }
    
    // bitmap
    bit_map_tabla_paginas = malloc( sizeof(bool) * cantidad_paginas);
    for (int i = 0; i < cantidad_paginas; i++) {
        bit_map_tabla_paginas[i] = false;
    }

    log_info(logger_worker, "Memoria iniciada");
}

int pagina_libre() {
    for (int i = 0; i < cantidad_paginas; i++) {
        if (!bit_map_tabla_paginas[i]) {
            return i;
        }
    }
    return -1;
}

nodo_pagina* pagina_en_Tabla(char* identificador, double nro_pagina) {
    for (int i = 0; i < cantidad_paginas; i++) {
        nodo_pagina* pagina = list_get(tabla_paginas, i);
        if (string_equals_ignore_case(pagina->identificador, identificador) && pagina->nro_pagina == nro_pagina) {
            return pagina;
        }
    }
    return NULL;
}

void reset_pagina(nodo_pagina* pagina, int i, char* query_id) {
    bit_map_tabla_paginas[i] = false;
    pagina->modificado = false;
    pagina->nro_pagina = -1;
    char** datos = string_split(pagina->identificador, ":");
    free(pagina->identificador);
    pagina->identificador = string_new();
    log_info(logger_worker, "Query %s: Se libera el Marco: %d perteneciente al - File: %s - Tag: %s", query_id, i, datos[0], datos[1]);
    string_array_destroy(datos);
}

nodo_pagina* solicitar_pagina(char* identificador, int nro_pagina, int fd_storage, char* query_id) {
    char **datos = string_split(identificador, ":");
    log_info(logger_worker, "Query %s: - Memoria Miss - File: %s - Tag: %s - Pagina: %d", query_id, datos[0], datos[1], nro_pagina);

    paquete_t* paquete = crear_paquete(SOLICITUD_STORAGE);
    agregar_a_paquete(paquete, datos[0], string_length(datos[0]) + 1);              // NOMBRE_FILE
    agregar_a_paquete(paquete, datos[1], string_length(datos[1]) + 1);              // TAG
    agregar_a_paquete(paquete, &nro_pagina, sizeof(int));
    void* paquete_s = serializar_paquete(paquete);

    if (send(fd_storage, paquete_s, espacio_paquete_serializado(paquete), 0) <= 0) {
        log_error(logger_worker, "Error o desconeccion en Storage al enviar resultado de solicitar pagina");
        exit(EXIT_FAILURE);
    }
    destruir_paquete(paquete);
    free(paquete_s);

    paquete = recibir_paquete(fd_storage, logger_worker); // Paquete: Informacion de la Pagina, Nro pagina
    if (paquete->codigo_operacion != PAGINA_WORKER) {
        log_error(logger_worker, "Se recibio un paquete desconcocido al solicitar pagina");
        exit(EXIT_FAILURE);
    }
    buffer_t* buffer = obtener_siguiente_item(paquete);

    int indice = pagina_libre();
    if (indice == -1) {
        // indice = liverar_pagina(); // TODO
    }
    log_info(logger_worker, "Query %s: Se asigna el Marco: %i a la Página: %i perteneciente al - File: %s - Tag: %s", query_id, indice, nro_pagina, datos[0], datos[1]);
    bit_map_tabla_paginas[indice] = true;
    nodo_pagina* pagina = list_get(tabla_paginas, indice);
    pagina->identificador = string_duplicate(identificador);
    memcpy(pagina->puntero, buffer->stream, buffer->size);
    buffer = obtener_siguiente_item(paquete);
    memcpy(&(pagina->nro_pagina), buffer->stream, buffer->size);
    
    destruir_paquete(paquete);
    free(buffer);

    log_info(logger_worker, "Query %s: - Memoria Add - File: %s - Tag: %s - Pagina: %d", query_id, datos[0], datos[1], nro_pagina);
    string_array_destroy(datos);
    return pagina;
}

void escribir_pagina(nodo_pagina* pagina, int direccion, char* datos, int fd_storage, char* query_id) {
    int direccion_en_pagina = direccion % tam_pagina;
    int size_dato = string_length(datos) + 1;

    char* puntero;
    pagina->modificado = true;
    if (size_dato <= tam_pagina - direccion_en_pagina) { // si el dato entra en la pagina actual
        puntero = pagina->puntero + direccion_en_pagina;
        memcpy(puntero, datos, size_dato);
    } else {
        int size_restante = size_dato - direccion_en_pagina;
        puntero = pagina->puntero + direccion_en_pagina;
        memcpy(puntero, datos, size_dato - size_restante);

        nodo_pagina* pagina_extra = pagina_en_Tabla(pagina->identificador, pagina->nro_pagina + 1);
        if (pagina_extra == NULL) {
            pagina_extra = solicitar_pagina(pagina->identificador, pagina->nro_pagina + 1, fd_storage, query_id);
        }
        pagina_extra->modificado = true;
        puntero = pagina_extra->puntero; // Como es una pagina nueva, se escrvira desde la base de la misma
        memcpy(puntero, datos + (size_dato - size_restante), size_restante);
    }

    log_info(logger_worker, "Query %s: Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", query_id, direccion, datos);
}

void leer_pagina(nodo_pagina* pagina, int direccion, int size, int fd_storage, int fd_master, char* query_id) {
    int direccion_en_pagina = direccion % tam_pagina;

    char* puntero_memoria;
    char* lectura = malloc(size);
    if (size <= tam_pagina - direccion_en_pagina) { // si el dato entra en la pagina actual
        puntero_memoria = pagina->puntero + direccion_en_pagina;
        memcpy(lectura, puntero_memoria, size);
    } else {
        int size_restante = size - direccion_en_pagina;
        puntero_memoria = pagina->puntero + direccion_en_pagina;
        memcpy(lectura, puntero_memoria, size - size_restante);

        nodo_pagina* pagina_extra = pagina_en_Tabla(pagina->identificador, pagina->nro_pagina + 1);
        if (pagina_extra == NULL) {
            pagina_extra = solicitar_pagina(pagina->identificador, pagina->nro_pagina + 1, fd_storage, query_id);
        }
        puntero_memoria = pagina_extra->puntero; // Como es una pagina nueva, se escrvira desde la base de la misma
        memcpy(lectura + (size - size_restante), puntero_memoria, size_restante);
    }
    paquete_t* paquete = crear_paquete(LECTURA_MASTER);
    agregar_a_paquete(paquete, lectura, size);
    void* paquete_s = serializar_paquete(paquete);
    if (send(fd_storage, paquete_s, espacio_paquete_serializado(paquete), 0) <= 0) {
        log_error(logger_worker, "Error o desconeccion en Master  al enviar la lectura");
        exit(EXIT_FAILURE);
    }

    log_info(logger_worker, "Query %s: Acción: LEER - Dirección Física: %d - Valor: %s", query_id, direccion, lectura);
    destruir_paquete(paquete);
    free(paquete_s);
    free(lectura);
}