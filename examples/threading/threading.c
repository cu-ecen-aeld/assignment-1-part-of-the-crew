#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
  struct thread_data* thread_func_args = (struct thread_data *) thread_param;
  //int res = 0;
  const struct timespec tto = {.tv_sec     = thread_func_args->wait_to_obtain_ms / 1000,
                               .tv_nsec = (thread_func_args->wait_to_obtain_ms % 1000) * 1000000};
  const struct timespec ttr = {.tv_sec     = thread_func_args->wait_to_release_ms / 1000,
                               .tv_nsec = (thread_func_args->wait_to_release_ms % 1000) * 1000000};
/*
  msec = thread_func_args->wait_to_obtain_ms;
  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;
*/
  nanosleep(&tto, NULL);
  pthread_mutex_lock(thread_func_args->mutex);
  nanosleep(&ttr, NULL);
  pthread_mutex_unlock(thread_func_args->mutex);
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
  thread_func_args->thread_complete_success = true;
  pthread_exit(thread_param);
  //return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
  int status = 0;
  //printf("dev_thread creating\n");
  struct thread_data* thread_data_p = calloc(1, sizeof (struct thread_data));
  if (NULL == thread_data_p)
  {
    return false;
  }
  thread_data_p->wait_to_obtain_ms = wait_to_obtain_ms;
  thread_data_p->wait_to_release_ms = wait_to_release_ms;
  thread_data_p->mutex = mutex;

  //pthread_attr_init( &dscr_dispatch_p->pthread_attr_dev );
  //pthread_attr_setdetachstate(&dscr_dispatch_p->pthread_attr_dev, PTHREAD_CREATE_DETACHED );
  //status = pthread_create( &dscr_dispatch_p->pthread_dev, &dscr_dispatch_p->pthread_attr_dev, &function_dev, dscr_dispatch_p );
  status = pthread_create( thread, NULL, threadfunc, thread_data_p );
  if (0 != status){
    //printf("can't create thread for dev = %s, status = %d\n", dscr_dispatch_p->dscr_dev_st_p->name, status);
    free(thread_data_p);
    return false;
  } else {
    //printf("dev_thread created\n");
  }

  //pthread_join(*thread, NULL);

  //free(thread_data_p);
  return true;
}

