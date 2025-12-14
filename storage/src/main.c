#include "storage_main.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Pasar por parámetro el nombre del archivo .config!\n");
        exit(EXIT_FAILURE);
    }

    // Inicializa logger, config y FRESH_START si corresponde
    init(argv[1]);

    atexit(liberar_recursos);
    signal(SIGINT, exit);

    int server_socket = create_server(PUERTO_ESCUCHA, logger_storage);
    log_info(logger_storage, "Servidor Storage escuchando en puerto %s", PUERTO_ESCUCHA);

    while (1)
    {
        int worker_socket = accept_connection(server_socket, logger_storage);
        if (worker_socket < 0)
        {
            log_error(logger_storage, "Error al aceptar conexión");
            continue;
        }

        int *socket_ptr = malloc(sizeof(int));
        *socket_ptr = worker_socket;

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, (void *)storage_worker_handler, socket_ptr);
        pthread_detach(client_thread);
    }

    return 0;
}
