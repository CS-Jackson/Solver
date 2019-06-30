CC = g++

Solver: main.o Util.o threadpool.o locker.o http_conn.o
	g++  -o Solver *.o -lpthread

main.o: main.cpp
	g++ -c main.cpp -o main.o

locker.o: locker.cpp locker.h
	g++ -c locker.cpp -o locker.o

Util.o: Util.cpp Util.h
	g++ -c Util.cpp -o Util.o

http_conn.o: http_conn.cpp http_conn.h
	g++ -c http_conn.cpp -o http_conn.o

threadpool.o: threadpool.cpp threadpool.h
	g++ -c threadpool.cpp -o threadpool.o


clean:
	rm *.o 
