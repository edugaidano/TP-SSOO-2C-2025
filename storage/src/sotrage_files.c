#include "storage_files.h"
#include "storage_bitmap.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>


// Crea un nuevo File:Tag en el FS
int storage_create(char* file_name, char* tag)
{
    char* path = string_from_format("%s/files/%s", PUNTO_MONTAJE, file_name);
    mkdir(path, 0777);
    string_append_with_format(&path, "/%s", tag);
    int result_mkdir = mkdir(path, 0777);
    int saved_errno = errno;
    if (result_mkdir != 0 && saved_errno == EEXIST) {
        log_error(logger_storage, "Ya existe el file tag");
        /* ERROR de preexistencia del file:tag*/
        return STORAGE_RESULT_ERROR;
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
        return STORAGE_RESULT_ERROR;
    }

    fprintf(meta, "TAMAÑO=0\nBLOCKS=[]\nESTADO=WORK_IN_PROGRESS\n");
    fclose(meta);

    log_info(logger_storage, "##CREATE - File:%s Tag:%s creado correctamente", file_name, tag);
    return STORAGE_RESULT_OK;
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

int storage_truncate(const char* file_name, const char* tag, int new_size)
{
    t_metadata_file* meta = storage_metadata_read(file_name, tag);
    if (!meta)
    {
        log_error(logger_storage, "No se pudo leer metadata de %s:%s para TRUNCATE", file_name, tag);
        return STORAGE_RESULT_ERROR;
    }

    // Calcular bloques 
    int current_blocks = list_size(meta->blocks);
    int needed_blocks = new_size / BLOCK_SIZE;

    log_info(logger_storage,
             "##TRUNCATE - %s:%s Tamaño actual=%d (%d bloques) -> Nuevo tamaño=%d (%d bloques)",
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

            // Borrar el hardlink lógico
            char* path = string_from_format("%s/%05d.dat", logical_path, last_index);
            unlink(path);

            // Liberar del bitmap
            if (block_to_free != 0) {
                mark_block_free(block_to_free);
            }

            // Quitar de metadata
            free(list_remove(meta->blocks, last_index));

            log_info(logger_storage, "##TRUNCATE - Bloque liberado %d para %s:%s", block_to_free, file_name, tag);
        }
    }

    meta->tamanio = new_size;
    storage_metadata_write(file_name, tag, meta);

    storage_metadata_destroy(meta);
    return STORAGE_RESULT_OK;
}

int storage_write(char* file_name, char* tag, int l_block_num, char* buffer)
{
    t_metadata_file* meta = storage_metadata_read(file_name, tag);
    if (!meta)
    {
        log_error(logger_storage, "No se pudo leer metadata para WRITE en %s:%s", file_name, tag);
        return STORAGE_RESULT_ERROR;
    }

    // Valid que haya bloques asignados
    if (list_size(meta->blocks) == 0)
    {
        log_error(logger_storage, "WRITE falló: no hay bloques asignados en %s:%s", file_name, tag);
        storage_metadata_destroy(meta);
        return STORAGE_RESULT_ERROR;
    }

    int p_block_num = atoi(list_get(meta->blocks, l_block_num));
    
    char* phys_path = string_from_format("physical_blocks/block%04d.dat", p_block_num);
    FILE* p_block = fopen(phys_path, "rb");
    free(phys_path);
    char* act_content = malloc(BLOCK_SIZE);
    fread(act_content, BLOCK_SIZE, 1, p_block);
    fclose(p_block);
    // TODO: Check hash del nuevo y antiguo contenido, check links del bloque != 2 (si mismo y este archivo)

    // Caso lindo: solo se reasigna el bloque fisico y el anterior sigue igual (por ej, el bloque fisico 0 que es para los bloques vacios)
    char* log_path = string_from_format("files/%s/%s/logical_blocks/%05d.dat", file_name, tag, l_block_num);
    unlink(log_path);
    int new_p_block_num = find_free_block();
    list_replace_and_destroy_element(meta->blocks, l_block_num, string_itoa(new_p_block_num), free);

    phys_path = string_from_format("physical_blocks/block%04d.dat", new_p_block_num);
    p_block = fopen(phys_path, "wb");
    fwrite(buffer, BLOCK_SIZE, 1, p_block);
    fclose(p_block);
    link(phys_path, log_path);
    free(phys_path);
    free(log_path);

    storage_metadata_write(file_name, tag, meta);
    storage_metadata_destroy(meta);

    log_info(logger_storage, "WRITE completado en %s:%s (B = %d)", file_name, tag, l_block_num);
    return STORAGE_RESULT_OK;
}

// el buffer ya debe tener un espacio de memoria asignado con un malloc(BLOCK_SIZE)
int storage_read(char* file_name, char* tag, int l_block_num, char* buffer)
{
    t_metadata_file* meta = storage_metadata_read(file_name, tag);
    if (!meta)
    {
        log_error(logger_storage, "No se pudo leer metadata para READ en %s:%s", file_name, tag);
        return STORAGE_RESULT_ERROR;
    }

    if (l_block_num > list_size(meta->blocks))
    {
        log_warning(logger_storage, "READ fuera de rango en %s:%s (B = %d)", file_name, tag, l_block_num);
        storage_metadata_destroy(meta);
        return STORAGE_RESULT_ERROR;
    }

    int p_block_num = atoi(list_get(meta->blocks, l_block_num));
    storage_metadata_destroy(meta);

    char* phys_path = string_from_format("physical_blocks/block%04d.dat", p_block_num);
    FILE* p_block = fopen(phys_path, "rb");
    fread(buffer, 1, BLOCK_SIZE, p_block);
    fclose(p_block);

    log_info(logger_storage, "READ completado en %s:%s (B = %d)", file_name, tag, l_block_num);
    return STORAGE_RESULT_OK;
}