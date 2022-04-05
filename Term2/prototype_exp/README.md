# Library checkpointing library
This is the prototype implementation of checkpointing library. Based on the following [project](https://github.com/ljw1004/blog/tree/master/Async/AsyncWorkflow). 

## DOC:
Run the code: 
```console
foo@bar:~$ g++-10 -fcoroutines -std=c++17 program.cpp
```
### Reference:
Libcoro library: https://github.com/jbaldwin/libcoro

Threadpool article: https://blog.eiler.eu/posts/20210512/

## TODO:
- [x] finished the prototype
- [x] remove namespace std
- [x] memory leak, (need to call handle.destroy() to free memory)
- [ ] come up with use cases
- [ ] hide ```goto``` in the coroutine body 
- [x] decouple library to header file
- [x] integrate thread pool
- [x] add syncronization
- [ ] enable coroutines to accept other coroutines (think how?)  
- [ ] integrate data structure which holds the variables (*e.g.*, template)
- [ ] augment `.data` for each coroutine name
