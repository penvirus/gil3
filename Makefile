BINS += 1a
BINS += 1b
BINS += 2a
BINS += 2b
BINS += 2c
BINS += 3a.so
BINS += 3b
BINS += 4a
BINS += 4b
BINS += 4c
BINS += 6b

HDRS += common.h
#HDRS += entropy.h

CFLAGS := -Wall -O2 -g -ggdb -I/usr/include/python2.7 -Wno-unused-result 
CFLAGS += -DDEBUG
LDFLAGS := -lpthread -lpython2.7

all: $(BINS)


1a: 1a.o
	$(CC) -o $@ $< $(LDFLAGS)
1b: 1b.o
	$(CC) -o $@ $< $(LDFLAGS)
2a: 2a.o
	$(CC) -o $@ $< $(LDFLAGS) -ldl
2b: 2b.o
	$(CC) -o $@ $< $(LDFLAGS) -ldl
2c: 2c.o
	$(CC) -o $@ $< $(LDFLAGS) -ldl
3a.so: 3a.c
	$(CC) -shared -fPIC -o $@ $<
3b: 3b.o 3a.so
	$(CC) -o $@ $< $(LDFLAGS) -ldl
4a: 4a.o
	$(CC) -o $@ $< $(LDFLAGS) -ldl
4b: 4b.o
	$(CC) -o $@ $< $(LDFLAGS) -ldl
4c: 4c.o
	$(CC) -o $@ $< $(LDFLAGS) -ldl
6b: 6b.o
	$(CC) -o $@ $< $(LDFLAGS) -ldl

%.o: %.c $(HDRS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(BINS) *.o *.so

.PHONY: all clean
