

###################Makefile#####################


all: argusd argus
	mkfifo userin
	mkfifo userout

servidor: argusd
	./argusd

cliente: argus
	./argus

argusd: argusd.c argus.h
	gcc -o argusd argusd.c

argus: argus.c argus.h
	gcc -o argus argus.c

clean:
	rm -rf *.o userin userout files/ backup/ argus argusd p1 p2 aqui ali
