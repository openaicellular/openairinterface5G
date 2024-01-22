#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "task_manager.h"

#define NUM_THREADS 4
#define NUM_JOBS 1024*1024

int64_t time_now_us(void)
{
  struct timespec tms;

  /* The C11 way */
  /* if (! timespec_get(&tms, TIME_UTC))  */

  /* POSIX.1-2008 way */
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

//static _Thread_local int64_t counter = 0;

static
int marker_fd;

void do_work(void* arg)
{
  //int64_t now = time_now_us();

  pair_t* a = (pair_t*)arg;

  naive_fibonnacci(23 + a->a);
 
  usleep(rand()%1024);
  completed_task_ans(a->ans);

  printf("Task completed\n");
  //int64_t stop = time_now_us();

  //char buffer[100] = {0};
  //int ret = snprintf(buffer, 100, "ID %lu Fib elapsed %ld start-stop %ld - %ld \n", pthread_self(),  stop - now, now, stop);
  //assert(ret > 0 && ret < 100);

//  write_marker_ft_mir(marker_fd, buffer);
  // puts(buffer);
}

int main()
{
  task_manager_t man = {0};
  init_task_manager(&man, NUM_THREADS);
  usleep(100);

  pair_t* arr = calloc(NUM_JOBS, sizeof(pair_t));
  assert(arr != NULL);
  task_ans_t* ans = calloc(NUM_JOBS, sizeof(task_ans_t));
  assert(ans != NULL);

  int64_t now = time_now_us();

  for(int i = 0; i < NUM_JOBS; ++i){
      usleep(rand()%1024);
      pair_t* pa = &arr[i]; 
      pa->a = 0; //i%10;
      pa->time = 0;
      pa->ans = &ans[i];
      task_t t = {.args = pa, t.func = do_work};
      async_task_manager(&man, t);
  }

  printf("Waiting %ld \n", time_now_us());
  join_task_ans(ans, NUM_JOBS);
  printf("Done %ld \n", time_now_us());



  free_task_manager(&man, NULL);

  printf("Total elapsed %ld \n", time_now_us() - now);

  free(arr);
  return EXIT_SUCCESS;
}



