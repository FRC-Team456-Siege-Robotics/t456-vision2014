
all: arduino_test vcontrol_test



OBJ1 = arduino_test.o arduino-serial-lib.o

arduino_test: $(OBJ1)
	$(CC) -o $@ $(OBJ1)

OBJ2 = vcontrol_test.o

vcontrol_test: $(OBJ2)
	$(CC) -o $@ $(OBJ2)

clean:
	rm -f arduino_test vcontrol_test $(OBJ1) $(OBJ2)



arduino-serial-lib.o: arduino-serial-lib.h
