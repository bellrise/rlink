OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c))
HEADERS := $(wildcard include/rlink/*.h)
CFLAGS 	:= -Wall -Wextra -Iinclude -ggdb -fsanitize=address
LIBS 	:= -lssl -lcrypto

tls-curl: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LIBS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS)
