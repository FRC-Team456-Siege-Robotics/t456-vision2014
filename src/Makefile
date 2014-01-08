
#
#  Define include paths
#
INC = -I ../external/iniparser/src

#
#  Define compile flags
#
CFLAGS = `pkg-config --cflags opencv` $(INC)

#
#  Define libraries to add
#
LIBS = `pkg-config --libs opencv`  -lpthread ../external/iniparser/libiniparser.a -lX11


#
#  Object files to compile
#
OBJS = main.o camera_funcs.o parse_funcs.o detect_target.o \
       udp_comm.o color_funcs.o

#
#  Compile target
#
bayou: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

#
#  Clean up directory
#
clean:
	rm -f bayou $(OBJS)

#
#  Object file dependencies
#
main.o:              main.c
camera_funcs.o:      camera_funcs.c
parse_funcs.o:             parse_funcs.c
detect_target.o:   detect_target.c
udp_com.o:           udp_comm.c
color_funcs.o:       color_funcs.c


