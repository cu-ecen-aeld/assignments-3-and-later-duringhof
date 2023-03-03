#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg, ...)
// #define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg, ...) printf("threading ERROR: " msg "\n", ##__VA_ARGS__)

void *threadfunc(void *thread_param)
{
    struct thread_data *thread_func_args = (struct thread_data *)thread_param;

    thread_func_args->thread_complete_success = true;

    if (usleep(thread_func_args->wait_ms_obtain * 1000) != 0)
    {
        thread_func_args->thread_complete_success = false;
    }

    if (pthread_mutex_lock(thread_func_args->mutex_lock) != 0)
    {
        thread_func_args->thread_complete_success = true;
    }

    if (usleep(thread_func_args->wait_ms_release * 1000) != 0)
    {
        thread_func_args->thread_complete_success = false;
    }

    if (pthread_mutex_unlock(thread_func_args->mutex_lock) != 0)
    {
        thread_func_args->thread_complete_success = false;
    }

    return thread_param;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    struct thread_data *thread_data_param = (struct thread_data *)malloc(sizeof(struct thread_data));

    thread_data_param->wait_ms_obtain = wait_to_obtain_ms;
    thread_data_param->wait_ms_release = wait_to_release_ms;
    thread_data_param->mutex_lock = mutex;

    if (pthread_create(thread, NULL, &threadfunc,(void *)(thread_data_param)) != 0)
    {
        return false;
    }

    return true;
}
