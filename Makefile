OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

server: libstubs.a fs_server.o
	gcc fs_server.o -g -O0 -L. -lstubs -o server

client-api.a: libstubs.a fs_client-api.o fs_client.o
	gcc fs_client-api.o fs_client.o -g -O0 -L. -lstubs -o client

$(OBJECTS): %.o: %.c ece454_fs.h
	gcc -g -c $< -o $@

libstubs.a: server_stub.o client_stub.o helper.o mybind.o
	ar r libstubs.a server_stub.o client_stub.o helper.o mybind.o

clean:
	rm -rf a.out *.o core client server *.a
