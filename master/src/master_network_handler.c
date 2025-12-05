#include "master_network_handler.h"

void *master_network_handler(void *arg)
{
    int socket_server = *(int *)arg;

    while (1)
    {
        int connection_socket = accept_connection(socket_server, logger_master);
        t_list *list = recv_package(connection_socket, logger_master);
        op_code opcode = get_opcode(list);

        switch (opcode)
        {
        case HANDSHAKE_QUERY_MASTER:
        {
            char *archivo = string_duplicate(list_get(list, 0));
            int prioridad = atoi(list_get(list, 1));

            query_t *query = malloc(sizeof(query_t));
            query->id = asign_query_id();
            query->pc = 0;
            query->file = archivo;
            query->prioridad = prioridad;
            query->socket = connection_socket;
            query->state = READY;
            query->worker = NULL;
            query->interrumpir = false;

            if (string_equals_ignore_case(ALGORITMO_PLANIFICACION, "FIFO"))
            {
                pthread_mutex_unlock(&mutex_ready);
                list_add(querys_ready, query);
                pthread_mutex_unlock(&mutex_ready);
            } 
            else if (string_equals_ignore_case(ALGORITMO_PLANIFICACION, "PRIORIDADES"))
            {
                pthread_mutex_unlock(&mutex_ready);
                int position = list_add_sorted(querys_ready, query, priority_comparator);
                pthread_mutex_unlock(&mutex_ready);

                if (position == 0) {
                    worker_t *worker_libre = buscar_worker_libre();
                    if (worker_libre == NULL && !list_is_empty(workers))
                    {
                       query_t *query_victima = buscar_victima();
                       if (query_victima->prioridad > query->prioridad)
                       {
                           query_victima->worker->interrumpir = true; // Se desalojara cuando verifique la interrupcion
                       }
                    }
                }
            } 
            else
            {
                log_error(logger_master, "El algoritmo de planificacion no esta definido correctamente");
                list_destroy_and_destroy_elements(list, free);
                exit(EXIT_FAILURE);
            }

            send(connection_socket, "ACK", 4, 0);

            sem_post(&sem_ready);

            log_info(logger_master, 
                "## Se conecta un Query Control para ejecutar la Query %s con prioridad %d- Id asignado: %d Nivel multiprocesamiento %d", 
                archivo, prioridad, query->id, list_size(workers));

            pthread_t query_handler_thread;
            pthread_create(&query_handler_thread, NULL, &query_handler, query);
            pthread_detach(query_handler_thread);
            break;
        }
        case HANDSHAKE_WORKER_MASTER:
        {
            char *id = string_duplicate(list_get(list, 0));

            worker_t *worker = malloc(sizeof(worker_t));
            worker->id = id;
            worker->socket = connection_socket;
            worker->state = READY;
            worker->query = NULL;
            worker->interrumpir = false;
            sem_init(&(worker->sem_interrupt), 0, 0);

            pthread_mutex_lock(&mutex_workers);
            list_add(workers, worker);
            pthread_mutex_unlock(&mutex_workers);

            send(connection_socket, "ACK", 4, 0);

            sem_post(&sem_workers);

            log_info(logger_master, "## Se conecta el Worker %s - Cantidad total de Workers: %d", id, list_size(workers));

            pthread_t worker_handler_thread;
            pthread_create(&worker_handler_thread, NULL, &worker_handler, worker);
            pthread_detach(worker_handler_thread);
            break;
        }
        default:
            log_error(logger_master, "opcode no reconocido como handshake");
            break;
        }
        list_destroy_and_destroy_elements(list, free);
    }
}

void *query_handler(void *arg)
{
    query_t *query = (query_t*)arg;
    t_list *list = recv_package(query->socket, logger_master);
    op_code opcode = get_opcode(list);
    if (opcode != DESCONEXION)
    {
        log_error(logger_master, "opcode no identificado en master");
        list_destroy_and_destroy_elements(list, free);
        destruir_query(query);
        return NULL;
    }
    log_info(logger_master, 
        "## Se desconecta un Query Control. Se finaliza la Query %d con prioridad %d. Nivel multiprocesamiento %d", 
        query->id, query->prioridad, list_size(workers));

    switch (query->state)
    {
    case READY:
        log_info(logger_master, "la query no estaba en ningun worker, eliminando query, id: %d", query->id);
        log_info(logger_master, "la query fue eliminada, querys en ready: %d", list_size(querys_ready) - 1);
        break;
    case EXEC:
        log_info(logger_master, "la query: %d estaba en exec, desalojando worker: %s", query->id, query->worker->id);
        query->interrumpir = true;
        sem_t sem_worker = query->worker->sem_interrupt;
        sem_wait(&(sem_worker));
        log_info(logger_master, "la query fue eliminada, querys en exec: %d", list_size(querys_exec) - 1);
        break;
    case FINISHED:
        log_info(logger_master, "la query %d notifica de su finalizacion, liberando recursos", query->id);
        break;
    default:
        log_warning(logger_master, "la query %d tenia su estado mal definido (%d), liberando recursos", query->id, query->state);
        break;
    }
    destruir_query(query);
    list_destroy_and_destroy_elements(list, free);

    return NULL;
}

