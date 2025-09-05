#include <storage_main.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Pasar por parámetro el nombre del archivo .conf!\n");
        exit(EXIT_FAILURE);
    }

    init(argv[1]);

    int server_socket = create_server(PUERTO_ESCUCHA, logger_storage);

    while (1)
    {
        int worker_socket = accept_connection(server_socket, logger_storage);
        //int *socket_ptr = malloc(sizeof(int));
        //*socket_ptr = worker_socket;

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, &storage_worker_handler, &worker_socket);
        pthread_detach(client_thread);
    }

    return 0;
}
