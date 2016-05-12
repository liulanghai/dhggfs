test:test.o network.o
	gcc test.o network.o -o test -lpthread
test.o:test.c network.h
	gcc -c -g test.c
network.o:network.c network.h	
	gcc -c -g network.c
clean:
	rm *.o
