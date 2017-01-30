CC=gcc
CFLAGS=-Wall -Werror -O3 -g -Ilibnet

CPPFLAGS=-Wall -Werror -O2 -g -Ilibnet

LDLIBS_SOLARIS=-lnsl -lsocket
LDLIBS=-lm 
LDFLAGS=-Llibnet

default: bigfile
