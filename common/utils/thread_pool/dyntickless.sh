#!/bin/bash

# Full dyntick CPU on which we'll run the user loop,
# it must be part of nohz_full kernel parameter
TARGET=4

# Migrate all possible tasks to CPU 0
for P in $(ls /proc)
do
	if [ -x "/proc/$P/task/" ]
	then
		echo $P
		taskset -acp 0 $P
	fi
done

# Migrate irqs to CPU 0
for D in $(ls /proc/irq)
do
	if [[ -x "/proc/irq/$D" && $D != "0" ]]
	then
		echo $D
		echo 1 > /proc/irq/$D/smp_affinity
	fi
done

# Delay the annoying vmstat timer far away
sysctl vm.stat_interval=120

# Shutdown nmi watchdog as it uses perf events
sysctl -w kernel.watchdog=0

# Remove -rt task runtime limit
echo -1 > /proc/sys/kernel/sched_rt_runtime_us

# Pin the writeback workqueue to CPU0
echo 1 > /sys/bus/workqueue/devices/writeback/cpumask

DIR=/sys/kernel/debug/tracing
echo > $DIR/trace
echo 0 > $DIR/tracing_on
# Uncomment the below for more details on what disturbs the CPU
echo 0 > $DIR/events/irq/enable
echo 1 > $DIR/events/sched/sched_switch/enable
echo 1 > $DIR/events/workqueue/workqueue_queue_work/enable
echo 1 > $DIR/events/workqueue/workqueue_execute_start/enable
echo 1 > $DIR/events/timer/hrtimer_expire_entry/enable
echo 1 > $DIR/events/timer/tick_stop/enable
echo nop > $DIR/current_tracer
echo 1 > $DIR/tracing_on

# Run a 10 secs user loop on target
taskset -c 3-7 ./a.out 
#sleep 20
#killall a.out

# Checkout the trace in trace.* file
cat /sys/kernel/debug/tracing/per_cpu/cpu4/trace > trace.4 
cat /sys/kernel/debug/tracing/per_cpu/cpu5/trace > trace.5 
cat /sys/kernel/debug/tracing/per_cpu/cpu6/trace > trace.6 
cat /sys/kernel/debug/tracing/per_cpu/cpu7/trace > trace.7 


