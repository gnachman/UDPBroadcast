all: discovery_client discovery_server

libdiscovery: socket.c socket.h libdiscovery.c libdiscovery.h log.h
	cc -c socket.c -o socket.o
	cc -c libdiscovery.c -o libdiscovery.o
	ar rcs libdiscovery.a socket.o libdiscovery.o

discovery_server: discovery_server.c libdiscovery log.h
	cc discovery_server.c libdiscovery.a -g -O0 -o discovery_server

discovery_client: discovery_client.c libdiscovery log.h
	cc discovery_client.c libdiscovery.a -g -O0 -o discovery_client

clean: force
	rm -rf *.dSYM discovery_client discovery_server *.o *.a

force:
