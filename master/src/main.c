#include <master_main.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Pasar por parámetro el nombre del archivo .conf!\n");
        exit(EXIT_FAILURE);
    }

    init(argv[1]);

    log_info(logger_master, "el include esta bien hecho");

    int server_socket = create_server(PUERTO_ESCUCHA, logger_master);

    pthread_t main_thread, connections_thread;
    pthread_create(&main_thread, NULL, &process_handler, NULL);
    pthread_create(&connections_thread, NULL, &master_network_handler, &server_socket);
    pthread_detach(connections_thread);
    pthread_detach(main_thread);

    while (1)
    {
    } // para que no termine

    log_destroy(logger_master);
    config_destroy(config_master);

    return 0;
}