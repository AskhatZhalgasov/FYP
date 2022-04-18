CXX=g++-10

all:
	$(CXX) -std=c++20 checkpoint_program.cpp -fcoroutines -pthread -o prototype_sample

debug: 
	$(CXX) -std=c++20 -g checkpoint_program.cpp -fcoroutines -pthread -o debug

clean:
	rm -f alpha_async.data beta_async.data prototype_sample
