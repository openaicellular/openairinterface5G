#ifndef TASK_WORK_STEALING_THREAD_POOL_H
#define TASK_WORK_STEALING_THREAD_POOL_H 


//#ifdef __cplusplus
//extern "C" {
//#endif



typedef struct{
  void* args;
  void (*func)(void* args);
} task_t;

// Compatibility with previous TPool
typedef struct {
  int* core_id;
  int sz;
  int const cap;
} span_core_id_t ;

void parse_num_threads(char const* params, span_core_id_t* out);

//#ifdef __cplusplus
//}
//#endif

#endif

