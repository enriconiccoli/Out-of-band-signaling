
all: supervisor.out libhash.a myhash.o server.out client.out 

supervisor.out: Supervisor.c libhash.a
	gcc Supervisor.c -o supervisor.out -L. -lhash

libhash.a : myhash.o
	ar rvs libhash.a myhash.o

myhash.o : myHash.c
	gcc myHash.c -c -o myhash.o

server.out : server.c
	gcc server.c -o server.out -lpthread

client.out : client.c
	gcc client.c -o client.out

.PHONY: clean
clean: 
	-rm -f *.out *.o *.a *.txt OOB-server-*

.PHONY: rmvtxt
rmvtxt: -rm -f *.txt

.PHONY: test
test: 
	bash test.sh

.PHONY: misura
misura:
	bash misura.sh


