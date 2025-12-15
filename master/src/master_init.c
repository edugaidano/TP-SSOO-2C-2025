#include "master_init.h"

void init(char *config_name)
{
    char *config_path = string_from_format("config/%s.config",config_name);
    config_master = config_create(config_path);
    
    if (!config_master)
    {
        printf("la ruta %s no existe, intentando nuevamente\n", config_path);
        free(config_path);
        config_path = build_config_path(config_name);
        config_master = config_create(config_path);
    }
    free(config_path);

    
    PUERTO_ESCUCHA = config_get_string_value(config_master, "PUERTO_ESCUCHA");
    ALGORITMO_PLANIFICACION = config_get_string_value(config_master, "ALGORITMO_PLANIFICACION");
    TIEMPO_AGING = config_get_int_value(config_master, "TIEMPO_AGING");
    LOG_LEVEL = config_get_string_value(config_master, "LOG_LEVEL");

    logger_master = log_create("logs_master.log", "master", true, log_level_from_string(LOG_LEVEL));
    log_info(logger_master, "log inicializado correctamente");

    querys_ready = list_create();
    querys_exec = list_create();
    workers = list_create();
    sem_init(&sem_workers, 0, 0);
    sem_init(&sem_ready, 0, 0);
    sem_init(&sem_int, 0, 0);
    sem_init(&sem_asign, 0, 0);
    sem_init(&sem_check_prior, 0, 1);

}