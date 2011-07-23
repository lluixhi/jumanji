# See LICENSE file for license and copyright information
# jumanji make config

VERSION = 0.0.0

# paths
PREFIX ?= /usr
MANPREFIX ?= ${PREFIX}/share/man

# gtk3
GTK_INC = $(shell pkg-config --cflags gtk+-3.0 webkitgtk-3.0 girara-gtk3)
GTK_LIB = $(shell pkg-config --libs   gtk+-3.0 webkitgtk-3.0 girara-gtk3)

# gtk2
#GTK_INC = $(shell pkg-config --cflags gtk+-2.0 webkit-1.0 girara-gtk2)
#GTK_LIB = $(shell pkg-config --libs   gtk+-2.0 webkit-1.0 girara-gtk2)

# libs
INCS = -I. -I/usr/include ${GTK_INC}
LIBS = -lc ${GTK_LIB} -lpthread

# flags
CFLAGS += -std=c99 -pedantic -Wall -Wno-format-zero-length $(INCS)

# debug
DFLAGS = -O0 -g

# compiler
CC ?= gcc

# strip
SFLAGS = -s

# possible values are: sqlite and plain
DATABASE ?= plain
#DATABASE ?= sqlite
