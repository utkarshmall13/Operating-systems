all:	get-one-file	get-one-file-sig	server-slow

get-one-file:	get-one-file.c
	gcc get-one-file.c -o get-one-file

get-one-file-sig:	get-one-file-sig.c
	gcc get-one-file-sig.c -o get-one-file-sig

server-slow:	server-slow.c
	gcc server-slow.c -o server-slow
	
clean:
	rm -rf get-one-file	get-one-file-sig server-slow
