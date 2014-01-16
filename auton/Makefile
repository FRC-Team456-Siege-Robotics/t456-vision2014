
CC = gcc

TARG_OBJS = mini_main.o target_tracking.o target_logic.o target_color.o

SERVER_OBJS = target_server.o 

PARSE_OBJS = parse_config.o

SERVER_SRC = ../external/mongoose/mongoose.c

TAR_FILES = mini_main.c target_tracking.c target_logic.c target_color.c \
              camera_info.h  target_info.h Makefile

PANDA_FLAGS = -mcpu=cortex-a9 -mfpu=neon -ftree-vectorize -fsingle-precision-constant -ffast-math
 
#  Useful compile flags
#  -D DIAG        - print out useful diagnostic information
#  -D FILE_OUTPUT - write target locations out to a file
#  -D GRAPHICS    - display images while processing
#  -D HTTP_SERVER - target message/info via http server :8080
#  -D UDP_SERVER  - target message/info via udp port
#
#FRCFLAGS = -O4  -D GRAPHICS
#FRCFLAGS =  -fPIC -Wall -O6  -D GRAPHICS
FRCFLAGS =  -fPIC -O6  -D GRAPHICS -D HTTP_SERVER

CFLAGS = $(FRCFLAGS) `pkg-config --cflags opencv` 

auton: $(TARG_OBJS) $(SERVER_OBJS) $(PARSE_OBJS)
	$(CC) $(FRCFLAGS) `pkg-config --cflags opencv` \
        -o $@ $(TARG_OBJS) $(SERVER_OBJS) \
        $(SERVER_SRC) $(PARSE_OBJS) \
        -I. -I../external/mongoose \
        `pkg-config --libs opencv` -lm -pthread -ldl \
        -L../external/iniparser -liniparser

clean:
	rm -f $(OBJS) $(TARG_OBJS) $(SERVER_OBJS) $(PARSE_OBJS)

target_tracking.o: target_tracking.c target_info.h camera_info.h

target_logic.o: target_logic.c target_info.h camera_info.h

parse_config.o: parse_config.c target_info.h camera_info.h target_externs.h
	$(CC) -c -W -Wall -I../external/iniparser/src parse_config.c

target_server.o: target_server.c target_info.h camera_info.h
	$(CC) -c -W -Wall -I../external/mongoose -pthread target_server.c

target_color.o: target_color.c

release.tar: 
	tar -cvf $@ $(TAR_FILES)

camera_config: camera_config.c
	$(CC) $(FRCFLAGS) -o $@ `pkg-config --cflags opencv` \
        camera_config.c `pkg-config --libs opencv` -lm

parser: parse_config.c
	$(CC) $(FRCFLAGS) `pkg-config --cflags opencv` \
        -D MAIN \
        -o $@ parse_config.c \
        -I. -I../external/mongoose -I../external/iniparser/src\
        `pkg-config --libs opencv` -lm -pthread -ldl \
        -L../lib -liniparser

camera_capture: camera_capture.c
	$(CC) $(FRCFLAGS) `pkg-config --cflags opencv` \
        -D MAIN \
        -o $@ camera_capture.c $(SERVER_OBJS) $(SERVER_SRC) $(PARSE_OBJS) \
        -I. -I../external/mongoose -I../external/iniparser/src\
        `pkg-config --libs opencv` -lm -pthread -ldl \
        -L../lib -liniparser

