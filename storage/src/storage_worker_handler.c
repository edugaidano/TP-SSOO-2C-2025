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
        op_code opcode = get_opcode(package);

        switch (opcode)
        {
        case HANDSHAKE_WORKER_STORAGE:
        {
            // HANDSHAKE: no aplicar RETARDO_OPERACION
            id_worker = string_duplicate(list_get(package, 0));

            // protege contador de workers
            pthread_mutex_lock(&worker_count_mutex);
            CANT_WORKERS++;
            pthread_mutex_unlock(&worker_count_mutex);

            // responder con BLOCK_SIZE
            send(socket, &BLOCK_SIZE, sizeof(int), 0);

            log_info(logger_storage, "##Se conecta el Worker %s - Cantidad de Workers: %d", id_worker, CANT_WORKERS);
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

            /*log_info(logger_storage, "El Worker %s - Solicito informacion sobre %s:%s (size=%d)", 
                id_worker ? id_worker : "?", file, tag, size);*/
            break;
        }
        case INSTRUCCION_STORAGE:
        {
            usleep(RETARDO_OPERACION * 1000);
            
            rta_storage result = desglozar_instruccion(package);

            // enviar resultado al Worker
            if (send(socket, &result, sizeof(rta_storage), 0) <= 0)
            {
                log_error(logger_storage, "Error o desconexión al enviar el resultado de la instrucción");
                close(socket);
                // limpiar paquete antes de salir
                list_destroy_and_destroy_elements(package, free);
                if (id_worker) free(id_worker);
                return NULL;
            }

            break;
        }
        case MODIFICACIONES_STORAGE:
        {
            usleep(RETARDO_OPERACION * 1000);

            char* file = list_get(package, 0);
            char* tag = list_get(package, 1);
            int nro_pagina = *(int*)list_get(package, 2);
            char* contenido = list_get(package, 3);
            char* query_id = list_get(package, 4);
            
            rta_storage result = storage_write(file, tag, nro_pagina, contenido, query_id);

            if (send(socket, &result, sizeof(rta_storage), 0) <= 0)
            {
                log_error(logger_storage, "Error o desconexión al enviar el resultado de la instrucción");
                close(socket);
                list_destroy_and_destroy_elements(package, free);
                if (id_worker) free(id_worker);
                return NULL;
            }

            break;
        }
        case SOLICITUD_STORAGE:
        {
            usleep(RETARDO_OPERACION * 1000);

            char* file = list_get(package, 0);
            char* tag = list_get(package, 1);
            int nro_pagina = *(int*)list_get(package, 2);
            char* query_id = (char*)list_get(package, 3);

            paquete_t *page_package = crear_paquete(PAGINA_WORKER);

            char* contenido = (char*)malloc(BLOCK_SIZE);
            rta_storage r = storage_read(file, tag, nro_pagina, contenido, query_id);

            agregar_a_paquete(page_package, &r, sizeof(rta_storage));
            agregar_a_paquete(page_package, contenido, BLOCK_SIZE);
            enviar_paquete(socket, page_package, logger_storage);
            free(contenido);

            break;
        }
        case DESCONEXION:
        {
            // decrementar contador de workers protegido
            pthread_mutex_lock(&worker_count_mutex);
            CANT_WORKERS--;
            pthread_mutex_unlock(&worker_count_mutex);

            log_info(logger_storage, "##Se desconecta el Worker <%s> - Cantidad de Workers: <%d>", 
                id_worker ? id_worker : "?", CANT_WORKERS);
            close(socket);
            // limpiar paquete
            list_destroy_and_destroy_elements(package, free);
            if (id_worker) free(id_worker);
            return NULL;
        }
        default:
        {
            log_warning(logger_storage, "##Worker %s - Operación desconocida (opcode=%d)",
                id_worker ? id_worker : "?", opcode);
            break;
        }
        }

        list_destroy_and_destroy_elements(package, free);
    }
    
    return NULL;
}