CC = g++

Solver: main.o http_conn.o 
	g++ -g -o Solver *.o -lpthread

main.o: main.cpp
	g++ -g -c main.cpp

http_conn.o: http_conn.cpp http_conn.h
	g++ -g -c http_conn.cpp 

clean:
	rm *.o 
