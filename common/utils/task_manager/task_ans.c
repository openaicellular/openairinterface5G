#include "task_ans.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void completed_task_ans(task_ans_t* task)
{
  assert(task != NULL);

  int const task_not_completed = 0;
  assert(atomic_load_explicit(&task->status, memory_order_acquire) == task_not_completed && "Task already finished?");

  atomic_store_explicit(&task->status, 1, memory_order_release);
}


void join_task_ans(task_ans_t* arr, size_t len)
{
  assert(len < INT_MAX);
  assert(arr != NULL);

  // We are believing Fedor
  const struct timespec ns = {0,1};
  uint64_t i = 0;
  int j = len -1;
  for(; j != -1 ; i++){
    for(; j != -1; --j){
      int const task_completed = 1;
      if(atomic_load_explicit(&arr[j].status, memory_order_acquire) != task_completed) 
        break;
      //if(atomic_load_explicit(&arr[j].status, memory_order_seq_cst) != task_completed) 
    }
    if(i % 8 == 0){
      nanosleep(&ns, NULL);
    }
    //sched_yield();
   // pause_or_yield(); 
  }
}



