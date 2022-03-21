## Sample use case

### Assumption
Suppose the following coroutines:

* secondary_task(n) -> it `co_yield`'s some value n times
* primary_task(n) -> does something internally, use secondary task inside of it.

## Interaction

```cpp
Task secondary_task(int n) {
  co_yield x1;
  ...
  co_yield x2;
  ...
  ...
  co_yield x_n;
  // n times in total
}

Task primary_task() {
  int n = 10;
  auto child_task = secondary_task(some n);
  
  // give up the execution
  // co_await pool.schedule();
   
  // give up the execution and wait for the (intermediate) result 
  auto result_1 = sync_await(child_task);
  ...
  auto result_2 = sync_await(child_task);
  ...
  ...
  auto result_n = sync_await(child_task);
  
  // some calculations
  
  return result;
}
```

## Possible implementation

Assume thread pool runs on top 2 threads.

The following scenario is *possible* execution:

1. Thread_1 executes primary_task.
2. Inside the primary task, it creates secondary task (line 13). 
*Note: child task may or may not be added to the thread pool on the moment of creation, depends on implementatin*.
3. Thread_1 continues execution until it hits first sync_await (line 19).

Up to the point 3, thread_2 may already produced first output for child task (see the *note* of point 2). As of now, I come up with one possible way to implement it.

#### Dependancy graph
* Within thread pool (prototype_exp/threadpool.hpp) we build the dependancy graph. 
* Because task queue is shared, it has protected sections. 
  For example thread_loop (threadpool.hpp#48) or enqueue_task (threadpool.hpp#48) are accessed by at most 1 thread at a time every task can be viewed as node of the graph.
* When primary_task gives up execution (line 19) we add edge secondary_task -> primary_task.
* Ready state is when task has no incoming edges.
* Thread doesn't pick up the task until its in ready state. If it's not in ready state, move it to the back of shared task queue. 
