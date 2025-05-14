.phony: clean all

all:
	make -C arduino_crypt

clean:
	make -C arduino_crypt clean
