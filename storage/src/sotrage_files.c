#include "storage_files.h"

// Crea un nuevo File:Tag en el FS
rta_storage storage_create(char* file_name, char* tag, char* query_id)
{
    char* path = string_from_format("%s/files/%s", PUNTO_MONTAJE, file_name);
    mkdir(path, 0777);
    string_append_with_format(&path, "/%s", tag);
    int result_mkdir = mkdir(path, 0777);
    int saved_errno = errno;
    if (result_mkdir != 0 && saved_errno == EEXIST) {
        log_error(logger_storage, "Ya existe el file tag");
        return ERR_PREEXISTENCIA;
    }

    char* logical_blocks_path = string_from_format("%s/%s", path, "logical_blocks");
    mkdir(logical_blocks_path, 0777);
    free(logical_blocks_path);
    
    char* metadata_path = string_from_format("%s/%s", path, "metadata.config");
    free(path);
    FILE* meta = fopen(metadata_path, "w");
    free(metadata_path);
    if (!meta)
    {
        log_error(logger_storage, "Error al crear metadata.config para %s:%s (%s)", file_name, tag, strerror(errno));
        return NOT_DEF_ERR;
    }

    fprintf(meta, "TAMAÑO=0\nBLOCKS=[]\nESTADO=WORK_IN_PROGRESS\n");
    fclose(meta);

    add_lock(file_name, tag);
    log_info(logger_storage, "## %s - File Creado %s:%s", query_id, file_name, tag);
    return OP_EXITOSA;
}


// Lee la metadata.config de un File:Tag
t_metadata_file* storage_metadata_read(const char* file_name, const char* tag)
{
    char* meta_path  = string_from_format("files/%s/%s/metadata.config", file_name, tag);
    t_config* config = config_create(meta_path);
    free(meta_path);
    if (!config)
    {
        log_error(logger_storage, "No se pudo abrir metadata.config de %s:%s", file_name, tag);
        return NULL;
    }

    t_metadata_file* meta = malloc(sizeof(t_metadata_file));
    meta->tamanio = config_get_int_value(config, "TAMAÑO");
    meta->estado = string_duplicate(config_get_string_value(config, "ESTADO"));

    meta->blocks = list_create();
    char** blocks_array = config_get_array_value(config, "BLOCKS");
    int cant_blks = string_array_size(blocks_array);
    for (int i = 0; i < cant_blks; i++)
    {
        char* block_num = string_array_pop(blocks_array);
        list_add_in_index(meta->blocks, 0, block_num);
    }
    string_array_destroy(blocks_array);

    config_destroy(config);

    log_info(logger_storage, "Metadata leída de %s:%s (Tamaño=%d, Estado=%s, Bloques=%d)",
             file_name, tag, meta->tamanio, meta->estado, list_size(meta->blocks));

    return meta;
}

// Escribe la metadata.config
void storage_metadata_write(const char* file_name, const char* tag, t_metadata_file* metadata)
{
    char* meta_path  = string_from_format("files/%s/%s/metadata.config", file_name, tag);

    FILE* meta_file = fopen(meta_path, "w");
    free(meta_path);
    if (!meta_file)
    {
        log_error(logger_storage, "No se pudo abrir metadata.config para escribir %s:%s", file_name, tag);
        return;
    }

    // Serializar la lista de bloques en formato [1,2,3]
    char* blocks_str = string_duplicate("[");
    for (int i = 0; i < list_size(metadata->blocks); i++)
    {
        char* nro_blk = list_get(metadata->blocks, i);
        string_append(&blocks_str, nro_blk);
        if (i < list_size(metadata->blocks) - 1)
            string_append(&blocks_str, ",");
    }
    string_append(&blocks_str, "]");

    fprintf(meta_file, "TAMAÑO=%d\nBLOCKS=%s\nESTADO=%s\n", metadata->tamanio, blocks_str, metadata->estado);
    fclose(meta_file);
    free(blocks_str);

    log_info(logger_storage, "Metadata actualizada %s:%s (Tamaño=%d, Estado=%s, Bloques=%d)",
             file_name, tag, metadata->tamanio, metadata->estado, list_size(metadata->blocks));
}

// Libera la estructura en memoria
void storage_metadata_destroy(t_metadata_file* metadata)
{
    if (!metadata) return;
    list_destroy_and_destroy_elements(metadata->blocks, free);
    free(metadata->estado);
    free(metadata);
}

