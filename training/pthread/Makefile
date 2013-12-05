
OBJ = core.o
OBJ2 = main.o sender.o

core: $(OBJ)
	$(CC) -o $@ $(OBJ) -lpthread

core.o: core.c


ptest: $(OBJ2)
	$(CC) -o $@ $(OBJ2) -lpthread

