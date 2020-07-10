# compiler: 
CC =	/opt/rh/devtoolset-8/root/usr/bin/g++

# compiler flags:
CFLAGS  = -std=c++11 -pthread -g -Wall 

#Linking Flag
LFLAGS =	-Wall

INCLUDES =	-I /usr/include/boost/include
LIBS = 
                                                                                                                    

# the build target executable:
TARGET = webserver


$(TARGET):	main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o

main.o:	main.cpp ranking.hpp server.hpp
	$(CC) $(INCLUDES) $(CFLAGS) -o main.o -c main.cpp

clean:	
	$(RM) $(TARGET) *.o *~
