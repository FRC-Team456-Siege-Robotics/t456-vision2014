
CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

filter: filter.o
	$(CC) $(CFLAGS) -o filter filter.o $(LIBS)

filter.o: filter.c