rta_storage storage_truncate(char* file_name, char* tag, int new_size, char* query_id)
{
    storage_wait(file_name, tag);
    t_metadata_file* meta = storage_metadata_read(file_name, tag);
    if (!meta)
    {
        storage_signal(file_name, tag);
        log_error(logger_storage, "No se pudo leer metadata de %s:%s para TRUNCATE", file_name, tag);
        return ERR_INEXISTENCIA;
    }
    
    if (string_equals_ignore_case(meta->estado, "COMMITED")) {
        storage_signal(file_name, tag);
        log_error(logger_storage, "TRUNCATE falló: %s:%s con estado COMMITED", file_name, tag);
        storage_metadata_destroy(meta);
        return ERR_WRITE_COMMITED;
    }

    // Calcular bloques 
    int current_blocks = list_size(meta->blocks);
    int needed_blocks = new_size / BLOCK_SIZE;

    log_info(logger_storage,
             "TRUNCATE - %s:%s Tamaño actual=%d (%d bloques) -> Nuevo tamaño=%d (%d bloques)",
             file_name, tag, meta->tamanio, current_blocks, new_size, needed_blocks);

    
    char* logical_path = string_from_format("files/%s/%s/logical_blocks", file_name, tag);
    // Si necesita más bloques
    if (needed_blocks > current_blocks)
    {
        for (int i = current_blocks; i < needed_blocks; i++)
        {
            char* path = string_from_format("%s/%05d.dat", logical_path, list_size(meta->blocks));
            link("physical_blocks/block0000.dat", path);
            free(path);
            log_info(logger_storage, 
                "## %s - %s:%s Se agregó el hard link del bloque lógico %05d al bloque 0000",
                query_id, file_name, tag, list_size(meta->blocks));

            // Agrega el bloque a metadata
            list_add(meta->blocks, string_duplicate("0"));

            log_info(logger_storage, "##TRUNCATE - Nuevo bloque asignado %d para %s:%s", 0, file_name, tag);
        }
    } 
    
    // libera los bloques sobrantes
    if (needed_blocks < current_blocks)
    {
        int blocks_to_remove = current_blocks - needed_blocks;
        
        for (int i = 0; i < blocks_to_remove; i++)
        {
            // Último bloque actual
            int last_index = list_size(meta->blocks) - 1;
            int block_to_free = atoi(list_get(meta->blocks, last_index));
            char* path = string_from_format("%s/%05d.dat", logical_path, last_index);

            // Borrar el hardlink lógico
            unlink(path);
            free(path);

            // Liberar del bitmap
            if (block_to_free != 0) {
                struct stat blk_stat;
                path = string_from_format("physical_blocks/block%04d.dat", block_to_free);
                stat(path, &blk_stat);
                if (blk_stat.st_nlink < 2)
                {
                    mark_block_free(block_to_free, query_id);
                }
                free(path);
            }

            // Quitar de metadata
            free(list_remove(meta->blocks, last_index));

            log_info(logger_storage, "##TRUNCATE - Bloque liberado %d para %s:%s", block_to_free, file_name, tag);
        }
    }

    meta->tamanio = new_size;
    storage_metadata_write(file_name, tag, meta);
    storage_signal(file_name, tag);
    

    storage_metadata_destroy(meta);
    log_info(logger_storage, "## %s - File Truncado %s:%s - Tamaño: %d", query_id, file_name, tag, new_size);
    return OP_EXITOSA;
}

