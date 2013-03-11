CC ?= clang
CFLAGS = -Wall -Wextra
LDFLAGS = 

OUTPUT = freec

all:
	$(CC) $(CFLAGS) $(LDFLAGS) freec.c -o $(OUTPUT)
