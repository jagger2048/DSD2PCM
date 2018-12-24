# dsd2pcm Makefile

VERSION = 0.1
SONAME = 0

OPTFLAGS ?= -Wall -g -O3
CFLAGS = $(OPTFLAGS) -fPIC -c
CC = g++

SRCDIR = .
OBJDIR = ./obj
INCLUDES = .

NAME = dsd2pcm
LIB_NAME = lib$(NAME)

LIB_HEADERS = \
$(SRCDIR)/dsd2pcm.h

LIB_SOURCES = \
$(SRCDIR)/dsd2pcm.c

LIB_OBJS = \
dsd2pcm.o

BIN_HEADERS = \
$(SRCDIR)/noiseshape.h \
$(SRCDIR)/noiseshape.hpp

BIN_SOURCES = \
$(SRCDIR)/main.cpp \
$(SRCDIR)/noiseshape.c

BIN_OBJS = \
main.o \
noiseshape.o

all: $(NAME)

$(NAME): $(LIB_NAME).so.$(VERSION) $(BIN_OBJS)
	$(CC) -o $(NAME) $(BIN_OBJS) $(LDFLAGS) -L . -ldsd2pcm

$(LIB_NAME).so.$(VERSION): $(LIB_OBJS)
	$(CC) -shared -Wl,-soname,$(LIB_NAME).so.$(SONAME) $(LDFLAGS) -o $(LIB_NAME).so.$(VERSION) $(LIB_OBJS)
	ln -s $(LIB_NAME).so.$(VERSION) $(LIB_NAME).so.$(SONAME)
	ln -s $(LIB_NAME).so.$(SONAME) $(LIB_NAME).so

dsd2pcm.o: dsd2pcm.c
	$(CC) -I $(INCLUDES) $(CFLAGS) dsd2pcm.c

main.o: main.cpp
	$(CC) -I $(INCLUDES) $(CFLAGS) main.cpp

noiseshape.o: noiseshape.c
	$(CC) -I $(INCLUDES) $(CFLAGS) noiseshape.c

clean:
	-rm $(LIB_OBJS) $(LIB_NAME)* $(BIN_OBJS) $(NAME)

