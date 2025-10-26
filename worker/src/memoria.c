#include "memoria.h"

// Private Functions //

int marco_libre();
void interaccion_con_memoria(void* dest, void* src, int size, nodo_marco* n_marco);
file_tag* agregar_file_tag_en_memoria(char* identificador);
nodo_pagina* solicitar_pagina(char* identificador, int nro_pagina);

// Public Functions //

void init_memoria() {
    // malloc memoria
    memoria = malloc(TAM_MEMORIA);

    // Lista de marcos
    cantidad_marcos = TAM_MEMORIA/tam_pagina;
    for (int i = 0; i < cantidad_marcos; i++) {
        nodo_marco* n_marco = malloc(sizeof(nodo_marco));
        n_marco->puntero_marco = (char*)memoria + i * tam_pagina;
        list_add(marco, n_marco);
    }
    
    // bitmap
    bit_map_marco = malloc( sizeof(bool) * cantidad_marcos);
    for (int i = 0; i < cantidad_marcos; i++) {
        bit_map_marco[i] = false;
    }

    if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "CLOCK-M")) {
        nodo_marco* n_marco = list_get(marco, 0);
        victima_clock = n_marco;
    }
    
    log_info(logger_worker, "Memoria iniciada");
}

file_tag* file_tag_en_memoria(char*identificador) {
    int file_tag_disp = list_size(file_tag_pages);
    if (file_tag_disp == 0) {
        return agregar_file_tag_en_memoria(identificador);
    }

    file_tag* ft = NULL;
    for (int i = 0; i < file_tag_disp; i++) {
        ft = list_get(file_tag_pages, i);
        if (string_equals_ignore_case(ft->identificador, identificador)) {
            return ft;
        }
    }
    return agregar_file_tag_en_memoria(identificador);
}

void agregar_file_tag(paquete_t* paquete, char* ft) {
    char **datos = string_split(ft, ":");
    agregar_a_paquete(paquete, datos[0], string_length(datos[0]) + 1);               // NOMBRE_FILE
    agregar_a_paquete(paquete, datos[1], string_length(datos[1]) + 1);               // TAG
    string_array_destroy(datos);
}

void free_file_tag(void* arg) {
    file_tag* ft = (file_tag*)arg;

    for (int i = 0; i < list_size(file_tag_pages); i++) {
        file_tag* aux = list_get(file_tag_pages, i);
        if (string_equals_ignore_case(aux->identificador, ft->identificador)) {
            list_remove(file_tag_pages, i);
            break;
        }
    }

    list_destroy_and_destroy_elements(ft->tabla_paginas, free);
    free(ft->identificador);
    free(ft);
}

nodo_pagina* pagina_en_Tabla(char* identificador, int nro_pagina) {
    file_tag* ft = file_tag_en_memoria(identificador);
    
    nodo_pagina* pagina = list_get(ft->tabla_paginas, nro_pagina);
    if (pagina->presencia) {
        return pagina;
    }
    
    return solicitar_pagina(identificador, nro_pagina);
}

void escribir_pagina(nodo_pagina* pagina, char* identificador, int direccion_base, char* datos) {
    int direccion_en_pagina = direccion_base % tam_pagina;
    int size_dato = string_length(datos) + 1;

    nodo_marco* n_marco = list_get(marco, pagina->nro_marco);

    char* puntero = n_marco->puntero_marco + direccion_en_pagina;
    pagina->modificado = true;
    pagina->uso = true;

    if (size_dato <= tam_pagina - direccion_en_pagina) { // si el dato entra en la pagina actual
        interaccion_con_memoria(puntero, datos, size_dato, n_marco);
    } else {
        int size_restante = size_dato - direccion_en_pagina;
        int cantidad_paginas_escribir = ceil(size_restante/tam_pagina);
        int pagina_siguiente = pagina->nro_pagina + 1;
        
        interaccion_con_memoria(puntero, datos, size_dato - size_restante, n_marco);
        for (int i = 0; i < cantidad_paginas_escribir; i++) {  
            nodo_pagina* pagina_extra = pagina_en_Tabla(identificador, pagina_siguiente);
            pagina_extra->modificado = true;
            pagina_extra->uso= true;
            n_marco = list_get(marco, pagina_extra->nro_marco);
            // Como es una pagina nueva, se escrvira desde la base de la misma (puntero_marco)
            int bytes_escribir;
            if (cantidad_paginas_escribir - i == 0) {
                bytes_escribir = size_restante;
            } else {
                bytes_escribir = tam_pagina;
            } 
            
            interaccion_con_memoria(n_marco->puntero_marco, datos + (size_dato - size_restante), bytes_escribir, n_marco);
            pagina_siguiente++;
            size_restante -= bytes_escribir;
        }
    }

    log_info(logger_worker, "Query %s: Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", query_id, direccion_base, datos);
}

