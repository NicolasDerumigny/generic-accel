LDFLAGS=-Llib -lcma -lpthread -lm

all: generic_accel.elf


generic_accel.elf: drivers/xgeneric_accel_linux.o drivers/xgeneric_accel.o main.o
	g++ $^ $(LDFLAGS) -o $@

gen_file: prgm.inc


main.o: main.cpp prgm.inc
	g++ $< -c -o $@

drivers/xgeneric_accel_linux.o: drivers/xgeneric_accel_linux.c
	g++ $^  -c -o $@

drivers/xgeneric_accel.o: drivers/xgeneric_accel.c
	g++ $^  -c -o $@

prgm.inc: prgm.S
	./compil $^

clean:
	-rm *.o generic_accel.elf drivers/*.o *.inc
