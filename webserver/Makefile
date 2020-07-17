# compiler: 
CC =	/opt/rh/devtoolset-8/root/usr/bin/g++

# compiler flags:
CFLAGS  = -std=c++14 -pthread -g -Wall -lrt
CTESTFLAGS  = -std=c++14 -Wall -g -pthread -lbenchmark -lrt

#Linking Flag
LFLAGS =	-Wall

INCLUDES =	-I /usr/include/boost/include
LIBS = 
                                                                                                                    
TARGET = webserver

$(TARGET):	main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o

main.o:	main.cpp ranking.hpp server.hpp exception.hpp
	$(CC) $(INCLUDES) $(CFLAGS) -o main.o -c main.cpp

# Test
test: test.cpp
	$(CC) $(INCLUDES) test.cpp $(CTESTFLAGS) -o test

test_basic:	test_basic.cpp ranking.hpp test.h
	$(CC) $(INCLUDES) test_basic.cpp $(CTESTFLAGS) -o test_basic

test_limit:	test_limit.cpp ranking.hpp
	$(CC) $(INCLUDES) test_limit.cpp $(CTESTFLAGS) -pg -o test_limit

test_comp:	test_comp.cpp
	$(CC) $(INCLUDES) test_comp.cpp $(CTESTFLAGS) -o test_comp

clean:	
	$(RM) $(TARGET) *.o *~ *.out test_basic test_limit main
