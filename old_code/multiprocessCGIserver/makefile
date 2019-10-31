CC = g++

Solver: main.o cgiserver.o 
	g++ -g -o Solver *.o -lpthread

main.o: main.cpp
	g++ -g -c main.cpp

cgiserver.o: CGIServer.cpp CGIServer.h
	g++ -g -c CGIServer.cpp 

clean:
	rm *.o 
