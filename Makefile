

###################Makefile#####################


all: servidor cliente

run: servidor
	./servidor

argus: cliente
	./argus

servidor: servidor.c 
	gcc -o servidor servidor.c

cliente: cliente.c
	gcc -o argus cliente.c

clean:
	rm -rf *.o userin userout files/ backup/ argus servidor

