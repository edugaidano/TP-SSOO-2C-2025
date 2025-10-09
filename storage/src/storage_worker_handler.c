#include <storage_worker_handler.h>

void *storage_worker_handler(void *arg)
{
    int socket = *(int *)arg;
    char *id_worker;
    free(arg);

    while (1)
    {
        t_list *package = recv_package(socket, logger_storage);
        op_code opcode = get_opcode(package);

        switch (opcode)
        {
        case HANDSHAKE_WORKER_STORAGE:
        {
            id_worker = string_duplicate(list_get(package, 0));
            CANT_WORKERS++;
            send(socket, &BLOCK_SIZE, sizeof(int), 0);

            log_info(logger_storage, "##Se conecta el Worker <%s> - Cantidad de Workers: <%d>", id_worker, CANT_WORKERS);
            break;
        }
        case INSTRUCCION_STORAGE:
            //TODO: desglosar paquete y realizar instrucciones (revisar workerr/src/interpreter.c)
            resultado_t result = OK;
            if (send(socket, &result, sizeof(resultado_t), 0) <= 0) {
                log_error(logger_storage, "Error o desconeccion al enviar el resultado de la instruccion");
                exit(EXIT_FAILURE);
            }
            break;
        case SOLICITUD_STORAGE: 
        {
            //TODO: buscar pagina exacta (revisar workerr/src/memoria.c > solicitar_pagina())
            paquete_t* page_package = crear_paquete(PAGINA_WORKER);
            agregar_a_paquete(page_package, "A", BLOCK_SIZE); //Contenido del bloque/pagina
            agregar_a_paquete(page_package, &(double){0.0}, sizeof(double)); //Numero de pagina
            enviar_paquete(socket, page_package, logger_storage);
            break;
        }
        case DESCONEXION:
        {
            CANT_WORKERS--;
            log_info(logger_storage, "##Se desconecta el Worker <%s> - Cantidad de Workers: <%d>", id_worker, CANT_WORKERS);
            close(socket);
            return NULL;
            break;
        }
        default:
            return NULL;
        }

        list_destroy_and_destroy_elements(package, free);
    }

    return NULL;
}