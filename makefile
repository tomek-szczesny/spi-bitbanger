GCCFLAGS=-fcompare-debug-second -std=gnu++17 -w -O3

SHELL=/bin/bash
PREFIX=/usr/local
EXECS=spi-bitbanger
GCC?=g++
SRC=spi-bitbanger.cpp
LIBS=-pthread
LDLIBS=-lgpiod


none:
	@echo ""
	@echo "Pick one of the options:"
	@echo "make build          - builds the daemon executable"
	@echo "make clean          - cleans build environment"

build:
	${GCC} ${LIBS} ${GCCFLAGS} -o ${EXECS} ${SRC} ${LDLIBS}

clean:
	rm -f ${EXECS}

