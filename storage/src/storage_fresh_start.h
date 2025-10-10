#ifndef STORAGE_FRESH_START_H
#define STORAGE_FRESH_START_H

#include <commons/log.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern t_log* logger_storage;

// Función principal de creación
void storage_fresh_start(const char* mount_point, int fs_size, int block_size);

#endif