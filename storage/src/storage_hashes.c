#include "storage_hashes.h"

// Genera el hash a partir de un string con length = BLOCK_SIZE
char* get_hash_from_content(char* data) {
    return crypto_md5(data, BLOCK_SIZE);
}

// Abre el file/tag/logical/block.dat y genera el hash
char* get_hash_from_block(char* file, char* tag, int l_block_num) {
    char* path = string_from_format("files/%s/%s/logical_blocks/%05d.dat", file, tag, l_block_num);
    FILE* blk = fopen(path, "rb");
    free(path);
    char* data = malloc(BLOCK_SIZE);
    fread(data, BLOCK_SIZE, 1, blk);
    fclose(blk);
    char* hash = get_hash_from_content(data);
    free(data);
    return hash;
}

// Si ya esta en el config, lo actualiza y elimina el anterior bloque
void load_hash_in_index(char* hash, int p_block_num) {
    char* file_name = string_from_format("block%04d", p_block_num);
    remove_hash(hash);
    dictionary_put(config_hash_index->properties, hash, file_name);
    config_save(config_hash_index);
}

bool hash_is_loaded(char* hash) {
    pthread_mutex_lock(&bhi_mutex);
    bool result = config_has_property(config_hash_index, hash);
    pthread_mutex_unlock(&bhi_mutex); 
    return result;
}

char* block_asocied_to(char* hash) {
    char* value;
    if (hash_is_loaded(hash)) {    
        pthread_mutex_lock(&bhi_mutex);
        value = config_get_string_value(config_hash_index, hash);
        pthread_mutex_unlock(&bhi_mutex);
    } else {
        value = NULL;
    }
    return value;
}

void remove_hash(char* hash) {
    if (hash_is_loaded(hash)) {
            pthread_mutex_lock(&bhi_mutex);
            config_remove_key(config_hash_index, hash);
            pthread_mutex_unlock(&bhi_mutex);
    }    
    pthread_mutex_lock(&bhi_mutex);
    config_save(config_hash_index);
    pthread_mutex_unlock(&bhi_mutex);
}