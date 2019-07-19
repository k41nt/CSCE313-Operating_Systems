# makefile

all: dataserver client 

#bounded_buffer.o: bounded_buffer.h
#	g++ -std=c++11 -c -g bounded_buffer.h

reqchannel.o: reqchannel.h reqchannel.cpp
	g++ -std=c++11 -c -g reqchannel.cpp

dataserver: dataserver.cpp reqchannel.o 
	g++ -std=c++11 -g -o dataserver dataserver.cpp reqchannel.o -lpthread

client: client.cpp reqchannel.o
	g++ -std=c++11 -g -o client client.cpp bounded_buffer.h reqchannel.o semaphore.h -lpthread

clean:
	rm *.o fifo* dataserver client output*
