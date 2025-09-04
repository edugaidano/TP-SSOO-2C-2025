#include <worker_main.h>

int main(int argc, char *argv[])
{
    init(argv[1]);

    int master_socket = connect_to_server(IP_MASTER, PUERTO_MASTER, logger_worker);

    return 0;
}
