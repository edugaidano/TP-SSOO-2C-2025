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
int storage_create(const char* file_name, const char* tag)
{
    char path[256];
    char metadata_path[256];
    char logical_blocks_path[256];

    mkdir("FS/files", 0777);

    snprintf(path, sizeof(path), "FS/files/%s", file_name);
    mkdir(path, 0777);

    snprintf(path, sizeof(path), "FS/files/%s/%s", file_name, tag);
    mkdir(path, 0777);

    snprintf(logical_blocks_path, sizeof(logical_blocks_path), "FS/files/%s/%s/logical_blocks", file_name, tag);
    mkdir(logical_blocks_path, 0777);

    snprintf(metadata_path, sizeof(metadata_path), "FS/files/%s/%s/metadata.config", file_name, tag);
    FILE* meta = fopen(metadata_path, "w");
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
    char meta_path[256];
    snprintf(meta_path, sizeof(meta_path), "FS/files/%s/%s/metadata.config", file_name, tag);

    t_config* config = config_create(meta_path);
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
    for (int i = 0; blocks_array[i] != NULL; i++)
    {
        int block_num = atoi(blocks_array[i]);
        list_add(meta->blocks, (void*)(intptr_t)block_num);
        // btw intptr_t convierte un puntero a un entero del mismo tamaño y es de la libreria <stdint.h>
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
    char meta_path[256];
    snprintf(meta_path, sizeof(meta_path), "FS/files/%s/%s/metadata.config", file_name, tag);

    FILE* meta_file = fopen(meta_path, "w");
    if (!meta_file)
    {
        log_error(logger_storage, "No se pudo abrir metadata.config para escribir %s:%s", file_name, tag);
        return;
    }

    // Serializar la lista de bloques en formato [1,2,3]
    char blocks_str[512] = "[";
    for (int i = 0; i < list_size(metadata->blocks); i++)
    {
        int blk = (int)(intptr_t)list_get(metadata->blocks, i);
        char tmp[16];
        sprintf(tmp, "%d", blk);
        strcat(blocks_str, tmp);
        if (i < list_size(metadata->blocks) - 1)
            strcat(blocks_str, ",");
    }
    strcat(blocks_str, "]");

    fprintf(meta_file, "TAMAÑO=%d\nBLOCKS=%s\nESTADO=%s\n",
            metadata->tamanio, blocks_str, metadata->estado);
    fclose(meta_file);

    log_info(logger_storage, "Metadata actualizada %s:%s (Tamaño=%d, Estado=%s, Bloques=%d)",
             file_name, tag, metadata->tamanio, metadata->estado, list_size(metadata->blocks));
}

// Libera la estructura en memoria
void storage_metadata_destroy(t_metadata_file* metadata)
{
    if (!metadata) return;
    list_destroy(metadata->blocks);
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
    int needed_blocks = (new_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    log_info(logger_storage,
             "##TRUNCATE - %s:%s Tamaño actual=%d (%d bloques) -> Nuevo tamaño=%d (%d bloques)",
             file_name, tag, meta->tamanio, current_blocks, new_size, needed_blocks);

    // Si necesita más bloques
    if (needed_blocks > current_blocks)
    {
        int blocks_to_add = needed_blocks - current_blocks;
        for (int i = 0; i < blocks_to_add; i++)
        {
            int free_block = find_free_block();
            if (free_block < 0)
            {
                log_error(logger_storage, "No hay bloques libres disponibles durante TRUNCATE");
                break;
            }

            char phys_path[256];
            snprintf(phys_path, sizeof(phys_path), "FS/physical_blocks/block%04d.dat", free_block);

            FILE* f = fopen(phys_path, "wb");
            if (f)
            {
                char zeros[BLOCK_SIZE];
                memset(zeros, 0, BLOCK_SIZE);
                fwrite(zeros, 1, BLOCK_SIZE, f);
                fclose(f);
            }

            // Crea hardlink lógico
            char logical_path[256];
            snprintf(logical_path, sizeof(logical_path), "FS/files/%s/%s/logical_blocks/%05d.dat",
                     file_name, tag, list_size(meta->blocks));
            link(phys_path, logical_path);

            // Agrega el bloque a metadata
            list_add(meta->blocks, (void*)(intptr_t)free_block);

            log_info(logger_storage,
                     "##TRUNCATE - Nuevo bloque asignado %d para %s:%s",
                     free_block, file_name, tag);
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
            int block_to_free = (int)(intptr_t)list_get(meta->blocks, last_index);

            // Borrar el hardlink lógico
            char logical_path[256];
            snprintf(logical_path, sizeof(logical_path),
                     "FS/files/%s/%s/logical_blocks/%05d.dat",
                     file_name, tag, last_index);
            unlink(logical_path);

            // Liberar del bitmap
            mark_block_free(block_to_free);

            // Quitar de metadata
            list_remove(meta->blocks, last_index);

            log_info(logger_storage,
                     "##TRUNCATE - Bloque liberado %d para %s:%s",
                     block_to_free, file_name, tag);
        }
    }

    meta->tamanio = new_size;
    storage_metadata_write(file_name, tag, meta);

    storage_metadata_destroy(meta);
    return STORAGE_RESULT_OK;
}

int storage_write(const char* file_name, const char* tag, int offset, int size, const void* buffer)
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

    // Calcula bloque inicial y posición interna
    int current_offset = offset;
    int remaining = size;
    const char* data_ptr = (const char*)buffer;

    while (remaining > 0)
    {
        int block_index = current_offset / BLOCK_SIZE;
        int block_offset = current_offset % BLOCK_SIZE;
        int block_num = (int)(intptr_t)list_get(meta->blocks, block_index);
        int to_write = BLOCK_SIZE - block_offset;
        if (to_write > remaining) to_write = remaining;

        // Ruta física del bloque
        char phys_path[256];
        snprintf(phys_path, sizeof(phys_path), "FS/physical_blocks/block%04d.dat", block_num);

        FILE* f = fopen(phys_path, "r+b");
        if (!f)
        {
            log_error(logger_storage, "No se pudo abrir bloque físico %d (%s)", block_num, phys_path);
            storage_metadata_destroy(meta);
            return STORAGE_RESULT_ERROR;
        }

        fseek(f, block_offset, SEEK_SET);
        fwrite(data_ptr, 1, to_write, f);
        fflush(f);
        fclose(f);

        log_info(logger_storage, "WRITE -> bloque %d [%d bytes en offset %d]",
                 block_num, to_write, block_offset);

        current_offset += to_write;
        data_ptr += to_write;
        remaining -= to_write;
    }

    if (offset + size > meta->tamanio)
        meta->tamanio = offset + size;

    storage_metadata_write(file_name, tag, meta);
    storage_metadata_destroy(meta);

    log_info(logger_storage, "WRITE completado en %s:%s (size=%d, offset=%d)", file_name, tag, size, offset);
    return STORAGE_RESULT_OK;
}

int storage_read(const char* file_name, const char* tag, int offset, int size, void* buffer)
{
    t_metadata_file* meta = storage_metadata_read(file_name, tag);
    if (!meta)
    {
        log_error(logger_storage, "No se pudo leer metadata para READ en %s:%s", file_name, tag);
        return STORAGE_RESULT_ERROR;
    }

    if (offset >= meta->tamanio)
    {
        log_warning(logger_storage, "READ fuera de rango en %s:%s (offset=%d, tamanio=%d)",
                    file_name, tag, offset, meta->tamanio);
        storage_metadata_destroy(meta);
        return STORAGE_RESULT_ERROR;
    }

    if (offset + size > meta->tamanio)
        size = meta->tamanio - offset;

    // Calcula bloque inicial y posición
    int current_offset = offset;
    int remaining = size;
    char* data_ptr = (char*)buffer;

    while (remaining > 0)
    {
        int block_index = current_offset / BLOCK_SIZE;
        int block_offset = current_offset % BLOCK_SIZE;
        int block_num = (int)(intptr_t)list_get(meta->blocks, block_index);
        int to_read = BLOCK_SIZE - block_offset;
        if (to_read > remaining) to_read = remaining;

        // Ruta física del bloque
        char phys_path[256];
        snprintf(phys_path, sizeof(phys_path), "FS/physical_blocks/block%04d.dat", block_num);

        FILE* f = fopen(phys_path, "rb");
        if (!f)
        {
            log_error(logger_storage, "No se pudo abrir bloque físico %d para lectura (%s)", block_num, phys_path);
            storage_metadata_destroy(meta);
            return STORAGE_RESULT_ERROR;
        }

        fseek(f, block_offset, SEEK_SET);
        fread(data_ptr, 1, to_read, f);
        fclose(f);

        log_info(logger_storage, "READ <- bloque %d [%d bytes desde offset %d]",
                 block_num, to_read, block_offset);

        current_offset += to_read;
        data_ptr += to_read;
        remaining -= to_read;
    }

    storage_metadata_destroy(meta);
    log_info(logger_storage, "READ completado en %s:%s (offset=%d, size=%d)", file_name, tag, offset, size);
    return STORAGE_RESULT_OK;
}