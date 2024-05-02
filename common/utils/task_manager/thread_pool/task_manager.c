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

#define _GNU_SOURCE
#include <unistd.h>

#include "task_manager.h"

#include <assert.h> 
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

#include <linux/futex.h>      /* Definition of FUTEX_* constants */
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>

#include <ctype.h> // toupper

//#define POLL_AND_SLEEP 

static
void pin_thread_to_core(int core_num)
{
  cpu_set_t set = {0};
  CPU_ZERO(&set);
  CPU_SET(core_num, &set);
  int ret = sched_setaffinity(gettid(), sizeof(set), &set);
  assert(ret != -1); 
  printf("Pining into core %d id %ld \n", core_num, pthread_self());
}

//////////////////////////////
//////////////////////////////
////////// RING //
//////////////////////////////
//////////////////////////////
//////////////////////////////

// For working correctly, maintain the default elements to a 2^N e.g., 2^5=32
#define DEFAULT_ELM 256

typedef struct seq_ring_buf_s
{
  task_t* array;
  size_t cap;
  uint32_t head;
  uint32_t tail;
  _Atomic uint64_t sz;
} seq_ring_task_t;

typedef void (*seq_free_func)(task_t*); 

static
size_t size_seq_ring_task(seq_ring_task_t* r)
{
  assert(r != NULL);

  return r->head - r->tail;
}

static
uint32_t mask(uint32_t cap, uint32_t val)
{
  return val & (cap-1);
}

static 
bool full(seq_ring_task_t* r)
{
  return size_seq_ring_task(r) == r->cap -1;
}

static
void enlarge_buffer(seq_ring_task_t* r)
{
  assert(r != NULL);
  assert(full(r));

  const uint32_t factor = 2;
  task_t* tmp_buffer = calloc(r->cap * factor, sizeof(task_t) );
  assert(tmp_buffer != NULL);

  const uint32_t head_pos = mask(r->cap, r->head);
  const uint32_t tail_pos = mask(r->cap, r->tail);

  if(head_pos > tail_pos){
    memcpy(tmp_buffer, r->array + tail_pos , (head_pos-tail_pos)*sizeof(task_t) );
  } else {
    memcpy(tmp_buffer, r->array + tail_pos, (r->cap-tail_pos)*sizeof(task_t));
    memcpy(tmp_buffer + (r->cap-tail_pos), r->array, head_pos*sizeof(task_t));
  }
  r->cap *= factor;
  free(r->array);
  r->array = tmp_buffer;
  r->tail = 0;
  r->head = r->cap/2 - 1;
}

static
void init_seq_ring_task(seq_ring_task_t* r)
{
  assert(r != NULL);
  task_t* tmp_buffer = calloc(DEFAULT_ELM, sizeof(task_t)); 
  assert(tmp_buffer != NULL);
  seq_ring_task_t tmp = {.array = tmp_buffer, .head = 0, .tail = 0, .cap = DEFAULT_ELM};
  memcpy(r, &tmp, sizeof(seq_ring_task_t));
  r->sz = 0;
}

static
void free_seq_ring_task(seq_ring_task_t* r, seq_free_func fp)
{
  assert(r != NULL);
  assert(fp == NULL);
  free(r->array);
}

static
void push_back_seq_ring_task(seq_ring_task_t* r, task_t t)
{
  assert(r != NULL);

  if(full(r))
    enlarge_buffer(r);
  
  const uint32_t pos = mask(r->cap, r->head);
  r->array[pos] = t;
  r->head += 1;
  r->sz += 1;
}

static
task_t pop_seq_ring_task(seq_ring_task_t* r )
{
  assert(r != NULL);
  assert(size_seq_ring_task(r) > 0);

  const uint32_t pos = mask(r->cap, r->tail);
  task_t t = r->array[pos];
  r->tail += 1; 
  r->sz -= 1;
  return t;
}

#undef DEFAULT_ELM 

//////////////////////////////
//////////////////////////////
////////// END RING //
//////////////////////////////
//////////////////////////////
//////////////////////////////



