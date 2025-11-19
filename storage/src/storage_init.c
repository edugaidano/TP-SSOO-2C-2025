#include "storage_init.h"

void init(char *config_name)
{
    logger_storage = log_create("log_file.log", "STORAGE", true, LOG_LEVEL_INFO);
    log_info(logger_storage, "Logger del Storage inicializado correctamente");

    char *config_path = build_config_path(config_name, logger_storage);
    config_storage = config_create(config_path);

    PUERTO_ESCUCHA = config_get_string_value(config_storage, "PUERTO_ESCUCHA");
    FRESH_START = config_get_int_value(config_storage, "FRESH_START");
    RETARDO_OPERACION = config_get_int_value(config_storage, "RETARDO_OPERACION");
    RETARDO_ACCESO_BLOQUE = config_get_int_value(config_storage, "RETARDO_ACCESO_BLOQUE");
    FS_SIZE = config_get_int_value(config_storage, "FS_SIZE");
    BLOCK_SIZE = config_get_int_value(config_storage, "BLOCK_SIZE");

    CANT_WORKERS = 0;

    log_info(logger_storage,
             "Config cargada -> PUERTO:%s | FRESH_START:%d | FS_SIZE:%d | BLOCK_SIZE:%d",
             PUERTO_ESCUCHA, FRESH_START, FS_SIZE, BLOCK_SIZE);

    if (FRESH_START)
    {
        log_info(logger_storage, "Iniciando FRESH_START...");
        storage_fresh_start("FS", FS_SIZE, BLOCK_SIZE);
    }

    int blocks_count = FS_SIZE / BLOCK_SIZE;
    if (bitmap_init("FS", blocks_count) < 0)
    {
        log_error(logger_storage, "Error al inicializar el bitmap");
        exit(EXIT_FAILURE);
    }

    log_info(logger_storage, "Bitmap inicializado correctamente (%d bloques)", blocks_count);
}