
all: libpatcher.dylib

libpatcher.dylib: patcher.c patcher.h config.h yourcode.c Makefile
	@gcc -arch i386 -arch x86_64 -shared -fPIC -o libpatcher.dylib patcher.c yourcode.c

install: libpatcher.dylib
	@echo Installing ...
	@sh install.sh
	@echo Done

clean:
	rm -rf *.dylib *.o *.a
