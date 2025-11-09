#ifndef STORAGE_FILES_H
#define STORAGE_FILES_H

#include "storage_globals.h"
#include <commons/config.h>
#include <commons/collections/list.h>

typedef struct {
    int tamanio;
    t_list* blocks;    
    char estado[32];
} t_metadata_file;

// Crea File:Tag
int storage_create(const char* file_name, const char* tag);

// Metadata
t_metadata_file* storage_metadata_read(const char* file_name, const char* tag);
void storage_metadata_write(const char* file_name, const char* tag, t_metadata_file* metadata);
void storage_metadata_destroy(t_metadata_file* metadata);

// operaciones
int storage_truncate(const char* file_name, const char* tag, int new_size);
int storage_write(const char* file_name, const char* tag, int offset, int size, const void* buffer);
int storage_read(const char* file_name, const char* tag, int offset, int size, void* buffer);

#endif