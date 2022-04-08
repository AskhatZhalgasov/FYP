# Nested coroutine framework 
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
    
    
### Library structure:
- Similarly, `await <=> co_await`, in other words, `co_await` will await until it the corotouine triggered will complete fully. This allows to get similar usage as in the C# library.

Moreover, if we observe, carefully, we can notice some special consequence of coroutines execution (if we adopt similar behavior as `await/async` provides in C#).

Suppose now we have task C and D in the body of task B:
```cpp=
task B {
// point 1
  co_await task C; // point 2
// point 3
  ... 
  co_await task D; // point 4
// point 5
}

task A {
  co_await B;
}
```
The coroutines nested within each other (look at point {num} in the above code):
* Point 1: [A, B]
* Point 2: [A, B, C]
* Point 3: [A, B]
* Point 4: [A, B, D]
* Point 5: [A, B]

The execution happens in the stack-line manner. This allows to get some order, i.e., after task C is finished task A is enabled to run.

## TODO
- [ ] implement synchronization barrier (in the main function) via existing mechanisms (for example, create Task barrier and just co_await the alpha async task).
- [ ] add checkpointing (should be very easy, since the sceleton is constructed)
- [ ] refactor a bit of code
