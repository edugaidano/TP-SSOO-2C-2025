#ifndef STORAGE_FILES_H
#define STORAGE_FILES_H

#include "storage_bitmap.h"
#include "storage_globals.h"
#include "storage_hashes.h"
#include "utils/op_codes.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <sys/stat.h>

typedef struct {
    int tamanio;
    t_list* blocks;    
    char* estado;
} t_metadata_file;

// Crea File:Tag
rta_storage storage_create(char* file_name, char* tag);

// Metadata
t_metadata_file* storage_metadata_read(const char* file_name, const char* tag);
void storage_metadata_write(const char* file_name, const char* tag, t_metadata_file* metadata);
void storage_metadata_destroy(t_metadata_file* metadata);

// operaciones
rta_storage storage_truncate(const char* file_name, const char* tag, int new_size);
rta_storage storage_write(char* file_name, char* tag, int l_block_num, char* buffer);
rta_storage storage_read(char* file_name, char* tag, int l_block_num, char* buffer);
rta_storage storage_commit(char* file, char* tag);
rta_storage storage_tag(char* file_o, char* tag_o, char* file_n, char* tag_n);
rta_storage storage_delete(char* file, char* tag);

#endif