//////////////////////////////
//////////////////////////////
////////// Start Notification Queue //
//////////////////////////////
//////////////////////////////
//////////////////////////////

typedef struct {
  pthread_mutex_t mtx;
  pthread_cond_t cv;
  seq_ring_task_t r;
  size_t t_id;
  size_t idx; // for debugginf
  _Atomic int done;
} not_q_t;

typedef struct{
  task_t t;
  bool success;
} ret_try_t;

static
void init_not_q(not_q_t* q, size_t idx, size_t t_id)
{
  assert(q != NULL);
  assert(t_id != 0 && "Invalid thread id");

  q->idx = idx;

  q->done = 0;
  init_seq_ring_task(&q->r);

  pthread_mutexattr_t attr = {0};
#ifdef _DEBUG
  int const rc_mtx = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
  assert(rc_mtx == 0);
#endif
  int rc = pthread_mutex_init(&q->mtx, &attr);
  assert(rc == 0 && "Error while creating the mtx");

  pthread_condattr_t* c_attr = NULL; 
  rc = pthread_cond_init(&q->cv, c_attr);
  assert(rc == 0);

  q->t_id = t_id;
}

static
void free_not_q(not_q_t* q, void (*clean)(task_t*) )
{
  assert(q != NULL);
  assert(q->done == 1);

  free_seq_ring_task(&q->r, clean);

  int rc = pthread_mutex_destroy(&q->mtx);
  assert(rc == 0);

  rc = pthread_cond_destroy(&q->cv);
  assert(rc == 0);
}

static
bool try_push_not_q(not_q_t* q, task_t t)
{
  assert(q != NULL);
  assert(q->done == 0 || q->done ==1);
  assert(t.func != NULL);
  assert(t.args != NULL);

  if(q->t_id == pthread_self() ){
    printf("[TASK_MAN]: Cycle detected. Thread from tpool calling itself. Reentrancy should be forbidden. Most probably a bug \n");
  }

  if(pthread_mutex_trylock(&q->mtx ) != 0)
    return false;

  push_back_seq_ring_task(&q->r, t);

  const size_t sz = size_seq_ring_task(&q->r);
  assert(sz > 0);

  int const rc = pthread_mutex_unlock(&q->mtx);
  assert(rc == 0);

  pthread_cond_signal(&q->cv);

  return true;
}

static
void push_not_q(not_q_t* q, task_t t)
{
  assert(q != NULL);
  assert(q->done == 0 || q->done ==1);
  assert(t.func != NULL);
  
  int const rc = pthread_mutex_lock(&q->mtx);
  assert(rc == 0);

  push_back_seq_ring_task(&q->r, t);

  assert(size_seq_ring_task(&q->r) > 0);

  pthread_mutex_unlock(&q->mtx);

  pthread_cond_signal(&q->cv);
}

static
ret_try_t try_pop_not_q(not_q_t* q)
{
  assert(q != NULL);

  ret_try_t ret = {.success = false}; 

  int rc = pthread_mutex_trylock(&q->mtx);
  assert(rc == 0 || rc == EBUSY);

  if(rc == EBUSY)
    return ret;

  assert(q->done == 0 || q->done ==1);

  size_t sz = size_seq_ring_task(&q->r); 
  if(sz == 0){
    rc = pthread_mutex_unlock(&q->mtx);
    assert(rc == 0);

    return ret;
  }

  assert(sz > 0);
  ret.t = pop_seq_ring_task(&q->r);

  rc = pthread_mutex_unlock(&q->mtx);
  assert(rc == 0);
  ret.success = true; 

  return ret;
}

#ifdef POLL_AND_SLEEP 

