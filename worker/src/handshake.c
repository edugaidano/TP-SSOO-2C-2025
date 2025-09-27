#include <handsake.h>

void enviar_handshake(op_code codigo_operacion_handshake, int fd_destinatario, char *destinatario, char *id_worker)
{
    paquete_t *paquete_handshake = crear_paquete(codigo_operacion_handshake);
    agregar_a_paquete(paquete_handshake, id_worker, string_length(id_worker) + 1);
    enviar_paquete(fd_destinatario, paquete_handshake, logger_worker);
}

void verificar_resultado_handshake(int fd_emisor, char *emisor)
{
    if (resultado_handshake(fd_emisor, logger_worker) == ERROR)
    {
        log_error(logger_worker, "No se verificó el handshake con el %s", emisor);
        exit(EXIT_FAILURE);
    }

    if (string_equals_ignore_case(emisor, "STORAGE") && recv(fd_emisor, &tam_pagina, sizeof(int), MSG_WAITALL) <= 0)
    {
        log_error(logger_worker, "Error o desconeccion al recibir el tamaño de pagina");
        exit(EXIT_FAILURE);
    }

    log_info(logger_worker, "Handsake con el %s realizado correctamente", emisor);
}