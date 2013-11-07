
CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

OBJS = contours.o

contours: $(OBJS)
	$(CC) $(CFLAGS) -o contours $(OBJS) $(LIBS)

contours.o: contours.c
