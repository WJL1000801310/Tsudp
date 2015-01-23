all:server client
server: server.o   MD5.o
client: client.o   MD5.o
	gcc -o server server.o  MD5.o
	gcc -o client client.o  MD5.o

server.o:server.c  MD5.c MD5.h
	gcc -c server.c  MD5.c

client.o:client.c  MD5.c MD5.h
	gcc -c client.c   MD5.c

MD5.o:MD5.c MD5.h
	gcc -c MD5.c

clean:
	rm server client server.o client.o  MD5.o recv.img
