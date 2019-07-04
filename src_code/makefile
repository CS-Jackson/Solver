CC = g++

Solver: main.o http_conn.o 
	g++ -o Solver *.o -lpthread

main.o: main.cpp
	g++ -c main.cpp

http_conn.o: http_conn.cpp http_conn.h
	g++ -c http_conn.cpp 

clean:
	rm *.o
