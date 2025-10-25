#include <utils/paquetes.h>

paquete_t *crear_paquete(op_code codigo_operacion);
void destruir_paquete(paquete_t *paquete);
void agregar_a_paquete(paquete_t *paquete, void *valor, int size);
int espacio_paquete_serializado(paquete_t *paquete);
void *serializar_paquete(paquete_t *paquete);
paquete_t *recibir_paquete(int socket_cliente, t_log *logger);
buffer_t *obtener_siguiente_item(paquete_t *paquete);

paquete_t *crear_paquete(op_code codigo_operacion)
{
    paquete_t *paquete = malloc(sizeof(paquete_t));
    paquete->codigo_operacion = codigo_operacion;
    paquete->buffer = calloc(1, sizeof(buffer_t));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
    return paquete;
}

void enviar_paquete(int socket, paquete_t *paquete, t_log *logger)
{
    void *paquete_serializado = serializar_paquete(paquete);

    if (send(socket, paquete_serializado, espacio_paquete_serializado(paquete), 0) == -1)
    {
        log_warning(logger, "No se pudo enviar el paquete");
    }

    destruir_paquete(paquete);
    free(paquete_serializado);
}

void destruir_paquete(paquete_t *paquete)
{
    free_buffer(paquete->buffer);
    free(paquete);
}

void free_buffer(buffer_t *buffer)
{
    free(buffer->stream);
    free(buffer);
}

void agregar_a_paquete(paquete_t *paquete, void *valor, int size)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + size + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &size, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, size);

    paquete->buffer->size += size + sizeof(int);
}

int espacio_paquete_serializado(paquete_t *paquete)
{
    int espacio_total = sizeof(op_code) + sizeof(int) + paquete->buffer->size;
    if (paquete->buffer->size != 0) {
        espacio_total += sizeof(int);
    }
    return espacio_total;
}

// debe liberarse
void *serializar_paquete(paquete_t *paquete)
{
    void *stream = malloc(espacio_paquete_serializado(paquete));

    memcpy(stream, &(paquete->codigo_operacion), sizeof(op_code));
    int desplazamiento = sizeof(op_code);
    memcpy(stream + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento += sizeof(int);
    if (paquete->buffer->size != 0) {
        memcpy(stream + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
        desplazamiento += paquete->buffer->size;
        int final = 0;
        memcpy(stream + desplazamiento, &final, sizeof(int));
    }

    return stream;
}

// debe liberarse
paquete_t *recibir_paquete(int socket_cliente, t_log *logger)
{
    op_code codigo_operacion;
    int res = recv(socket_cliente, &codigo_operacion, sizeof(op_code), MSG_WAITALL);
    paquete_t *paquete;

    if (res <= 0)
    {
        paquete = crear_paquete(DESCONEXION);
        return paquete;
    }
    paquete = crear_paquete(codigo_operacion);

    int size_total;
    int size = 1;
    void *buffer;

    if (recv(socket_cliente, &size_total, sizeof(int), MSG_WAITALL) <= 0)
    {
        log_error(logger, "Error en recv (fd %d)", socket_cliente);
        exit(EXIT_FAILURE);
    }

    while (size != 0 && size_total != 0)
    {
        if (recv(socket_cliente, &size, sizeof(int), MSG_WAITALL) <= 0)
        {
            log_error(logger, "Error en recv (fd %d)", socket_cliente);
            exit(EXIT_FAILURE);
        }
        if (size > 0)
        {
            buffer = malloc(size);
            if (recv(socket_cliente, buffer, size, MSG_WAITALL) <= 0)
            {
                log_error(logger, "Error en recv (fd %d)", socket_cliente);
                exit(EXIT_FAILURE);
            }
            agregar_a_paquete(paquete, buffer, size);
            free(buffer);
        }
    }

    return paquete;
}

// debe liberarse
buffer_t *obtener_siguiente_item(paquete_t *paquete)
{
    if (!paquete->buffer || paquete->buffer->size == 0)
        return NULL;
    int size;
    void *content;
    memcpy(&size, paquete->buffer->stream, sizeof(int));
    if (size == 0)
        return NULL;
    content = malloc(size);
    memcpy(content, paquete->buffer->stream + sizeof(int), size);
    buffer_t *item = malloc(sizeof(buffer_t));
    item->size = size;
    item->stream = content;

    int new_size = paquete->buffer->size - sizeof(int) - size;
    void *new_buffer = malloc(new_size);
    memcpy(new_buffer, paquete->buffer->stream + sizeof(int) + size, new_size);
    free(paquete->buffer->stream);
    paquete->buffer->size = new_size;
    paquete->buffer->stream = new_buffer;

    return item;
}
int get_opcode(t_list *list)
{
    int *ptr = list_remove(list, 0);
    op_code code = *ptr;
    free(ptr);
    return code;
}

t_list *recv_package(int socket, t_log *logger)
{
    t_list *lista = list_create();

    paquete_t *paquete = recibir_paquete(socket, logger);

    int *codigo = malloc(sizeof(int));
    *codigo = paquete->codigo_operacion;
    list_add(lista, codigo);

    if (paquete->codigo_operacion == DESCONEXION)
    {
        return lista;
    }

    buffer_t *item = obtener_siguiente_item(paquete);
    while (item != NULL)
    {
        char *dato = malloc(item->size);
        memcpy(dato, item->stream, item->size);
        list_add(lista, dato);

        item = obtener_siguiente_item(paquete);
    }

    destruir_paquete(paquete);
    return lista;
}