Task Manager
=========================

General
-----------
Task manager is a simple abstraction to handle tasks concurrently. 
It consists of the following 3 functions that can be found task\_manager\_gen header file.

1. init\_task\_manager
  * Initialize the task manager. The first argument is the type, while the second and third argument are a list of the cores where the threads should be pinned to and its size. Only a range within the core id of the machine is valid i.e., min. 0 max. output of the command \$nproc --all. A -1 represents floating threads. There is no guarantee that the underlying thread pool pins the threads to the cores. 
2.  free\_task\_manager
  * Free the resources acquired by init. Terminate the running threads.
3.  async\_task\_manager
  * Asynchronously send a task to the task manager. The second argument is a task that consists of a function pointer
    where the task will run and a void* arg to the function. Similar syntax to c++ std::async.  

The underlying thread pool of the task manager can be selected at compile time uncommenting the desired thread pool in
file task\_man\_gen.h
It is not expected to be utilized by basic users, and thus, more sophisticated methods for selection are not provided.

Joining tasks
-----------

For joining the tasks, a decoupled mechanism is also provided in the file task\_ans.h 
There are two methods:
1. completed\_task\_ans
  * Once the task is finished, it can itself announce that it finished
 2. join\_task\_ans
   * This is a blocking join point. It will wait for all the tasks to be completed before continuing.
