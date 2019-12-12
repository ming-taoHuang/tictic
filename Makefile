all: client server

client: client.c
	gcc client_v3.c -o client

server: server.c
	gcc -pthread server_v4.c -o server

clean:
	rm -rf client server
