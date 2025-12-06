#ifndef STORAGE_BITMAP_H
#define STORAGE_BITMAP_H

#include <commons/bitarray.h>
#include <commons/string.h>
#include <pthread.h>
#include <stdlib.h>
#include "storage_globals.h"

// Variables globales del bitmap 
extern t_bitarray* bitmap;
extern pthread_rwlock_t bitmap_lock;

// Inicializa el bitmap 
int bitmap_init(const char* mount_point, int blocks_count);

// Libera el bitmap
void bitmap_destroy(void);

// Busca el primer bloque libre, lo marca como usado y devuelve el número
int find_free_block(void);

// Marca un bloque como ocupado
void mark_block_used(int block_num);

// Marca un bloque como libre
void mark_block_free(int block_num, char* query_id);

#endif // STORAGE_BITMAP_H