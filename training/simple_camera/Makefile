
CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

show_camera: show_camera.o
	$(CC) $(CFLAGS) -o show_camera show_camera.o $(LIBS)

show_camera.o: show_camera.c
