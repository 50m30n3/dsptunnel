BINARY = dsptunnel
CFLAGS = -Wall -O3
LDFLAGS = -lpthread

.PHONY: all
all: $(BINARY)


dsptunnel: dsptunnel.o tun.o dsp.o input.o output.o


dsptunnel.o: dsptunnel.c dsptunnel.h tun.h dsp.h input.h output.h
input.o: input.c dsptunnel.h input.h
output.o: output.c dsptunnel.h output.h
dsp.o: dsp.c
tun.o: tun.c


.PHONY: clean
clean:
	rm -f $(BINARY) *.o

