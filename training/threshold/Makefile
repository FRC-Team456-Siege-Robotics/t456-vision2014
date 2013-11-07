
CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

OBJS = threshold.o

threshold: $(OBJS)
	$(CC) $(CFLAGS) -o threshold $(OBJS) $(LIBS)

threshold.o: threshold.c
