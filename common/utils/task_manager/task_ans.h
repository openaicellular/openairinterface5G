#ifndef TASK_ANSWER_THREAD_POOL_H
#define TASK_ANSWER_THREAD_POOL_H  

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
#include <stdalign.h>
#include <stdatomic.h>
#else
#include <atomic>
#define _Atomic(X) std::atomic< X >
#define _Alignas(X) alignas(X) 
#endif

#include <stddef.h>
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

#ifdef __cplusplus
}
#endif

#endif
