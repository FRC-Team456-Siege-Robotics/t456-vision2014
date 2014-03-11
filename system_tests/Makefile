
all: arduino_test



OBJ1 = arduino_test.o arduino-serial-lib.o

arduino_test: $(OBJ1)
	$(CC) -o $@ $(OBJ1)

clean:
	rm -f arduino_test $(OBJ1)



arduino-serial-lib.o: arduino-serial-lib.h
