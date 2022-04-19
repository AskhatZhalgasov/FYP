# [Final] Nested coroutine framework 
This is more powerful version of the framework which enabled to create nested coroutines structures.

Look into `nested_program.cpp` for more details. The execution is following
```console
foo@bar:~$ g++-10 -fcoroutines -std=c++17 nested_program.cpp -o prototype_sample
foo@bar:~$ ./prototype_sample
[AlphaAsync] PHASE 1 with thread id: 140269264783104
[BetaAsync] PHASE 1 with thread id: 140269264783104
[ThetaAsync] PHASE 1 with thread id: 140269264783104
[BetaAsync] PHASE 2 with thread id: 140269264783104
[AlphaAsync] PHASE 2 with thread id: 140269264783104
Finished!
```
## Context

After carefully reviewing [library](https://github.com/ljw1004/blog/tree/master/Async/AsyncWorkflow) and learning about async/await pattern in **C#** in the following [article](https://vkontech.com/exploring-the-async-await-state-machine-the-awaitable-pattern/).

### TLDR:
`await` operator it C# awaits the completion of asynchronous call via `GetAwaiter`. 

If its already done in synchronous fashion, then return the result. 
Otherwise, call the current continuation via `OnCompleted` callback.

If we observe, carefully, we can notice some special consequence of execution with `await/async` in C#.

Suppose now we have task C and D in the body of task B:

```C#
task B {
// point 1
  await task C; // point 2
// point 3
  ... 
  await task D; // point 4
// point 5
}

task A {
  await B;
}
```
The async methods nested within each other (look at point {num} in the above code):
* Point 1: [A, B]
* Point 2: [A, B, C]
* Point 3: [A, B]
* Point 4: [A, B, D]
* Point 5: [A, B]

The execution happens in the stack-line manner. This allows to get some order, i.e., only after task C is finished task A is enabled to run.

### Library structure:
For better understanding of the code and better intution on the workflow, the following assumptions (concepts) are implied in the library.

- `await <=> co_await`, in other words, `co_await` will await until it the corotouine triggered will complete fully. 
   
    This allows to get similar usage as in the C# library. Why we need to wait until coroutines completes? See the **TLDR** section above.
- Semantically, each task at any time can be in one of the states, `[Blocked, Running, Finished]`.
- Threadpool only contains `Running` tasks and each thread picks only runnable task.
- Task is added into the threadpool with state `Running` in the initial suspend point.
- Task is labeled as `Finished` when it reaches final_suspend point.
- `co_await some_other_task()` makes current coroutine `Blocked`, so that only `some_other_task()` can be runned. Meanwhile, `some_other_task()` remembers current coroutine as its continuation.
- When coroutine reaches final suspend it not only change label to `Finished`, but also converts the continuation coroutine into the `Running` state and adds it to threadpool.

### Syncronization
`.wait_completion` function which is available is the syncronization method for `Task` structures.

## TODO
- [ ] implement synchronization barrier (in the main function) via existing mechanisms (for example, create Task barrier and just co_await the `AlphaAsync` task).
- [ ] add checkpointing (should be very easy, since the sceleton is constructed)
- [ ] refactor ideas?
