#include <handsake.h>

void enviar_handshake (op_code codigo_operacion_handshake, int fd_destinatario, char* destinatario, char* id_worker) {
    paquete_t *paquete_handshake = crear_paquete(codigo_operacion_handshake);
    agregar_a_paquete(paquete_handshake, id_worker, string_length(id_worker) + 1);
    void* handshake = serializar_paquete(paquete_handshake);

    if (send(fd_destinatario, handshake, espacio_paquete_serializado(paquete_handshake), 0) == -1) {
        log_error(logger_worker, "No se pudo enviar el handshake al %s", destinatario);
        abort();
    }
    
    destruir_paquete(paquete_handshake);
    free(handshake);
}

void verificar_resultado_handshake (int fd_emisor, char* emisor) {
    if (resultado_handshake(fd_emisor, logger_worker) == ERROR) {
        log_error(logger_worker, "No se verificó el handshake con el %s", emisor);
        exit(EXIT_FAILURE);
    }

    if (string_equals_ignore_case(emisor, "STORAGE") && recv(fd_emisor, &tam_pagina, sizeof(int), MSG_WAITALL) <= 0) {
        log_error(logger_worker, "Error o desconeccion al recibir el tamaño de pagina");
        exit(EXIT_FAILURE);
    }

    log_info(logger_worker, "Handsake con el %s realizado correctamente", emisor);
}