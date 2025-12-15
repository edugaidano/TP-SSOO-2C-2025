#include "storage_liberar_recursos.h"

// Private Functions //

void free_block_mutex(void* mutex);
void free_file_tags_locks(void* dictionary_locks);
void free_lock(void* lock_prt);

// Public Functions //

void liberar_recursos() {
    log_info(logger_storage, "liberando recursos");
    pthread_mutex_destroy(&bhi_mutex);
    pthread_mutex_destroy(&worker_count_mutex);
    sem_close(&sem_locks_index);
    dictionary_destroy_and_destroy_elements(locks_index, free_file_tags_locks);
    list_destroy_and_destroy_elements(blk_mutex_list, free_block_mutex);
    config_destroy(config_super_block);
    config_destroy(config_hash_index);
    config_destroy(config_storage);
    log_destroy(logger_storage);
}

// Private Functions //

void free_block_mutex(void* mutex) {
    sem_t* sem_mutex = (sem_t*)mutex;
    sem_destroy(sem_mutex);
    free(sem_mutex);
}

void free_file_tags_locks(void* dictionary_locks){
    t_dictionary* d_file_tag = (t_dictionary*)dictionary_locks;
    dictionary_destroy_and_destroy_elements(d_file_tag, free_lock);
}

void free_lock(void* lock_prt) {
    t_lock* lock = (t_lock*)lock_prt;
    sem_destroy(&(lock->sem_file_tag));
    //free(&(lock->sem_file_tag));
    free(lock_prt);
}