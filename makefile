prefix=/usr/local

all:
	gcc ckfile.c -o ckfile
    
install:
	install -m 0755 ckfile $(prefix)/bin

uninstall:
	rm -f $(prefix)/bin/ckfile
