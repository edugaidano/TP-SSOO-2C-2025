#include <master_keepalive.h>

void *master_keepalive(void *arg)
{
    sem_wait(&sem_workers);
    pthread_t worker_monitor_thread;
    pthread_create(&worker_monitor_thread, NULL, &worker_monitor, NULL);
    // pthread_create(&query_monitor_thread, NULL, &query_monitor, NULL);
    pthread_detach(worker_monitor_thread);
    // pthread_detach(&query_monitor_thread);

    return NULL;
}

void *worker_monitor(void *arg)
{
    sem_post(&sem_workers);
    while (1)
    {
        pthread_mutex_lock(&mutex_workers);
        t_list_iterator *iterator = list_iterator_create(workers);
        int buff[1];

        while (list_iterator_has_next(iterator))
        {
            worker_t *worker = list_iterator_next(iterator);
            pthread_mutex_lock(&worker->mutex);
            int res = recv(worker->fd, buff, sizeof(buff), MSG_PEEK);
            if (!(worker->is_connected) || res == 0)
            {
                list_iterator_remove(iterator);
                if (!worker->is_free)
                {
                    finalizar_query(worker->query, ERR_DESC_WORKER);
                    destruir_query(worker->query);
                }
                destruir_worker(worker);
            }
            pthread_mutex_unlock(&worker->mutex);
        }
        pthread_mutex_unlock(&mutex_workers);
        list_iterator_destroy(iterator);
        usleep(100 * 1000);
    }
    return NULL;
}
