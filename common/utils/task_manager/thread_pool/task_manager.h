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

