objects := $(patsubst %.c,%.o,$(wildcard src/*.c))
objects := $(filter-out $(patsubst %,src/%.o,$(WITHOUT_LAYERS)),$(objects))
headers := $(wildcard include/rlink/*.h)
cflags 	:= -Wall -Wextra -Iinclude
libs 	:= -lssl -lcrypto
test_sources := $(wildcard tests/*_test/test.c)
test_bins := $(patsubst %/test.c,%/test,$(test_sources))

librlink.a: $(objects)
	$(AR) rs $@ $(objects)

%.o: %.c $(headers)
	$(CC) $(cflags) -c -o $@ $<

tests/%_test/test: tests/%_test/test.c tests/_shared/http_test.h librlink.a
	$(CC) $(cflags) -o $@ $^

test: $(test_bins)
	@python3 ./tests/_run

clean:
	rm -f $(objects) $(test_bins)

.PHONY: clean test
