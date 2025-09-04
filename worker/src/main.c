#include <worker_main.h>

int main(int argc, char *argv[])
{
    init(argv[1]);

    log_info(logger_worker, "el log de worker funciona");

    int client_socket = connect_to_server(IP_MASTER, PUERTO_MASTER, logger_worker);

    return 0;
}
