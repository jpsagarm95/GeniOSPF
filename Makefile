all:	process

process:	main.o	helloPacketsHandler.o lsaPacketsHandler.o	shortestPathFinder.o
	gcc -o process main.o helloPacketsHandler.o lsaPacketsHandler.o shortestPathFinder.o -pthread

main.o:	main.c
	gcc -g -c main.c
	
helloPacketsHandler.o:	helloPacketsHandler.c
	gcc -g -c helloPacketsHandler.c

lsaPacketsHandler.o:	lsaPacketsHandler.c
	gcc -g -c lsaPacketsHandler.c

shortestPathFinder.o:	shortestPathFinder.c
	gcc -g -c shortestPathFinder.c

clean:
	rm *.o process