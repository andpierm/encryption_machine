.phony: clean all

all:
	make -C arduino_crypt
	make -C desktop_app

clean:
	make -C arduino_crypt clean
	make -C desktop_app clean
