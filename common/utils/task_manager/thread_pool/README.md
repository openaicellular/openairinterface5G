Thread Pool implemented in C following the talk of Sean Parent "Better Code: Concurrency" from 2016

dyntickless contains the script to run if low-latency is needed
Remember that isolcpus, rcu_nocbs-4-7, nohz_full=4-7 and rcu_nocb_poll is needed. 
The Kernel needs to be compiled appropiattely 


