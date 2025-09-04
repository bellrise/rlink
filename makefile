objects := $(patsubst %.c,%.o,$(wildcard src/*.c))
objects := $(filter-out $(patsubst %,src/%.o,$(WITHOUT_LAYERS)),$(objects))
headers := $(wildcard include/rlink/*.h)
cflags 	:= -Wall -Wextra -Iinclude -ggdb -fsanitize=address
libs 	:= -lssl -lcrypto

librlink.a: $(objects)
	$(AR) rs $@ $(objects)

%.o: %.c $(headers)
	$(CC) $(cflags) -c -o $@ $<

clean:
	rm -f $(objects)
