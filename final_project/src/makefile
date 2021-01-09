CC = g++
LIBS = -std=c++11 -O3
OBJS = main.o lib.o
project: $(OBJS)
	$(CC) $(OBJS) -o ../bin/project
main.o: main.cpp lib.h
	$(CC) $(LIBS) -c main.cpp
lib.o: lib.cpp lib.h
	$(CC) $(LIBS) -c lib.cpp
clean:
	rm *.o
	rm ../bin/project