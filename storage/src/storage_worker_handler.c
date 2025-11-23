#include "storage_worker_handler.h"

// Valor fijo definido
#define STORAGE_RESULT_OK 0
#define STORAGE_RESULT_ERROR -1

void *storage_worker_handler(void *arg)
{
    int socket = *(int *)arg;
    char *id_worker = NULL;
    free(arg);

    while (1)
    {
        t_list *package = recv_package(socket, logger_storage);
        if (!package)
        {
            log_error(logger_storage, "Error al recibir paquete del socket %d", socket);
            break;
        }
        
        op_code opcode = get_opcode(package);

        switch (opcode)
        {
        case HANDSHAKE_WORKER_STORAGE:
        {
            id_worker = string_duplicate(list_get(package, 0));
            CANT_WORKERS++;

            send(socket, &BLOCK_SIZE, sizeof(int), 0);

            log_info(logger_storage,
                     "##Se conecta el Worker %s - Cantidad de Workers: %d",
                     id_worker, CANT_WORKERS);
            break;
        }
        case INFO_FILE_TAG_STORAGE:
        {
            char* file = list_get(package, 0);
            char* tag = list_get(package, 1);

            t_metadata_file* metadata = storage_metadata_read(file, tag); 
            int size;
            if (metadata) {
                size = metadata->tamanio;
            } else {
                size = -1;
            }
            storage_metadata_destroy(metadata);

            paquete_t* size_tag_file = crear_paquete(INFO_FILE_TAG_WORKER);
            agregar_a_paquete(size_tag_file, &size, sizeof(int));

            enviar_paquete(socket, size_tag_file, logger_storage);

            log_info(logger_storage, "El Worker %s Solicito informacion sobre %s:%s", id_worker, file, tag);
            break;
        }
        case INSTRUCCION_STORAGE:
        {
            resultado_t result = desglozar_instruccion(package);

            if (send(socket, &result, sizeof(resultado_t), 0) <= 0)
            {
                log_error(logger_storage,
                          "Error o desconexión al enviar el resultado de la instrucción");
                close(socket);
                return NULL;
            }

            log_info(logger_storage,
                     "##Worker <%s> - Operación recibida: INSTRUCCION_STORAGE - Resultado: %d",
                     id_worker, result);
            break;
        }
        case MODIFICACIONES_STORAGE:
        {
            char* file = list_get(package, 0);
            char* tag = list_get(package, 1);
            int nro_pagina = *(int*)list_get(package, 2);
            char* contenido = list_get(package, 3);
            
            resultado_t result = storage_write(file, tag, nro_pagina, contenido);

            // Envia resultado fijo
            if (send(socket, &result, sizeof(resultado_t), 0) <= 0)
            {
                log_error(logger_storage,
                          "Error o desconexión al enviar el resultado de la instrucción");
                close(socket);
                return NULL;
            }

            log_info(logger_storage,
                     "Worker <%s> - Modificaciones realizadas sobre %s:%s en la pagina %d",
                     id_worker, file, tag, nro_pagina);
            break;
        }
        case SOLICITUD_STORAGE:
        {
            char* file = list_get(package, 0);
            char* tag = list_get(package, 1);
            int nro_pagina = *(int*)list_get(package, 2);

            paquete_t *page_package = crear_paquete(PAGINA_WORKER);

            char* contenido = (char*)malloc(BLOCK_SIZE);
            resultado_t r = storage_read(file, tag, nro_pagina, contenido);

            agregar_a_paquete(page_package, contenido, BLOCK_SIZE);
            enviar_paquete(socket, page_package, logger_storage);
            free(contenido);

            log_info(logger_storage,
                     "##Worker <%s> - Se envio el contenido de %s:%s - %d",
                     id_worker, file, tag, nro_pagina);
            break;
        }
        case DESCONEXION:
        {
            CANT_WORKERS--;
            log_info(logger_storage,
                     "##Se desconecta el Worker %s - Cantidad de Workers: %d",
                     id_worker, CANT_WORKERS);
            close(socket);
            return NULL;
        }
        default:
        {
            log_warning(logger_storage,
                        "##Worker %s - Operación desconocida (opcode=%d)",
                        id_worker ? id_worker : "?", opcode);
            break;
        }
        }

        list_destroy_and_destroy_elements(package, free);
    }

    if (id_worker)
        free(id_worker);

    return NULL;
}