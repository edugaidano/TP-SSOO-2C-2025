#include <worker_main.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Uso: %s [archivo_conf] [ID Worker]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Iniciar Logs y Configs
    init(argv[1]);
    char ack[4];

    /*
        int master_socket = connect_to_server(IP_MASTER, PUERTO_MASTER, logger_worker);
        // Enviar handshake al Master
        paquete_t *handshake = crear_paquete(HANDSHAKE_WORKER_MASTER);
        agregar_a_paquete(handshake, argv[1], sizeof(argv[1]));
        enviar_paquete(master_socket, handshake);

        char ack[4];
        recv(master_socket, ack, 4, MSG_WAITALL);
        log_info(logger_worker, "## handshake con master realizado");
    */

    // Handshake con Storage
    int storage_socket = connect_to_server(IP_STORAGE, PUERTO_STORAGE, logger_worker);
    paquete_t *handshake_storage = crear_paquete(HANDSHAKE_WORKER_STORAGE);
    agregar_a_paquete(handshake_storage, argv[2], strlen(argv[2]) + 1);
    enviar_paquete(storage_socket, handshake_storage, logger_worker);
    log_info(logger_worker, "se envia handshake a storage, id enviado: %s", argv[2]);
    recv(storage_socket, &tam_pagina, sizeof(int), MSG_WAITALL);
    log_info(logger_worker, "se confirma la conexion con storage, tamaño de pagina: %d", tam_pagina);

    // Handshake con Master
    int master_socket = connect_to_server(IP_MASTER, PUERTO_MASTER, logger_worker);
    paquete_t *handshake_master = crear_paquete(HANDSHAKE_WORKER_MASTER);
    agregar_a_paquete(handshake_master, argv[2], strlen(argv[2]) + 1);
    enviar_paquete(master_socket, handshake_master, logger_worker);
    log_info(logger_worker, "se envia handshake a master, id enviado: %s", argv[2]);
    recv(master_socket, ack, 4, MSG_WAITALL);
    log_info(logger_worker, "se confirma la conexion con master");

    // Iniciar memoria interna
    init_memoria();
    t_list *instrucciones;

    while (true) {
        // Espera de Query (Paqute: nombre del archivo, query ID, PC)
        paquete_t *paquete_query = recibir_paquete(master_socket, logger_worker);
        if (paquete_query->codigo_operacion != SOLICITUD_EJECUCION) {
            log_error(logger_worker, "Se recibio un paquete con un op_code distinto a %d", SOLICITUD_EJECUCION);
            exit(EXIT_FAILURE);
        }

        buffer_t *buffer = obtener_siguiente_item(paquete_query);
        char *path = string_from_format("%s/%s", PATH_SCRIPTS, (char *){buffer->stream});
        liverar_buffer(buffer);

        buffer = obtener_siguiente_item(paquete_query);
        char *query_id = string_duplicate((char *){buffer->stream});
        liverar_buffer(buffer);

        buffer = obtener_siguiente_item(paquete_query);
        int pc = *(int *){buffer->stream};
        liverar_buffer(buffer);

        destruir_paquete(paquete_query);

        log_info(logger_worker, "## Query %s: Se recibe la Query. El path de operaciones es: %s", query_id, path);

        instrucciones = parsear_archivo(path);
        free(path);

        bool fin = false;
        // Lectura de instrucciones
        while (list_size(instrucciones) > pc && !fin) {
            t_instrucion *instruccion = list_get(instrucciones, pc);
            log_info(logger_worker, "## Query %s: FETCH - Program Counter: %d - %s", query_id, pc, instruccion->identificador);

            switch (instruccion->copi) {
            case CREATE:
                interpretar_CREATE(instruccion, query_id, storage_socket);
                break;
            case COMMIT:
                interpretar_COMMIT(instruccion, query_id, storage_socket);
                break;
            case DELETE:
                interpretar_DELETE(instruccion, query_id, storage_socket);
                break;
            case FLUSH:
                interpretar_FLUSH(instruccion, query_id, storage_socket);
                break;
            case TRUNCATE:
                interpretar_TRUNCATE(instruccion, query_id, storage_socket);
                break;
            case TAG:
                interpretar_TAG(instruccion, query_id, storage_socket);
                break;
            case READ:
                interpretar_READ(instruccion, query_id, storage_socket, master_socket);
                break;
            case WRITE:
                interpretar_WRITE(instruccion, query_id, storage_socket);
                break;
            default: // END
                interpretar_END(instruccion, query_id, master_socket);
                fin = true; // Sale del while y elimina la query
                break;
            }

            // Idea para manejar interrupciones desde el worker
            send(master_socket, &(op_code){CONSULTA_INTERRUPCION}, sizeof(int), 0); // Podria ser un paquete
            bool resultado;
            recv(master_socket, &resultado, sizeof(bool), MSG_WAITALL); // True: es necesario interrumpir la ejecucion, False: se continua con normalidad
            if (resultado)  {
                break; // sale del while y espera un nuevo query (Aqui se puede agregar un paquete si es necesario para el master)
            }

            pc++;
        }

        list_destroy_and_destroy_elements(instrucciones, destruir_instrucciones);
    }

    // Liveracion de Recursos
    config_destroy(config_worker);
    log_destroy(logger_worker);
    // list_destroy_and_destroy_elements(tabla_paginas, destruir_paginas); TODO
    free(memoria);

    return 0;
}
