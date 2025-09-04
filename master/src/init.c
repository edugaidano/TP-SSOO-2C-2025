#include "init.h"

void init(char *config_name)
{

    logger_master = log_create("log_file.log", "logg_master", true, LOG_LEVEL_INFO);
    log_info(logger_master, "log inicializado correctamente");

    char *config_path = build_config_path(config_name, logger_master);

    t_config *config_master = config_create(config_path);

    PUERTO_ESCUCHA = config_get_string_value(config_master, "PUERTO_ESCUCHA");
    ALGORITMO_PLANIFICACION = config_get_string_value(config_master, "ALGORITMO_PLANIFICACION");
    TIEMPO_AGING = config_get_int_value(config_master, "TIEMPO_AGING");
}