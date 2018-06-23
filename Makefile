PLATFORM = $(shell uname)


## Compilation flags
##comment out one or the other
##debugging
CFLAGS = -g
##release
#CFLAGS = -O3 -DNDEBUG
LDFLAGS=

CFLAGS+= -Wall -std=c++11


CC = g++ -std=c++11 -O3 -Wall $(INCLUDEPATH)


PROGS = place_first_years

default: $(PROGS)

place_first_years: place_first_years.o header.o
	$(CC) -o $@ place_first_years.o header.o  $(LDFLAGS)

place_first_years.o: place_first_years.cpp  header.h
	$(CC) -c $(INCLUDEPATH) $(CFLAGS) place_first_years.cpp  -o $@

header.o: header.c header.h
	$(CC) -c $(INCLUDEPATH) $(CFLAGS) header.c -o $@

clean::
	rm *.o
	rm place_first_years
