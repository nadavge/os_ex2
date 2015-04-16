CC=g++
FLAGS=-Wall -std=c++11

TAR=tar
TARFLAGS=cvf
TARNAME=ex2.tar
TARSRCS=Makefile README uthreads.* utils.* thread.* stablePriorityQueue.*

all: libuthreads.a

#======== OBJECTS ==========

uthreads.o: uthreads.cpp
	$(CC) $(FLAGS) -c uthreads.cpp

stablePriorityQueue.o: stablePriorityQueue.cpp
	$(CC) $(FLAGS) -c stablePriorityQueue.cpp

#======== SOURCE ===========

uthreads.cpp: uthreads.h stablePriorityQueue.h thread.h utils.h

stablePriorityQueue.cpp: uthreads.h thread.h stablePriorityQueue.h

libuthreads.a: uthreads.o stablePriorityQueue.o
	ar rcs libuthreads.a uthreads.o stablePriorityQueue.o

#======== MISC =============

tar: 
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

.PHONY: clean tar

clean:
	rm *.o libuthreads.a