static
bool pop_not_q(not_q_t* q, ret_try_t* out)
{
  assert(q != NULL);
  assert(out != NULL);
  assert(q->done == 0 || q->done ==1);

  int rc = pthread_mutex_lock(&q->mtx);
  assert(rc == 0);
  assert(q->done == 0 || q->done ==1);

  // Polling can be tunned using different combination
  // of cnt values. it can also be done using pthread_cond_timedwait
  const struct timespec ns = {0,1};
  int cnt = 0;
  while(size_seq_ring_task(&q->r) == 0 && q->done == 0){

    rc = pthread_mutex_unlock(&q->mtx);
    assert(rc == 0);

    cnt++;
    if(cnt % 64)
    	nanosleep(&ns, NULL);

    int rc = pthread_mutex_lock(&q->mtx);
    assert(rc == 0);

    if(cnt == 4*1024){
      cnt = 0;
      pthread_cond_wait(&q->cv , &q->mtx);
    }
    cnt++;
  }

  assert(q->done == 0 || q->done ==1);
  if(q->done == 1){
    int rc = pthread_mutex_unlock(&q->mtx);
    assert(rc == 0);
    return false;
  }

  out->t = pop_seq_ring_task(&q->r);

  rc = pthread_mutex_unlock(&q->mtx);
  assert(rc == 0);

  return true;
}

#else

static
bool pop_not_q(not_q_t* q, ret_try_t* out)
{
  assert(q != NULL);
  assert(out != NULL);
  assert(q->done == 0 || q->done ==1);

  int rc = pthread_mutex_lock(&q->mtx);
  assert(rc == 0);
  assert(q->done == 0 || q->done ==1);

  while(size_seq_ring_task(&q->r) == 0 && q->done == 0){
    pthread_cond_wait(&q->cv , &q->mtx);
  }

  assert(q->done == 0 || q->done ==1);
  if(q->done == 1){
    int rc = pthread_mutex_unlock(&q->mtx);
    assert(rc == 0);
    return false;
  }

  out->t = pop_seq_ring_task(&q->r);

  rc = pthread_mutex_unlock(&q->mtx);
  assert(rc == 0);

  return true;
}
#endif

static
void done_not_q(not_q_t* q)
{
  assert(q != NULL);

  int rc = pthread_mutex_lock(&q->mtx);
  assert(rc == 0);
  
  q->done = 1;

  rc = pthread_cond_signal(&q->cv);
  assert(rc == 0);

  //long r = syscall(SYS_futex, q->futex, FUTEX_WAKE_PRIVATE, INT_MAX, NULL, NULL, 0);
  //assert(r != -1);
  rc = pthread_mutex_unlock(&q->mtx);
  assert(rc == 0);

//  q->futex++;
}

//////////////////////////////
//////////////////////////////
////////// END Notification Queue //
//////////////////////////////
//////////////////////////////
//////////////////////////////

typedef struct{
  ws_task_manager_t* man;
  int idx;
  int core_id; 
} task_thread_args_t;

// Just for debugging purposes, it is very slow!!!!
//static
//_Atomic int cnt_out = 0;

//static
//_Atomic int cnt_in = 0;

static
void* worker_thread(void* arg)
{
  assert(arg != NULL);

  task_thread_args_t* args = (task_thread_args_t*)arg; 
  int const idx = args->idx;

  ws_task_manager_t* man = args->man;

  uint32_t const len = man->len_thr;
  uint32_t const num_it = 2*(man->len_thr + idx); 

  not_q_t* q_arr = (not_q_t*)man->q_arr;

  init_not_q(&q_arr[idx], idx, pthread_self() );   

  int const logical_cores = get_nprocs_conf();
  assert(logical_cores > 0);
  assert(args->core_id > -2 && args->core_id < logical_cores);
  if(args->core_id != -1)
    pin_thread_to_core(args->core_id);

  // Synchronize all threads
  pthread_barrier_wait(&man->barrier);

  size_t acc_num_task = 0;
  for(;;){
    ret_try_t ret = {.success = false}; 

    for(uint32_t i = idx; i < num_it; ++i){
      ret = try_pop_not_q(&q_arr[i%len]);
      if(ret.success == true)
        break;
    }

    if(ret.success == false){
      man->num_task -= acc_num_task;
      acc_num_task = 0;
      if(pop_not_q(&q_arr[idx], &ret) == false)
        break;
    }
    
    //int64_t now = time_now_us();
    //printf("Calling func \n");
    ret.t.func(ret.t.args); 
    //printf("Returning from func \n");
    //int64_t stop = time_now_us(); 

    acc_num_task += 1; 
    //cnt_out++;
  }

  free(args);
  return NULL;
}