rta_storage storage_write(char* file_name, char* tag, int l_block_num, char* buffer, char* query_id)
{
    usleep(RETARDO_ACCESO_BLOQUE * 1000);
    storage_wait(file_name, tag);
    t_metadata_file* meta = storage_metadata_read(file_name, tag);
    if (!meta)
    {
        storage_signal(file_name, tag);
        log_error(logger_storage, "No se pudo leer metadata para WRITE en %s:%s", file_name, tag);
        return ERR_INEXISTENCIA;
    }
    
    if (string_equals_ignore_case(meta->estado, "COMMITED")) {
        storage_signal(file_name, tag);
        log_error(logger_storage, "WRITE falló: %s:%s con estado COMMITED", file_name, tag);
        storage_metadata_destroy(meta);
        return ERR_WRITE_COMMITED;
    }
    
    // Valid que haya bloques asignados
    if (list_size(meta->blocks) < l_block_num)
    {
        storage_signal(file_name, tag);
        log_error(logger_storage, "WRITE falló: no existe el bloque %d en %s:%s", l_block_num, file_name, tag);
        storage_metadata_destroy(meta);
        return ERR_FUERA_LIMITE;
    }

    int p_block_num = atoi(list_get(meta->blocks, l_block_num));
    
    char* phys_path = string_from_format("physical_blocks/block%04d.dat", p_block_num);
    
    struct stat blk_stat;
    stat(phys_path, &blk_stat);
    
    if (blk_stat.st_nlink > 2 ) {
        free(phys_path);
        // Buscar nuevo bloque
        p_block_num = find_free_block();
        if (p_block_num == -1) {
            storage_signal(file_name, tag);
            storage_metadata_destroy(meta);
            return ERR_ESP_INSUFICIENTE; 
        } else {
            log_info(logger_storage, "## %s - Bloque Físico Reservado - Número de Bloque: %d",
                query_id, p_block_num);
        }

        // Reemplazar en config
        list_remove_and_destroy_element(meta->blocks, l_block_num, free);
        list_add_in_index(meta->blocks, l_block_num, string_itoa(p_block_num));
        storage_metadata_write(file_name, tag, meta);

        // linkear nuevo bloque
        char* log_path = string_from_format("files/%s/%s/logical_blocks/%05d.dat", file_name, tag, l_block_num);
        unlink(log_path);
        phys_path = string_from_format("physical_blocks/block%04d.dat", p_block_num);
        link(phys_path, log_path);
        log_info(logger_storage, 
            "## %s - %s:%s Se agregó el hard link del bloque lógico %05d al bloque %04d",
            query_id, file_name, tag, l_block_num, p_block_num);
    }
    
    storage_signal(file_name, tag);
    storage_metadata_destroy(meta);

    // TODO sem para bloques fisicos
    FILE* p_block = fopen(phys_path, "wb");
    fwrite(buffer, BLOCK_SIZE, 1, p_block);
    fclose(p_block);
    free(phys_path);

    log_info(logger_storage, "## %s - Bloque Lógico Escrito %s:%s - Número de Bloque: %d", 
        query_id, file_name, tag, l_block_num);
    return OP_EXITOSA;
}

// el buffer ya debe tener un espacio de memoria asignado con un malloc(BLOCK_SIZE)
rta_storage storage_read(char* file_name, char* tag, int l_block_num, char* buffer, char* query_id)
{
    usleep(RETARDO_ACCESO_BLOQUE * 1000);
    storage_wait(file_name, tag);
    t_metadata_file* meta = storage_metadata_read(file_name, tag);
    if (!meta)
    {
        storage_signal(file_name, tag);
        log_error(logger_storage, "No se pudo leer metadata para READ en %s:%s", file_name, tag);
        return ERR_INEXISTENCIA;
    }

    if (l_block_num > list_size(meta->blocks))
    {
        storage_signal(file_name, tag);
        log_error(logger_storage, "READ fuera de rango en %s:%s (B = %d)", file_name, tag, l_block_num);
        storage_metadata_destroy(meta);
        return ERR_FUERA_LIMITE;
    }

    int p_block_num = atoi(list_get(meta->blocks, l_block_num));
    storage_signal(file_name, tag);
    storage_metadata_destroy(meta);

    char* phys_path = string_from_format("physical_blocks/block%04d.dat", p_block_num);
    FILE* p_block = fopen(phys_path, "rb");
    fread(buffer, 1, BLOCK_SIZE, p_block);
    fclose(p_block);

    log_info(logger_storage, "## %s - Bloque Lógico Leído %s:%s - Número de Bloque: %d",
        query_id, file_name, tag, l_block_num);
    return OP_EXITOSA;
}

rta_storage storage_commit(char* file, char* tag, char* query_id) {
    storage_wait(file, tag);
    t_metadata_file* metadata = storage_metadata_read(file, tag);
    if (!metadata)
    {
        storage_signal(file, tag);
        log_error(logger_storage, "No se pudo leer metadata para COMMIT en %s:%s", file, tag);
        return ERR_INEXISTENCIA;
    }
    free(metadata->estado);
    metadata->estado = string_duplicate("COMMITED");
    storage_metadata_write(file, tag, metadata);

    // check hashes
    int cant_blks = list_size(metadata->blocks);
    for (int i = 0; i < cant_blks; i++) {

        char* log_path = string_from_format("files/%s/%s/logical_blocks/%05d.dat", file, tag, i);
        char* hash = get_hash_from_block(file, tag, i);

        int actual_blk = atoi(list_get(metadata->blocks, i));

        char* phys_blk = block_asocied_to(hash);
        if (!phys_blk) {
            load_hash_in_index(hash, actual_blk);
            continue;
        }
        
        char* phys_path = string_from_format("physical_blocks/%s.dat", phys_blk);

        unlink(log_path);
        link(phys_path, log_path);

        free(phys_path);
        
        log_info(logger_storage, "## %s - %s:%s Se agregó el hard link del bloque lógico %05d al bloque %s",
            query_id, file, tag, i, phys_blk + 5);

        mark_block_free(actual_blk, query_id);
        list_remove_and_destroy_element(metadata->blocks, i, free);
        list_add_in_index(metadata->blocks, i, string_itoa(atoi(phys_blk + 5))); // atoi -> itoa para tener un %d y no un %04d 

        storage_metadata_write(file, tag, metadata);

        log_info(logger_storage, "## %s - %s:%s Bloque Lógico %5d se reasigna de %4d a %s",
            query_id, file, tag, i, actual_blk, phys_blk + 5);
        
        free(hash);
    }

    storage_signal(file, tag);
    storage_metadata_destroy(metadata);
    log_info(logger_storage, "## %s - Commit de File:Tag %s:%s", query_id, file, tag);
    return OP_EXITOSA;
}

