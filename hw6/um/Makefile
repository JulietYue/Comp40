CC = gcc

IFLAGS = -I/comp/40/include -I/usr/sup/cii40/include/cii -I.
CFLAGS = -O3 -std=gnu99 -Wall -Wextra -Wfatal-errors -pedantic $(IFLAGS)
LDFLAGS = -g -L/comp/40/lib64 -L/usr/sup/cii40/lib64 
LDLIBS = -l40locality -lnetpbm -lm -lrt -lbitpack -lcii40 

EXECS = um

all: $(EXECS)

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

segments.o: segments.c segments.h
	$(CC) $(CFLAGS) -c $< -o $@

um.o: um.c um.h segments.h
	$(CC) $(CFLAGS) -c $< -o $@

um: um.o segments.o main.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f $(EXECS) *.o
