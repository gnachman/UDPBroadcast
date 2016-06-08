all: server client

server: test_server.c listen.c listen.h
	cc test_server.c listen.c -g -O0 -o server

client: test_client.c broadcast.c broadcast.h
	cc test_client.c broadcast.c -g -O0 -o client
