#ifndef STORAGE_BITMAP_H
#define STORAGE_BITMAP_H

#include <commons/bitarray.h>
#include <pthread.h>
#include "storage_globals.h"

// Variables globales del bitmap 
extern t_bitarray* bitmap;
extern pthread_rwlock_t bitmap_lock; // no se por q carajo no me toma el pthread_rwlock_t si esta incluido el pthread y ni el amigo sabe decirme q mierda pasa
// me dijo un amigo q puede ser q tengo q actualizar algo de las commons??? no se pero bold of u to assume que voy a hacer eso ahora, mi prioridad es dormir pq dormi 2 hs copadoo

// Inicializa el bitmap 
int bitmap_init(const char* mount_point, int blocks_count);

// Libera el bitmap
void bitmap_destroy(void);

// Busca el primer bloque libre, lo marca como usado y devuelve el número
int find_free_block(void);

// Marca un bloque como ocupado
void mark_block_used(int block_num);

// Marca un bloque como libre
void mark_block_free(int block_num);

#endif // STORAGE_BITMAP_H