void leer_pagina(nodo_pagina* pagina, char*identificador, int direccion, int size) {
    int direccion_en_pagina = direccion % tam_pagina;

    pagina->uso = true;
    nodo_marco* n_marco = list_get(marco, pagina->nro_marco);
    char* puntero_memoria = n_marco->puntero_marco + direccion_en_pagina;
    char* lectura = malloc(size);
    if (size <= tam_pagina - direccion_en_pagina) { // si el dato entra en la pagina actual
        interaccion_con_memoria(lectura, puntero_memoria, size, n_marco);
    } else {
        int size_restante = size - direccion_en_pagina;
        int cantidad_paginas_leer = ceil(size_restante/tam_pagina);
        int pagina_siguiente = pagina->nro_pagina + 1;
        
        interaccion_con_memoria(lectura, puntero_memoria, size - size_restante, n_marco);
        for (int i = 0; i < cantidad_paginas_leer; i++) {  
            nodo_pagina* pagina_extra = pagina_en_Tabla(identificador, pagina_siguiente);
            pagina_extra->uso= true;
            n_marco = list_get(marco, pagina_extra->nro_marco);
            // Como es una pagina nueva, se leera desde la base de la misma (puntero_marco)
            int bytes_leer;
            if (cantidad_paginas_leer - i == 0) {
                bytes_leer = size_restante;
            } else {
                bytes_leer = tam_pagina;
            } 
            
            interaccion_con_memoria(lectura + (size - size_restante), n_marco->puntero_marco, bytes_leer, n_marco);
            pagina_siguiente++;
            size_restante -= bytes_leer;
        }
    }
    
    paquete_t* paquete = crear_paquete(LECTURA_MASTER);
    agregar_a_paquete(paquete, identificador, string_length(identificador) + 1);
    agregar_a_paquete(paquete, lectura, size);
    enviar_paquete(master_socket, paquete, logger_worker);
    
    resultado_t result;
    if (recv(master_socket, &result, sizeof(resultado_t), 0) <= 0) {
        log_error(logger_worker, "Error o desconeccion en Master al recibir un resultado de la lectura");
        free(lectura);
        exit(EXIT_FAILURE);
    }
    
    if (result == ERROR) {
        log_error(logger_worker, "Error al realizar la lectura en el master");
        free(lectura);
        exit(EXIT_FAILURE);
    }

    log_info(logger_worker, "Query %s: Acción: LEER - Dirección Física: %d - Valor: %s", query_id, direccion, lectura);
    free(lectura);
}

// Private Functions //

int marco_libre() {
    for (int i = 0; i < cantidad_marcos; i++) {
        if (!bit_map_marco[i]) {
            return i;
        }
    }
    return -1;
}

void interaccion_con_memoria(void* dest, void* src, int size, nodo_marco* n_marco) {
    sleep(RETARDO_MEMORIA/1000);
    memcpy(dest, src, size);
    if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "LRU")) {
        n_marco->time = temporal_gettime(cronometro);
    }
}

file_tag* agregar_file_tag_en_memoria(char* identificador) {
    paquete_t* paquete = crear_paquete(INFO_FILE_TAG_STORAGE);
    agregar_file_tag(paquete, identificador);
    enviar_paquete(storage_socket, paquete, logger_worker);
    
    paquete = recibir_paquete(storage_socket, logger_worker); // Paquete: Tamaño del FILE:TAG
    if (paquete->codigo_operacion != INFO_FILE_TAG_WORKER) {
        log_error(logger_worker, "Se recibio un paquete desconcocido al solicitar informacion sobre %s", identificador);
        destruir_paquete(paquete);
        exit(EXIT_FAILURE);
    }
    
    buffer_t* buffer = obtener_siguiente_item(paquete);
    file_tag* ft = malloc(sizeof(file_tag));
    ft->identificador = string_duplicate(identificador);
    ft->tabla_paginas = list_create();
    ft->cantidad_paginas = 0;

    int size_ft = *(int*)buffer->stream;
    int entradas_tabla = ceil(size_ft/size_ft);
    for (int i = 0; i < entradas_tabla; i++) {
        nodo_pagina* pagina = malloc(sizeof(nodo_pagina));
        pagina->nro_pagina = i;
        pagina->nro_marco = -1;
        pagina->presencia = false;
        pagina->modificado = false;
        pagina->uso = false;
        list_add_in_index(ft->tabla_paginas, i, pagina);
    }
    list_add(file_tag_pages, ft);

    free_buffer(buffer);
    destruir_paquete(paquete);
    return ft;
}

nodo_pagina* solicitar_pagina(char* identificador, int nro_pagina) {
    char **datos = string_split(identificador, ":");
    log_info(logger_worker, "Query %s: - Memoria Miss - File: %s - Tag: %s - Pagina: %d", query_id, datos[0], datos[1], nro_pagina);

    file_tag* ft = file_tag_en_memoria(identificador);

    paquete_t* paquete = crear_paquete(SOLICITUD_STORAGE);
    agregar_file_tag(paquete, identificador);
    agregar_a_paquete(paquete, &nro_pagina, sizeof(int));                           // Nro_Pagina
    enviar_paquete(storage_socket, paquete, logger_worker);

    paquete = recibir_paquete(storage_socket, logger_worker); // Paquete: Informacion de la Pagina
    if (paquete->codigo_operacion != PAGINA_WORKER) {
        log_error(logger_worker, "Se recibio un paquete desconcocido al solicitar pagina");
        destruir_paquete(paquete);
        exit(EXIT_FAILURE);
    }
    buffer_t* buffer = obtener_siguiente_item(paquete);

    int indice = marco_libre();
    if (indice == -1) {
        indice = algoritmo_reemplazo(identificador, nro_pagina);
    }
    log_info(logger_worker, "Query %s: Se asigna el Marco: %i a la Página: %i perteneciente al - File: %s - Tag: %s", query_id, indice, nro_pagina, datos[0], datos[1]);
    bit_map_marco[indice] = true;
    nodo_pagina* pagina = list_get(ft->tabla_paginas, nro_pagina);
    pagina->nro_marco = indice;
    pagina->presencia = true;
    nodo_marco* n_marco = list_get(marco, indice);
    memcpy(n_marco->puntero_marco, buffer->stream, buffer->size);
    n_marco->identificador = ft->identificador;
    n_marco->pagina = pagina;
    free_buffer(buffer);
    destruir_paquete(paquete);
    
    ft->cantidad_paginas++; 
    log_info(logger_worker, "Query %s: - Memoria Add - File: %s - Tag: %s - Pagina: %d", query_id, datos[0], datos[1], nro_pagina);
    string_array_destroy(datos);
    return pagina;
}