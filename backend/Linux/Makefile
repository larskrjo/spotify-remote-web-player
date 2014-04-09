CFLAGS = -std=c99 -Wall -I../../libs/Linux/libspotify/link -I/usr/local/include -I/usr/local/include/apr-1 $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags alsa)
LDFLAGS = -L/usr/local/lib $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs-only-L alsa)
LDLIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs-only-l --libs-only-other alsa) -lspotify -ljansson -lapr-1 -levent -levent_pthreads -lpthread

SOURCES = globals.c alsa-audio.c json.c helper.c audio.c jukebox.c diff.c server.c requests.c playlist-callbacks.c container-callbacks.c sp-callbacks.c ws-callbacks.c main.c

all: server

server: $(SOURCES)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f *.o server
	rm -rf .settings .cache
