# compiler: 
CC =	/opt/rh/devtoolset-8/root/usr/bin/g++

# compiler flags:
CFLAGS  = -std=c++14 -pthread -g -Wall
CTESTFLAGS  = -std=c++14 -Wall -pg -pthread -lbenchmark

#Linking Flag
LFLAGS =	-Wall

INCLUDES =	-I /usr/include/boost/include
LIBS = 
                                                                                                                    

# the build target executable:
TARGET = webserver

$(TARGET):	main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o

main.o:	main.cpp ranking.hpp server.hpp exception.hpp
	$(CC) $(INCLUDES) $(CFLAGS) -o main.o -c main.cpp

# Test
test:	test_script.cpp ranking.hpp
	$(CC) $(INCLUDES) test_script.cpp $(CTESTFLAGS) -o test

clean:	
	$(RM) $(TARGET) *.o *~
