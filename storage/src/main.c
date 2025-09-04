#include <storage_main.h>

int main(int argc, char *argv[])
{
    init(argv[1]);

    int server_socket = create_server(PUERTO_ESCUCHA, logger_storage);

    while (1)
    {
        int worker_socket = accept_connection(server_socket, logger_storage);
        int *socket_ptr = malloc(sizeof(int));
        *socket_ptr = worker_socket;

        pthread_t client_thread;
        pthread_create(client_thread, NULL, worker_handler, socket_ptr);
    }

    return 0;
}
