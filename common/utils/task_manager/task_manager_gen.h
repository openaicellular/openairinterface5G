#ifndef TASK_MANAGER_GENERIC_H
#define TASK_MANAGER_GENERIC_H

#include "task.h"
#include "thread_pool/task_manager.h"
#include "threadPool/thread-pool.h"
#include "C-Thread-Pool/thpool.h"


// Uncomment one task_manager_t to compile the 
// desired thread pool

// Work stealing thread pool
 #define task_manager_t ws_task_manager_t
// Previous single queue OAI thread pool 
// #define task_manager_t tpool_t
// Most rated C thread pool in github
//#define task_manager_t threadpool



#define init_task_manager(T, P, N) _Generic ((T), \
                                   ws_task_manager_t*: init_ws_task_manager, \
                                   tpool_t*: init_sq_task_manager,\
                                   threadpool* : init_c_tp, \
                                   default:  init_ws_task_manager) (T, P, N)

#define free_task_manager(T, F) _Generic ((T), \
                                ws_task_manager_t*: free_ws_task_manager, \
                                tpool_t*: free_sq_task_manager,\
                                threadpool* : free_c_tp, \
                                default:  free_ws_task_manager) (T, F)

#define async_task_manager(T, TASK) _Generic ((T), \
                                    ws_task_manager_t*: async_ws_task_manager, \
                                    tpool_t*: async_sq_task_manager,\
                                    threadpool* : async_tp_task_manager, \
                                    default: async_ws_task_manager) (T, TASK)

#endif

