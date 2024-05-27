# Makefile
# USAGE:
# make clean - removes all object files and executables
# make all - compiles the program
# make csim - compiles the program
# make main.o - compiles main.cpp
# make Simulator.o - compiles Simulator.cpp

# Variables
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O # Final Build
#CFLAGS = -std=c++17 -Wall -Wextra -pedantic -O0 -g # Debugging

# Targets
all: csim

csim: main.o Simulator.o
	$(CXX) $(CXXFLAGS) -o csim main.o Simulator.o -lm

main.o: main.cpp Simulator.h
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

Simulator.o: Simulator.cpp Simulator.h
	$(CXX) $(CXXFLAGS) -c Simulator.cpp -o Simulator.o

clean:
	rm -f *.o csim

# TESTING PORTION
# make test_gcc - cleans, compiles, and runs the program with the gcc.trace file
results: clean csim
	./csim 256 4 16 write-allocate write-back fifo < trace/gcc.trace > gcc_trace_output.txt
	./csim 256 4 16 write-allocate write-back fifo < trace/read01.trace > read01_output.txt
	./csim 256 4 16 write-allocate write-back fifo < trace/read02.trace > read02_output.txt
	./csim 256 4 16 write-allocate write-back fifo < trace/read03.trace > read03_output.txt
	./csim 256 4 16 write-allocate write-back fifo < trace/swim.trace > swim_output.txt
	./csim 256 4 16 write-allocate write-back fifo < trace/write01.trace > write01_output.txt
	./csim 256 4 16 write-allocate write-back fifo < trace/write02.trace > write02_output.txt
cache:
	./csim 128 4 16 write-allocate write-back fifo < trace/gcc.trace > cache_1.txt
	./csim 256 4 16 write-allocate write-back fifo < trace/gcc.trace > cache_2.txt
	./csim 512 4 16 write-allocate write-back fifo < trace/gcc.trace > cache_3.txt

associative:
	./csim 256 1 16 write-allocate write-back fifo < trace/gcc.trace > associative_1.txt
	./csim 256 2 16 write-allocate write-back fifo < trace/gcc.trace > associative_2.txt
	./csim 256 4 16 write-allocate write-back fifo < trace/gcc.trace > associative_3.txt
	./csim 256 8 16 write-allocate write-back fifo < trace/gcc.trace > associative_4.txt

block:
	./csim 256 4 8 write-allocate write-back fifo < trace/gcc.trace > block_1.txt
	./csim 256 4 16 write-allocate write-back fifo < trace/gcc.trace > block_2.txt
	./csim 256 4 32 write-allocate write-back fifo < trace/gcc.trace > block_3.txt
	./csim 256 4 64 write-allocate write-back fifo < trace/gcc.trace > block_4.txt

write:
	./csim 256 4 16 write-allocate write-back fifo < trace/gcc.trace > write_1.txt
	./csim 256 4 16 write-allocate write-through fifo < trace/gcc.trace > write_2.txt

allocation:
	./csim 256 4 16 write-allocate write-back fifo < trace/gcc.trace > allocation_1.txt
	./csim 256 4 16 no-write-allocate write-back fifo < trace/gcc.trace > allocation_2.txt

eviction:
	./csim 256 4 16 write-allocate write-back lru < trace/gcc.trace > eviction_1.txt
	./csim 256 4 16 write-allocate write-back fifo < trace/gcc.trace > eviction_2.txt
