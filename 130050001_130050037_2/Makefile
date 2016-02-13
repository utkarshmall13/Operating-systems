SerCli: server-mp multi-client

server-mp: server-mp.c
	gcc -o server-mp server-mp.c

multi-client: multi-client.c
	gcc -pthread -o multi-client multi-client.c

clean:
	rm -rf multi-client server-mp

distclean: clean
	rm -rf fileGenerator
	rm -rf files

files:
	g++ -o fileGenerator fileGenerator.cpp
	[ -d files ] || mkdir files
	./fileGenerator 1000

all: distclean files SerCli
