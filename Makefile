all: graceful-terminating-console-application-windows.zip

graceful-terminating-console-application-windows.exe: main.c
	gcc -o $@ -std=gnu99 -pedantic -Os -Wall -m64 -municode main.c
	strip $@

graceful-terminating-console-application-windows.zip: graceful-terminating-console-application-windows.exe
	zip -9 $@ $<

clean:
	rm -f graceful-terminating-console-application-windows.exe graceful-terminating-console-application-windows.zip

.PHONY: all clean
