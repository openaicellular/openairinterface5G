/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include "task_ans.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#if defined (__i386__) || defined(__x86_64__)
  #define pause_or_yield  __builtin_ia32_pause
#elif __aarch64__
  #define pause_or_yield() asm volatile("yield" ::: "memory")
#else
    static_assert(0!=0, "Unknown CPU architecture");
#endif

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

  // Spin lock inspired by: 
  // The Art of Writing Efficient Programs: 
  // An advanced programmer's guide to efficient hardware utilization 
  // and compiler optimizations using C++ examples
  // P.S. Should work better than sched_yield() or pause_or_yield()
  const struct timespec ns = {0,1};
  uint64_t i = 0;
  int j = len -1;
  for(; j != -1 ; i++){
    for(; j != -1; --j){
      int const task_completed = 1;
      if(atomic_load_explicit(&arr[j].status, memory_order_acquire) != task_completed) 
        break;
    }
    if(i % 8 == 0){
      nanosleep(&ns, NULL);
    }
  }
}

#undef pause_or_yield

