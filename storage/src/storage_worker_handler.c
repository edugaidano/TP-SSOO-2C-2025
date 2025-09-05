#include <storage_worker_handler.h>

void *storage_worker_handler(void *arg) {
    int* aux = (int*){arg};
    int socket = *aux;

    paquete_t *paquete_handsake = recibir_paquete(socket, logger_storage);
    if (paquete_handsake->codigo_operacion != HANDSHAKE_WORKER_STORAGE) {
        log_error(logger_storage, "Se recivio un handshake de un modulo distinto a Worker");
        enviar_resultado_handshake(socket, ERROR, logger_storage);
        close(socket);
        exit(EXIT_FAILURE);
    }
    
    buffer_t *buffer = obtener_siguiente_item(paquete_handsake);
    char* identificador = (char*){buffer->stream};

    log_info(logger_storage, "##Se conecta el Worker %s - Cantidad de Workers: /TODO/", identificador);
    enviar_resultado_handshake(socket, OK, logger_storage);

    return NULL;
}