void init_ws_task_manager(ws_task_manager_t* man, int* core_id, size_t num_threads)
{
  assert(man != NULL);
  assert(num_threads > 0 && num_threads < 33 && "Do you have zero or more than 32 processors??");
  
  man->q_arr = calloc(num_threads, sizeof(not_q_t));
  assert(man->q_arr != NULL && "Memory exhausted");

  man->t_arr = calloc(num_threads, sizeof(pthread_t));
  assert(man->t_arr != NULL && "Memory exhausted" );
  man->len_thr = num_threads;

  man->index = 0;

  const pthread_barrierattr_t * barrier_attr = NULL;
  int rc = pthread_barrier_init(&man->barrier, barrier_attr, num_threads + 1);
  assert(rc == 0);

  for(size_t i = 0; i < num_threads; ++i){
    task_thread_args_t* args = malloc(sizeof(task_thread_args_t) ); 
    assert(args != NULL && "Memory exhausted");
    args->idx = i;
    args->man = man;
    args->core_id = core_id[i];

    pthread_attr_t attr = {0};

    int ret = pthread_attr_init(&attr);
    assert(ret == 0);
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    assert(ret == 0);
    ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    assert(ret == 0);
    struct sched_param sparam = {0};
    sparam.sched_priority = 97;
    ret = pthread_attr_setschedparam(&attr, &sparam);

    int rc = pthread_create(&man->t_arr[i], &attr, worker_thread, args);
    if(rc != 0){
      printf("[TASK_MAN]: %s \n", strerror(rc));
      printf("[TASK_MAN]: Could not create the pthread with attributtes, trying without attributes\n" );
      rc = pthread_create(&man->t_arr[i], NULL, worker_thread, args);
      assert(rc == 0 && "Error creating a thread");
    }
  }

  // Syncronize thread pool threads. All the threads started
  pthread_barrier_wait(&man->barrier);
}

void free_ws_task_manager(ws_task_manager_t* man, void (*clean)(task_t*))
{
  not_q_t* q_arr = (not_q_t*)man->q_arr;

  for(uint32_t i = 0; i < man->len_thr; ++i){
    done_not_q(&q_arr[i]);
  }

  for(uint32_t i = 0; i < man->len_thr; ++i){
    int rc = pthread_join(man->t_arr[i], NULL); 
    assert(rc == 0);
  }

  for(uint32_t i = 0; i < man->len_thr; ++i){
    free_not_q(&q_arr[i], clean); 
  }

  int rc = pthread_barrier_destroy(&man->barrier);
  assert(rc == 0);

  free(man->q_arr);

  free(man->t_arr);
}

void async_ws_task_manager(ws_task_manager_t* man, task_t t)
{
  assert(man != NULL);
  assert(man->len_thr > 0);
  assert(t.func != NULL);

  size_t const index = man->index++;
  size_t const len_thr = man->len_thr;

  not_q_t* q_arr = (not_q_t*)man->q_arr;
  //assert(pthread_self() != q_arr[index%len_thr].t_id);

  for(size_t i = 0; i < len_thr ; ++i){
    if(try_push_not_q(&q_arr[(i+index) % len_thr], t)){
      man->num_task +=1;

      //printf("Pushing idx %ld %ld \n",(i+index) % len_thr, time_now_us());

      //  Debbugging purposes
      //cnt_in++;
      //printf(" async_task_manager t_id %ld Tasks in %d %ld num_task %ld idx %ld \n", pthread_self(), cnt_in, time_now_us(), man->num_task, (i+index) % len_thr );
      return;
    }
  }

  push_not_q(&q_arr[index%len_thr], t);

  //printf("Pushing idx %ld %ld \n", index % len_thr, time_now_us());

  man->num_task +=1;

  // Debbugging purposes
  //cnt_in++;
  //printf("t_id %ld Tasks in %d %ld num_takss %ld idx %ld \n", pthread_self(), cnt_in, time_now_us(), man->num_task , index % len_thr );
}

