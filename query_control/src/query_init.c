#include "query_init.h"

void init(char *config_name, char *query_name)
{
    char *config_path = string_from_format("config/%s.conf",config_name);
    config_query_control = config_create(config_path);
    
    if (!config_query_control)
    {
        printf("la ruta %s no existe, intentando nuevamente\n", config_path);
        free(config_path);
        config_path = build_config_path(config_name);
        config_query_control = config_create(config_path);
    }
    free(config_path);
    
    PUERTO_MASTER = config_get_string_value(config_query_control, "PUERTO_MASTER");
    IP_MASTER = config_get_string_value(config_query_control, "IP_MASTER");
    LOG_LEVEL = config_get_string_value(config_query_control, "LOG_LEVEL");

    char *log_file = string_from_format("log_%s.log", query_name);
    logger_query_control = log_create(log_file, "query_control", true, log_level_from_string(LOG_LEVEL));
    free(log_file);
    log_info(logger_query_control, "log inicializado correctamente");
}