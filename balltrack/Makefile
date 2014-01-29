
CC = gcc
#
#  Define include paths
#
INC = -I ../external/iniparser/src

#
#  Define compile flags
#
CFLAGS = `pkg-config --cflags opencv` $(INC)

DROID_FLAGS = -O3 -mcpu=cortex-a9 -mfpu=neon -ftree-vectorize \
              -mfloat-abi=hard -ffast-math -fsingle-precision-constant
#
#  Define libraries to add
#
LIBS = `pkg-config --libs opencv`  -lpthread ../external/iniparser/libiniparser.a -lX11 -lm


#
#  Object files to compile
#
OBJS = main.o camera_funcs.o parse_funcs.o detect_target.o \
       udp_comm.o color_funcs.o

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
main.o:              main.c
camera_funcs.o:      camera_funcs.c
parse_funcs.o:             parse_funcs.c
detect_target.o:   detect_target.c
udp_com.o:           udp_comm.c
color_funcs.o:       color_funcs.c


