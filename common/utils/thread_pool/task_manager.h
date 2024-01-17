#ifndef TASK_MANAGER_WORKING_STEALING_H
#define TASK_MANAGER_WORKING_STEALING_H 


#define TASK_MANAGER_UE_DECODING
#define TASK_MANAGER_SIM
#define TASK_MANAGER_LTE

#include "task.h"

#ifndef __cplusplus
#include <stdalign.h>
#include <stdatomic.h>
#else
#include <atomic>
#define _Atomic(X) std::atomic< X >
#define _Alignas(X) alignas(X) 
#endif

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#if defined (__i386__) || defined(__x86_64__)
  #define LEVEL1_DCACHE_LINESIZE 64
#elif __aarch64__
  // This is not always true for ARM 
  // in linux, you can obtain the size at runtime using sysconf (_SC_LEVEL1_DCACHE_LINESIZE) 
  // or from the bash with the command $ getconf LEVEL1_DCACHE_LINESIZE
  // in c++ using std::hardware_destructive_interference_size
  #define LEVEL1_DCACHE_LINESIZE 64
#else
    static_assert(0!=0, "Unknown CPU architecture");
#endif

typedef struct{
  // Avoid false sharing
 _Alignas(LEVEL1_DCACHE_LINESIZE) _Atomic(int) status;
} task_ans_t;

void join_task_ans(task_ans_t* arr, size_t len);

void completed_task_ans(task_ans_t* task); 



typedef struct{
  uint8_t* buf;
  size_t len;
  task_ans_t* ans;
} thread_info_tm_t;

typedef struct{

  pthread_t* t_arr;
  size_t len_thr;
  
  _Atomic(uint64_t) index;

  void* q_arr;

  _Atomic(uint64_t) num_task;

  pthread_barrier_t barrier;

} task_manager_t;

void init_task_manager(task_manager_t* man, size_t num_threads);

void free_task_manager(task_manager_t* man, void (*clean)(task_t* args) );

void async_task_manager(task_manager_t* man, task_t t);

// Compatibility with previous TPool
int parse_num_threads(char const* params);

#endif

