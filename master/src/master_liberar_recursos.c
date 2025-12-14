#include "master_liberar_recursos.h"

// Private Functions //

void free_query(void* worker_ptr);
void free_worker(void* worker_ptr);

// Public Functions //

void liberar_recursos() {
    log_info(logger_master, "liberando recursos globales");
    log_destroy(logger_master);
    config_destroy(config_master);
    list_destroy_and_destroy_elements(querys_ready, free_query);
    list_destroy_and_destroy_elements(querys_exec, free_query);
    list_destroy_and_destroy_elements(workers, free_worker);
    pthread_mutex_destroy(&mutex_exec);
    pthread_mutex_destroy(&mutex_ready);
    pthread_mutex_destroy(&mutex_workers);
    sem_close(&sem_workers);
    sem_close(&sem_ready);
    sem_close(&sem_check_prior);
}

// Private Functions //

void free_query(void* query_ptr) {
    query_t *query = (query_t*)query_ptr;
    close(query->socket);
    free(query->file);
    free(query);
}

void free_worker(void* worker_ptr) {
    worker_t *worker = (worker_t*)worker_ptr;
    sem_close(&(worker->sem_interrupt));
    close(worker->socket);
    free(worker->id);
    free(worker);
}