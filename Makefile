

###################Makefile#####################


all: servidor

run: servidor
	./servidor.o

servidor: servidor.c 
	gcc -o servidor.o servidor.c

clean:
	rm -rf *.o fifo index log argus servidor

