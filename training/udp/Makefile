
OBJ = sender.o
SERVER_OBJ = server.o

all: sender server

sender: $(OBJ)
	$(CC) -o $@ $(OBJ)


server: $(SERVER_OBJ)
	$(CC) -o $@ $(SERVER_OBJ)

server.o: server.c
sender.o: sender.c
