build:
	gcc -lcurl ./upnext.c -o ./upnext.so `pkg-config --cflags mpv` -shared -fPIC
