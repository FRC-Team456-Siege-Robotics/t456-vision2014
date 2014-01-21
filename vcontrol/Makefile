
OBJS = main.o

all: vcontrol auto_stub balltrack_stub


vcontrol: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

auto_stub: auto_stub.o
	$(CC) -o $@ auto_stub.o

balltrack_stub: balltrack_stub.o
	$(CC) -o $@ balltrack_stub.o
