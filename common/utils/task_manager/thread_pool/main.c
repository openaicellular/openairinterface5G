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

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "task_manager.h"

#define NUM_THREADS 4
#define NUM_JOBS 1024*1024

// To compile:
// gcc main.c task_manager.c ../task_ans.c

int64_t time_now_us(void)
{
  struct timespec tms;

  if (clock_gettime(CLOCK_MONOTONIC_RAW,&tms)) {
    return -1;
  }
  /* seconds, multiplied with 1 million */
  int64_t micros = tms.tv_sec * 1000000;
  /* Add full microseconds */
  int64_t const tv_nsec = tms.tv_nsec;
  micros += tv_nsec/1000;
  /* round up if necessary */
  if (tv_nsec % 1000 >= 500) {
    ++micros;
  }
  return micros;
}

typedef struct{
  int64_t a;
  int64_t time;
  task_ans_t* ans;
} pair_t;


static inline
int64_t naive_fibonnacci(int64_t a)
{
  assert(a < 1000);
  if(a < 2)
    return a;
  
  return naive_fibonnacci(a-1) + naive_fibonnacci(a-2);
}

static
int marker_fd;

void do_work(void* arg)
{
  //int64_t now = time_now_us();

  pair_t* a = (pair_t*)arg;

  naive_fibonnacci(23 + a->a);
 
  //usleep(rand()%1024);
  completed_task_ans(a->ans);

  //printf("Task completed\n");
  //int64_t stop = time_now_us();

  //char buffer[100] = {0};
  //int ret = snprintf(buffer, 100, "ID %lu Fib elapsed %ld start-stop %ld - %ld \n", pthread_self(),  stop - now, now, stop);
  //assert(ret > 0 && ret < 100);

//  write_marker_ft_mir(marker_fd, buffer);
  // puts(buffer);
}

int main()
{
  ws_task_manager_t man = {0};
  int arr_core_id[NUM_THREADS] = {0}; 
  for(int i = 0; i < NUM_THREADS; ++i){
     arr_core_id[i] = -1;
  }
  init_ws_task_manager(&man, arr_core_id, NUM_THREADS);

  pair_t* arr = calloc(NUM_JOBS, sizeof(pair_t));
  assert(arr != NULL);
  task_ans_t* ans = calloc(NUM_JOBS, sizeof(task_ans_t));
  assert(ans != NULL);

  int64_t now = time_now_us();

  for(int i = 0; i < NUM_JOBS; ++i){
      pair_t* pa = &arr[i]; 
      pa->a = 0; //i%10;
      pa->time = 0;
      pa->ans = &ans[i];
      task_t t = {.args = pa, t.func = do_work};
      async_ws_task_manager(&man, t);
  }

  printf("Waiting %ld \n", time_now_us());
  join_task_ans(ans, NUM_JOBS);
  int64_t end = time_now_us();
  printf("Done %ld \n", end);


  free_ws_task_manager(&man, NULL);

  printf("Total elapsed %ld \n",  end - now);

  free(arr);
  free(ans);
  return EXIT_SUCCESS;
}



