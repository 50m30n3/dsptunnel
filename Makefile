BINARY = dsptunnel
CFLAGS = -Wall -O3
LDFLAGS = -lpthread

.PHONY: all
all: $(BINARY)


dsptunnel: dsptunnel.o tun.o dsp.o input.o output.o fletcher.o


dsp.o: dsp.c
dsptunnel.o: dsptunnel.c dsptunnel.h tun.h dsp.h input.h output.h
fletcher.o: fletcher.c
input.o: input.c dsptunnel.h fletcher.h input.h
output.o: output.c dsptunnel.h fletcher.h output.h
tun.o: tun.c


.PHONY: clean
clean:
	rm -f $(BINARY) *.o

