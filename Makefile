

###################Makefile#####################


all: argusd argus
	mkfifo userin || true
	mkfifo userout || true

servidor: argusd
	./argusd

cliente: argus
	./argus

argusd: argusd.c argus.h
	gcc -o argusd argusd.c

argus: argus.c argus.h
	gcc -o argus argus.c

clean:
	pkill argusd || true
	pkill argus || true
	rm -rf *.o userin userout files/ backup/ argus argusd p1 p2 aqui ali
