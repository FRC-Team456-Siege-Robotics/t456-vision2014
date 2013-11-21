
CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

OBJS = main.o

bayou: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

main.o: main.c
