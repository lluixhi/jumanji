# See LICENSE file for license and copyright information
# jumanji make config

VERSION = 0.0.0

# paths
PREFIX ?= /usr
MANPREFIX ?= ${PREFIX}/share/man

# libs
GTK_INC ?= $(shell pkg-config --cflags gtk+-3.0)
GTK_LIB ?= $(shell pkg-config --libs   gtk+-3.0)

WEBKIT_INC ?= $(shell pkg-config --cflags webkitgtk-3.0)
WEBKIT_LIB ?= $(shell pkg-config --libs   webkitgtk-3.0)

GIRARA_INC ?= $(shell pkg-config --cflags girara-gtk3)
GIRARA_LIB ?= $(shell pkg-config --libs girara-gtk3)

SQLITE_INC ?= $(shell pkg-config --cflags sqlite3)
SQLITE_LIB ?= $(shell pkg-config --libs sqlite3)

# if you want to use gtk2 uncomment the following lines
#GTK_INC ?= $(shell pkg-config --cflags gtk+-2.0)
#GTK_LIB ?= $(shell pkg-config --libs   gtk+-2.0)

#WEBKIT_INC ?= $(shell pkg-config --cflags webkit-1.0)
#WEBKIT_LIB ?= $(shell pkg-config --libs   webkit-1.0)

#GIRARA_INC ?= $(shell pkg-config --cflags girara-gtk2)
#GIRARA_LIB ?= $(shell pkg-config --libs girara-gtk2)

# libs
INCS = ${GTK_INC} ${WEBKIT_INC} ${GIRARA_INC}
LIBS = ${GTK_LIB} ${WEBKIT_LIB} ${GIRARA_LIB} -lpthread

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
