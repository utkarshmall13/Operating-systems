all: server-mt

server-mt: server-mt.c
	g++ -o server-mt  -pthread server-mt.c

clean:
	rm -rf server-mt

distclean: clean
	rm -rf fileGenerator
	rm -rf files

files:
	g++ -o fileGenerator fileGenerator.cpp
	[ -d files ] || mkdir files
	./fileGenerator 1000
