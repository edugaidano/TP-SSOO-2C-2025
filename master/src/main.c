#include "master_main.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Pasar por parámetro el nombre del archivo .conf!\n");
        exit(EXIT_FAILURE);
    }
    signal(SIGPIPE, SIG_IGN);

    init(argv[1]);

    atexit(liberar_recursos);
    signal(SIGINT, exit);

    int server_socket = create_server(PUERTO_ESCUCHA, logger_master);

    pthread_t process_thread;
    pthread_create(&process_thread, NULL, &process_handler, NULL);
    pthread_detach(process_thread);

    master_network_handler(&server_socket);

    return 0;
}