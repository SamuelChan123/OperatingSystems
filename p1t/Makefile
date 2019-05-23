# You can compile with either gcc or g++
CC = g++
# CC = gcc
CFLAGS = -I. -Wall -lm -ldl -no-pie 
#-std=c++98
# disable the -DNDEBUG flag for the printing the freelist
OPTFLAG = -O2

all: deli

deli: deli.cc
	$(CC) -o deli thread.o deli.cc libinterrupt.a $(CFLAGS)

clean:
	rm -f deli

testlib: thread_libinit_test.cc thread.cc
	g++ -o thread_libinit_test.cc thread.cc -I.
