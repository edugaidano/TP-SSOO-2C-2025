#include "worker_init.h"

void init(char *config_name) {
    logger_worker = log_create("log_file.log", "logg_worker", true, LOG_LEVEL_INFO);
    log_info(logger_worker, "log inicializado correctamente");

    char *config_path = build_config_path(config_name, logger_worker);
    config_worker = config_create(config_path);

    IP_MASTER = config_get_string_value(config_worker, "IP_MASTER");
    PUERTO_MASTER = config_get_string_value(config_worker, "PUERTO_MASTER");
    IP_STORAGE = config_get_string_value(config_worker, "IP_STORAGE");
    PUERTO_STORAGE = config_get_string_value(config_worker, "PUERTO_STORAGE");
    TAM_MEMORIA = config_get_int_value(config_worker, "TAM_MEMORIA");
    RETARDO_MEMORIA = config_get_int_value(config_worker, "RETARDO_MEMORIA");
    ALGORITMO_REEMPLAZO = config_get_string_value(config_worker, "ALGORITMO_REEMPLAZO");
    PATH_SCRIPTS = config_get_string_value(config_worker, "PATH_SCRIPTS");

    
    if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "LRU")) {
        algoritmo_reemplazo = algoritmo_LRU;
        cronometro = temporal_create();
    } else if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "CLOCK-M")) {
        algoritmo_reemplazo = algoritmo_CLOCK_M;
    }
}