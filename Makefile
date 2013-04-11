CC ?= clang
CFLAGS = -Wall -Wextra -g3
LDFLAGS = 
SRCDIR = src/

OUTPUT = freec

all:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCDIR)freec.c -o $(OUTPUT)
