# See LICENSE file for license and copyright information
# jumanji make config

VERSION = 0.0.0

# paths
PREFIX ?= /usr
MANPREFIX ?= ${PREFIX}/share/man

# gtk3
GTK_INC ?= $(shell pkg-config --cflags gtk+-3.0)
GTK_LIB ?= $(shell pkg-config --libs   gtk+-3.0)

WEBKIT_INC ?= $(shell pkg-config --cflags webkitgtk-3.0)
WEBKIT_LIB ?= $(shell pkg-config --libs   webkitgtk-3.0)

GIRARA_INC ?= $(shell pkg-config --cflags girara-gtk3)
GIRARA_LIB ?= $(shell pkg-config --libs   girara-gtk3)

# gtk2
#GTK_INC ?= $(shell pkg-config --cflags gtk+-2.0)
#GTK_LIB ?= $(shell pkg-config --libs   gtk+-2.0)

#WEBKIT_INC ?= $(shell pkg-config --cflags webkit-1.0)
#WEBKIT_LIB ?= $(shell pkg-config --libs   webkit-1.0)

#GIRARA_INC ?= $(shell pkg-config --cflags girara-gtk2)
#GIRARA_LIB ?= $(shell pkg-config --libs   girara-gtk2)


# libs
INCS = -I. ${GTK_INC} ${WEBKIT_INC} ${GIRARA_INC}
LIBS = -lc ${GTK_LIB} ${WEBKIT_LIB} ${GIRARA_LIB} -lpthread

# flags
CFLAGS += -std=c99 -pedantic -Wall -Wno-format-zero-length $(INCS)

# debug
DFLAGS = -O0 -g

# compiler
CC ?= gcc

# strip
SFLAGS ?= -s

# possible values are: sqlite and plain
DATABASE ?= plain
