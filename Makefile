all: discovery_client discovery_server

discovery_server: discovery_server.c socket.c socket.h
	cc discovery_server.c socket.c -g -O0 -o discovery_server

discovery_client: discovery_client.c socket.c socket.h
	cc discovery_client.c socket.c -g -O0 -o discovery_client

test_server: test_server.c socket.c socket.h
	cc test_server.c socket.c -g -O0 -o test_server

test_client: test_client.c socket.c socket.h
	cc test_client.c socket.c -g -O0 -o test_client

clean: force
	rm -rf *.dSYM test_client test_server discovery_client discovery_server

force:
