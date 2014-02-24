
CC = gcc
#
#  Define include paths
#
INC = -I ../external/iniparser/src

EXTRA_FLAGS = -O2

#
#  Define compile flags
#
CFLAGS = `pkg-config --cflags opencv` $(INC) $(EXTRA_FLAGS)

DROID_FLAGS = -O3 -mcpu=cortex-a9 -mfpu=neon -ftree-vectorize \
              -mfloat-abi=hard -ffast-math -fsingle-precision-constant
#
#  Define libraries to add
#
LIBS = `pkg-config --libs opencv`  -lpthread ../external/iniparser/libiniparser.a -lX11 -lm


#
#  Object files to compile
#
OBJS = main.o parse_funcs.o image_graphics.o \
       udp_comm.o color_funcs.o write_video.o read_webcam.o find_ball.o \
       track_ball.o arduino-serial-lib.o

#
#  Compile target
#
balltrack: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

#
#  Clean up directory
#
clean:
	rm -f balltrack $(OBJS)

#
#  Object file dependencies
#
main.o:              main.c t456-vision.h
camera_funcs.o:      camera_funcs.c t456-vision.h
parse_funcs.o:             parse_funcs.c t456-vision.h
image_graphics.o:   image_graphics.c t456-vision.h
udp_com.o:           udp_comm.c t456-vision.h
color_funcs.o:       color_funcs.c t456-vision.h
write_video.o:       write_video.c t456-vision.h

read_webcam.o:       read_webcam.c t456-vision.h
find_ball.o:       find_ball.c t456-vision.h
track_ball.o:       track_ball.c t456-vision.h
arduino-serial-lib.o: arduino-serial-lib.c arduino-serial-lib.h


