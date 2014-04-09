CFLAGS = -std=c99 -Wall -I../../libs/OSX/libspotify.framework/link -I/usr/local/include -I/usr/local/include/apr-1
LDFLAGS = -L/usr/local/lib -F../../libs/OSX
LDLIBS = -framework OpenAL -framework libspotify -ljansson -lapr-1 -levent -levent_pthreads

SOURCES = globals.c openal-audio.c json.c helper.c audio.c jukebox.c diff.c server.c requests.c playlist-callbacks.c container-callbacks.c sp-callbacks.c ws-callbacks.c main.c

all: server

server: $(SOURCES)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f *.o server
	rm -rf .settings .cache