rta_storage storage_tag(char* file_o, char* tag_o, char* file_n, char* tag_n, char* query_id) {
    rta_storage result = storage_create(file_n, tag_n, query_id);
    if (result != OP_EXITOSA) {return result;}

    storage_wait(file_o, tag_o);
    t_metadata_file* metadata_ft_o = storage_metadata_read(file_o, tag_o);
    storage_signal(file_o, tag_o);
    if (!metadata_ft_o)
    {
        log_error(logger_storage, "No se pudo leer metadata para TAG en %s:%s", file_o, tag_o);
        return ERR_INEXISTENCIA;
    }
    free(metadata_ft_o->estado);
    metadata_ft_o->estado = string_duplicate("WORK_IN_PROGRESS");
    storage_wait(file_n, tag_n);
    storage_metadata_write(file_n, tag_n, metadata_ft_o);

    int cant_blks = list_size(metadata_ft_o->blocks);
    for (int i = 0; i < cant_blks; i++) {
        char* log_path = string_from_format("files/%s/%s/logical_blocks/%05d.dat", file_n, tag_n, i);
        int blk_num = atoi(list_get(metadata_ft_o->blocks, i));
        char* phys_path = string_from_format("physical_blocks/block%04d.dat", blk_num);
        int r = link(phys_path, log_path);
        free(phys_path);
        free(log_path);
        if (r != 0) { 
            log_error(logger_storage, "no se pudo hacer el link() durante una instruccion TAG");
            return NOT_DEF_ERR;
        } else {
            log_info(logger_storage, 
                "## %s - %s:%s Se agregó el hard link del bloque lógico %05d al bloque %04d",
                query_id, file_n, tag_n, i, blk_num);
        }
    }

    storage_signal(file_n, tag_n);
    storage_metadata_destroy(metadata_ft_o);
    log_info(logger_storage, "## %s - Tag creado %s:%s", query_id, file_n, tag_n);
    return OP_EXITOSA;
}

rta_storage storage_delete(char* file, char* tag, char* query_id) {
    storage_wait(file, tag);
    t_metadata_file* meta = storage_metadata_read(file, tag);
    if (!meta)
    {
        storage_signal(file, tag);
        log_error(logger_storage, "No se pudo leer metadata para DELETE en %s:%s", file, tag);
        return ERR_INEXISTENCIA;
    }
    int cant_blks = list_size(meta->blocks);
    char* log_path = string_from_format("files/%s/%s", file, tag);
    for (int i = 0; i < cant_blks; i++) {
        int blk_num = atoi(list_get(meta->blocks, i));

        char* hash = get_hash_from_block(file, tag, i);
        remove_hash(hash);
        free(hash);

        char* log_block = string_from_format("%s/logical_blocks/%05d.dat", log_path, i);
        char* phys_path = string_from_format("physical_blocks/block%04d.dat", blk_num);

        unlink(log_block);
        free(log_block);

        struct stat blk_stat;
        stat(phys_path, &blk_stat);
        free(phys_path);
        
        if (blk_stat.st_nlink < 2) {
            mark_block_free(blk_num, query_id);
        }
    }
    storage_metadata_destroy(meta);

    char* meta_conf = string_from_format("%s/metadata.config", log_path);
    remove(meta_conf);
    free(meta_conf);

    char* log_blks = string_from_format("%s/logical_blocks", log_path);
    rmdir(log_blks);
    free(log_blks);
    
    rmdir(log_path);
    free(log_path);

    remove_lock(file, tag);
    storage_signal(file, tag);

    log_info(logger_storage, "## %s - Tag Eliminado %s:%s", query_id, file, tag);

    return OP_EXITOSA;
}