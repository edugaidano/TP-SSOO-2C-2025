#include <master_network_handler.h>

void *master_network_handler(void *arg) {
    int *aux = (int*){arg};
    int socket_server = *aux;
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
                char* aux_c = (char*){buffer->stream};
                nueva_query.prioridad = atoi(aux_c);

                pthread_t hilo_administrar_query;
                pthread_create(&hilo_administrar_query, NULL, &administrar_query, &nueva_query);
                pthread_detach(hilo_administrar_query);

                enviar_resultado_handshake(fd, OK, logger_master);
                log_info(
                        logger_master, 
                        "## Se conecta un Query Control para ejecutar la Query %s con prioridad %d - Id asignado: /TODO/. Nivel multiprogramación /TODO/",
                        nueva_query.paht, 
                        nueva_query.prioridad // Falta el ID y el Nvl de multiprogramacion
                    );
                break;
            case HANDSHAKE_WORKER_MASTER:
                enviar_resultado_handshake(fd, OK, logger_master);
/*
                pthread_t hilo_administrar_worker;
                pthread_create(&hilo_administrar_worker, NULL, &administrar_worker, &fd);
                pthread_detach(hilo_administrar_worker);
*/
                log_info(logger_master, "Worker conectado");
                break;
            default:
                enviar_resultado_handshake(fd, ERROR, logger_master);
                log_error(logger_master, "Modulo distinto a Query Control o Worker se intento conctar a Master");
                break;
        }
    }
    return NULL;
}

void *administrar_query(void* info_query) {
    return NULL;
}
void *administrar_worker(void* arg) {
    return NULL;
}