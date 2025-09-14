#ifndef PAQUETES_H
#define PAQUETES_H

#include <utils/networking.h>
#include <utils/op_codes.h>

typedef struct {
    int size;
    void* stream;
} buffer_t;

typedef struct {
    op_code codigo_operacion;
    buffer_t* buffer;
} paquete_t;

paquete_t* crear_paquete(op_code codigo_operacion);
void destruir_paquete(paquete_t* paquete);
void liverar_buffer(buffer_t* buffer);
void agregar_a_paquete(paquete_t* paquete, void* valor, int size);
int espacio_paquete_serializado(paquete_t* paquete);
void* serializar_paquete(paquete_t* paquete);
paquete_t* recibir_paquete(int socket_cliente, t_log* logger);
buffer_t* obtener_siguiente_item(paquete_t* paquete);

#endif