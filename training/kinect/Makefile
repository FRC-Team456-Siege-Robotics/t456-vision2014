
CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

OBJS = cvmovie.o libfreenect_cv.o

FREENECT_LIB = ../libfreenect/build/lib


cvmovie: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) -L $(FREENECT_LIB) -lfreenect -lfreenect_sync

cvmovie.o: cvmovie.c

libfreenect_cv.o: libfreenect_cv.c
