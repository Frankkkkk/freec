CC ?= clang
CFLAGS ?= -O2
LDFLAGS +=
CFDEBUG = -g3 -Wall -Wextra -Wunused -Wunused-parameter

SRC = src
MAN = man
EXEC = freec

SRCS = ${SRC}/freec.c

OBJS = ${SRCS:.c=.o}

.PATH: ${SRC}

PREFIX ?= /usr/local
BINDIR = ${PREFIX}/bin
MANDIR = ${PREFIX}/man

all: ${EXEC}

.c.o:
	${CC} ${CFLAGS} -o $@ -c $<

${EXEC}: ${OBJS}
	${CC} ${LDFLAGS} -o ${EXEC} ${OBJS}

install-main: ${EXEC}
	test -d ${DESTDIR}${BINDIR} || mkdir -p ${DESTDIR}${BINDIR}
	install -m755 ${EXEC} ${DESTDIR}${BINDIR}/${EXEC}

install-data: ${MAN}/${EXEC}.1
	test -d ${DESTDIR}${MANDIR}/man1 || mkdir -p ${DESTDIR}${MANDIR}/man1
	install -m644 ${MAN}/${EXEC}.1 ${DESTDIR}${MANDIR}/man1/${EXEC}.1

install: all install-main install-data

debug: ${EXEC}
debug: CC += ${CFDEBUG}

clean:
	rm -rf ${SRC}/*.o ${EXEC}

.PHONY: all clean install install-main install-data
