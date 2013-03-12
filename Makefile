CC ?= clang
CFLAGS = -Wall -Wextra
LDFLAGS = 
SRCDIR = src/

OUTPUT = freec

all:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCDIR)freec.c -o $(OUTPUT)
