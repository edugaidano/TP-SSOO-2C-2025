#ifndef _STORAGE_HASHES_H
#define _STORAGE_HASHES_H

#include "storage_globals.h"
#include <stdio.h>
#include <commons/crypto.h>
#include <commons/string.h>

char* get_hash_from_content(char* data);
char* get_hash_from_block(char* file, char* tag, int l_block_num);
void load_hash_in_index(char* hash, int p_block_num);
bool hash_is_loaded(char* hash);
char* block_asocied_to(char* hash);
void remove_hash(char* hash);

#endif