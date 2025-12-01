#include "storage_init.h"

void init(char *config_name)
{
    // storage.config
    char *config_path = string_from_format("config/%s.config",config_name);
    config_storage = config_create(config_path);
    if (!config_storage)
    {
        printf("la ruta %s no existe, intentando nuevamente\n", config_path);
        free(config_path);
        config_path = build_config_path(config_name);
        config_storage = config_create(config_path);
    }
    free(config_path);

    PUERTO_ESCUCHA = config_get_string_value(config_storage, "PUERTO_ESCUCHA");
    FRESH_START = config_get_string_value(config_storage, "FRESH_START");
    PUNTO_MONTAJE = config_get_string_value(config_storage, "PUNTO_MONTAJE");
    RETARDO_OPERACION = config_get_int_value(config_storage, "RETARDO_OPERACION");
    RETARDO_ACCESO_BLOQUE = config_get_int_value(config_storage, "RETARDO_ACCESO_BLOQUE");
    LOG_LEVEL = config_get_string_value(config_storage, "LOG_LEVEL");

    FS_SIZE = config_get_int_value(config_storage, "FS_SIZE");
    BLOCK_SIZE = config_get_int_value(config_storage, "BLOCK_SIZE");

    // LOGGER
    logger_storage = log_create("logs_storage.log", "storage", true, log_level_from_string(LOG_LEVEL));
    log_info(logger_storage, "log inicializado correctamente");

    log_info(logger_storage,
            "Config genral cargada -> PUERTO:%s | FRESH_START:%s | FS_SIZE:%d | BLOCK_SIZE:%d",
            PUERTO_ESCUCHA, FRESH_START, FS_SIZE, BLOCK_SIZE);

    // superclock.config
    CANT_WORKERS = 0;    
    
    // FRESH START
    if (string_equals_ignore_case(FRESH_START, "TRUE"))
    {
        log_info(logger_storage, "Iniciando FRESH_START...");
        storage_fresh_start();
    } else 
    {
        config_path = string_from_format("%s/superblock.config",PUNTO_MONTAJE);
        config_super_block = config_create(config_path);
        free(config_path); 
        if (!config_super_block) {
            log_error(logger_storage, "No existe %s/superblock.config",PUNTO_MONTAJE);
            log_info(logger_storage, "se recomienda activar el FRESH_START o crear el archivo con el formato correspondiente");
            exit(EXIT_FAILURE);
        }
        FS_SIZE = config_get_int_value(config_super_block, "FS_SIZE");
        BLOCK_SIZE = config_get_int_value(config_super_block, "BLOCK_SIZE");
    }

    log_info(logger_storage, "Config superblock cargada -> FS_SIZE:%d | BLOCK_SIZE:%d", FS_SIZE, BLOCK_SIZE);

    // HASH_INDEX
    config_hash_index = config_create("blocks_hash_index.config");
    char *zero_block = string_repeat('0', BLOCK_SIZE);
    char* hash = get_hash_from_content(zero_block);
    free(zero_block);
    load_hash_in_index(hash, 0);
    free(hash);

    // BITMAP
    int blocks_count = FS_SIZE / BLOCK_SIZE;
    if (bitmap_init(PUNTO_MONTAJE, blocks_count) < 0)
    {
        log_error(logger_storage, "Error al inicializar el bitmap");
        exit(EXIT_FAILURE);
    }

    log_info(logger_storage, "Bitmap inicializado correctamente (%d bloques)", blocks_count);

    init_locks_index();
}