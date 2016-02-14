shell:	get-one-file-sig	client-shell

all:	get-one-file-sig	server-slow		client-shell

get-one-file-sig:	get-one-file-sig.c
	@gcc get-one-file-sig.c -o get-one-file-sig

server-slow:	server-slow.c
	@gcc server-slow.c -o server-slow

client-shell:	client-shell.cpp
	@g++ -std=c++11 client-shell.cpp -o client-shell
	@echo "Shell executable created"
	
clean:
	@rm -rf client-shell	get-one-file-sig server-slow