void *worker_handler(void *arg)
{
    worker_t *worker = arg;
    worker->error_at_exec = false;
    int bkp_query_id;
    int bkp_query_prioridad;
    while (true)
    {
        t_list *package = recv_package(worker->socket, logger_master);
        op_code opcode = get_opcode(package);

        switch (opcode)
        {
        case CONSULTA_INTERRUPCION: 
        {
            bool result = worker->interrumpir || worker->query->interrumpir || worker->error_at_exec;
            
            if (send(worker->socket, &result, sizeof(bool), 0) <= 0) 
            {
                log_error(logger_master, "Error al enviar interrupción al Worker %s", worker->id);
                list_destroy_and_destroy_elements(package, free);
                destruir_worker(worker);
                return NULL;
            }

            if (worker->interrumpir) // Interrupcion del master (algoritmo de desalojo)
            {
                log_info(logger_master, "## Se desaloja la Query %d(%d) del Worker %s - Motivo: PRIORIDAD", 
                    worker->query->id, worker->query->prioridad, worker->id);
                
                int new_pc = *(int*) list_get(package, 0);
                worker->query->pc = new_pc;

                pthread_mutex_unlock(&mutex_ready);
                list_add_sorted(querys_ready, worker->query, priority_comparator);
                sem_post(&sem_ready);
                pthread_mutex_unlock(&mutex_ready);
            }

            if (worker->query->interrumpir && !worker->error_at_exec) // Interrupcion por desconexion de query_control
            {
                log_info(logger_master, "## Se desaloja la Query %d(%d) del Worker %s - Motivo: DESCONEXION", 
                    worker->query->id, worker->query->prioridad, worker->id);
                sem_post(&(worker->sem_interrupt));
            }

            if (worker->error_at_exec) {
                log_info(logger_master, "## Se desaloja la Query %d(%d) del Worker %s - Motivo: DESCONEXION", 
                    bkp_query_id, bkp_query_prioridad, worker->id);
                worker->error_at_exec = false;
            }

            if (result)
            {
                liberar_worker(worker);
                sem_post(&sem_workers);
            }

            break;
        }
        case LECTURA_MASTER: 
        {   
            char* aux = list_get(package, 0);
            char* content = list_get(package, 1);
            char** file_tag = string_split(aux, ":");

            notificar_read(worker->query, file_tag[0], file_tag[1], content);
            resultado_t result = OK;
            if (send(worker->socket, &result, sizeof(bool), 0) <= 0) 
            {
                log_error(logger_master, "Error al enviar el resultado de la lectura al Worker %s", worker->id);
                string_array_destroy(file_tag);
                list_destroy_and_destroy_elements(package, free);
                destruir_worker(worker);
                return NULL;
            }

            string_array_destroy(file_tag);

            break;
        }
        case INSTRUCCION_MASTER: // La unica es el EXIT, se podria hacer una verificacion con el contenido
        {
            finalizar_query(worker->query, FINALIZACION_CORRECTA, 0);
            resultado_t result = OK;
            if (send(worker->socket, &result, sizeof(resultado_t), 0) <= 0) 
            {
                log_error(logger_master, "Error al enviar el resultado del exit al Worker %s", worker->id);
                list_destroy_and_destroy_elements(package, free);
                destruir_worker(worker);
                return NULL;
            }
            liberar_worker(worker);
            sem_post(&sem_workers);
            break;
        }
        case DESCONEXION:
        {
            if (worker->state == EXEC)
            {
                finalizar_query(worker->query, ERR_DESC_WORKER, 0);
            }
            else
            {
                log_info(logger_master, 
                    "Se desconecta el Worker %s - No habia una query en ejecucion - Cantidad total de Workers: %d", 
                    worker->id, list_size(workers) - 1);
            }
            destruir_worker(worker);
            list_destroy_and_destroy_elements(package, free);
            return NULL;
        }
        case ERROR_AT_EXEC:
        {
            worker->error_at_exec = true;
            resultado_t c_error = *(resultado_t*)list_get(package, 0);
            bkp_query_id = worker->query->id;
            bkp_query_prioridad = worker->query->prioridad;
            finalizar_query(worker->query, ERR_STORAGE, c_error);
            break;
        }
        default:
            log_error(logger_master, "Se recibio un paquete desconocido o no definido correctamente de parte del woeker %s (%d)", worker->id, opcode);
            break;
        }

        list_destroy_and_destroy_elements(package, free);
    }
    return NULL;
}
