.SILENT:

CC = gcc
CFLAGS = -Wall -ansi -pedantic -g


all: mytar


mytar: mytar.o archive.o listing.o extract.o header.o
	$(CC) $(CFLAGS) $^ -o $@

mytar.o: mytar.c
	$(CC) $(CFLAGS) -c $^

archive.o: archive.c archive.h
	$(CC) $(CFLAGS) -c $^

listing.o: listing.c listing.h
	$(CC) $(CFLAGS) -c $^

extract.o: extract.c extract.h
	$(CC) $(CFLAGS) -c $^

header.o: header.c header.h
	$(CC) $(CFLAGS) -c $^

test:

clean:
	rm -f mytar *.o *.out *.gch
