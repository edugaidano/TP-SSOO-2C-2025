#include <master_network_handler.h>

void *master_network_handler(void *arg) {
    int *aux_i = (int*){arg};
    char* aux_c;
    int socket_server = *aux_i;
    while(1) {
        int fd = accept_connection(socket_server, logger_master);
        paquete_t *paquete_handshake = recibir_paquete(fd, logger_master);
        buffer_t *buffer = obtener_siguiente_item(paquete_handshake);

        switch (paquete_handshake->codigo_operacion) {
            case HANDSHAKE_QUERY_MASTER:
                info_query nueva_query;
                nueva_query.fd = fd;
                nueva_query.paht = (char*){buffer->stream}; 
                buffer = obtener_siguiente_item(paquete_handshake);
                aux_c = (char*){buffer->stream};
                nueva_query.prioridad = atoi(aux_c);

                pthread_t hilo_administrar_query;
                pthread_create(&hilo_administrar_query, NULL, &administrar_query, &nueva_query);
                pthread_detach(hilo_administrar_query);

                break;
            case HANDSHAKE_WORKER_MASTER:
                info_worker nuevo_worker;
                nuevo_worker.fd = fd;
                nuevo_worker.identificador = (char*){buffer->stream};
                
                pthread_t hilo_administrar_worker;
                pthread_create(&hilo_administrar_worker, NULL, &administrar_worker, &nuevo_worker);
                pthread_detach(hilo_administrar_worker);

                break;
            default:
                enviar_resultado_handshake(fd, ERROR, logger_master);
                log_error(logger_master, "Modulo distinto a Query Control o Worker se intento conctar a Master");
                break;
        }
    }
    return NULL;
}

//Para testear:
char* test_path;

void *administrar_query(void* arg) {
    info_query* query = (info_query*){arg};
    enviar_resultado_handshake(query->fd, OK, logger_master);
    log_info(
        logger_master, 
        "## Se conecta un Query Control para ejecutar la Query %s con prioridad %d - Id asignado: /TODO/. Nivel multiprogramación /TODO/",
        query->paht, 
        query->prioridad // Falta el ID y el Nvl de multiprogramacion
    );

    test_path = string_duplicate(query->paht); // MOCK
    //TODO

    while (1) {
    }
    

    return NULL;
}

void *administrar_worker(void* arg) {
    info_worker* worker = (info_worker*){arg};
    enviar_resultado_handshake(worker->fd, OK, logger_master);
    log_info(
        logger_master,
        "## Se conecta el Worker %s - Cantidad total de Workers: /TODO/",
        worker->identificador // Falta la cantidad total
    );

    // Test
    paquete_t *paquete_path = crear_paquete(PATH_MASTER_WORKER);
    agregar_a_paquete(paquete_path, test_path, string_length(test_path) + 1); // MOCK
    void* path = serializar_paquete(paquete_path);

    log_info(logger_master, "path %s ", test_path);

    if (send(worker->fd, path, espacio_paquete_serializado(paquete_path), 0)  == -1) {
        log_error(logger_master, "No se pudo enviar el path al Worker");
        abort();
    }

    //TODO
    log_info(logger_master, "## Se envía la Query <QUERY_ID> al Worker <WORKER_ID>"); 
    return NULL;
}