CC=
LD=gcc

REGISTRY_DIR=$(HOME)/.lxreg

all:	lxreg.so lxreg_test

lxreg.so:	lxreg.o
	gcc -shared -Wl,-soname,liblxreg.so -o liblxreg.so lxreg.o

lxreg.o: lxreg.c
	gcc -c lxreg.c

lxreg_test: lxreg_test.o
	gcc -o lxreg_test lxreg_test.o -L./ -llxreg

lxreg_test.o:	lxreg_test.c
	gcc -c lxreg_test.c

clean:
	rm -f *.o *.so
