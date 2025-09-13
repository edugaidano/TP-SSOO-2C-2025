#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include <utils/networking.h>
#include <worker_globals.h>

void enviar_handshake (op_code codigo_operacion_handshake, int fd_destinatario, char* destinatario, char* id_worker);
void verificar_resultado_handshake (int fd_emisor, char* emisor);

#endif