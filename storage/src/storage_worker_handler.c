#include <storage_worker_handler.h>

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
                     "##Se conecta el Worker <%s> - Cantidad de Workers: <%d>",
                     id_worker, CANT_WORKERS);
            break;
        }
        case INSTRUCCION_STORAGE:
        {
            // TODO: desglosar paquete en el futuro
            resultado_t result = STORAGE_RESULT_OK; 

            // Envia resultado fijo
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
        case SOLICITUD_STORAGE:
        {
            // TODO: en futuros checks enviar datos reales
            paquete_t *page_package = crear_paquete(PAGINA_WORKER);

            // Envia contenido fijo del bloque/página
            char *contenido_fijo = "A";
            agregar_a_paquete(page_package, contenido_fijo, strlen(contenido_fijo) + 1);

            // Envia número de página ficticio
            double pagina = 0.0;
            agregar_a_paquete(page_package, &pagina, sizeof(double));

            enviar_paquete(socket, page_package, logger_storage);

            log_info(logger_storage,
                     "##Worker <%s> - Operación recibida: SOLICITUD_STORAGE - Contenido fijo enviado",
                     id_worker);
            break;
        }
        case DESCONEXION:
        {
            CANT_WORKERS--;
            log_info(logger_storage,
                     "##Se desconecta el Worker <%s> - Cantidad de Workers: <%d>",
                     id_worker, CANT_WORKERS);
            close(socket);
            return NULL;
        }
        default:
        {
            log_warning(logger_storage,
                        "##Worker <%s> - Operación desconocida (opcode=%d)",
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