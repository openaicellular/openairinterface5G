#ifndef TASK_MANAGER_WORKING_STEALING_H
#define TASK_MANAGER_WORKING_STEALING_H 

#include "../task.h"
#include "../task_ans.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct{
  uint8_t* buf;
  size_t len;
  size_t cap; // capacity
  task_ans_t* ans;
} thread_info_tm_t;

typedef struct{

  pthread_t* t_arr;
  size_t len_thr;
  
  _Atomic(uint64_t) index;

  void* q_arr;

  _Atomic(uint64_t) num_task;

  pthread_barrier_t barrier;

} ws_task_manager_t;

void init_ws_task_manager(ws_task_manager_t* man, int* core_id, size_t num_threads);

void free_ws_task_manager(ws_task_manager_t* man, void (*clean)(task_t* args) );

void async_ws_task_manager(ws_task_manager_t* man, task_t t);

#endif

