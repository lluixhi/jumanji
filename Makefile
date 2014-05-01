# See LICENSE file for license and copyright information

include config.mk
include common.mk

PROJECT  = jumanji
SOURCE   = $(shell find . -iname "*.c" -a ! -iname "database-*")
OBJECTS  = $(patsubst %.c, %.o,  $(SOURCE))
DOBJECTS = $(patsubst %.c, %.do, $(SOURCE))

ifeq (${DATABASE}, sqlite)
INCS   += $(SQLITE_INC)
LIBS   += $(SQLITE_LIB)
SOURCE += database-sqlite.c
else
ifeq (${DATABASE}, plain)
SOURCE += database-plain.c
endif
endif

all: options ${PROJECT}

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LIBS    = ${LIBS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%.o: %.c
	$(ECHO) CC $<
	@mkdir -p .depend
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

# force recompilation of database.o if the DATABASE has changed
database.o: database-${DATABASE}.o

%.do: %.c
	$(ECHO) CC $<
	@mkdir -p .depend
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${DFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

${OBJECTS}:  config.mk
${DOBJECTS}: config.mk

${PROJECT}: ${OBJECTS}
	$(ECHO) CC -o $@
	$(QUIET)${CC} ${SFLAGS} ${LDFLAGS} -o $@ ${OBJECTS} ${LIBS}

clean:
	$(QUIET)rm -rf ${PROJECT} \
		${OBJECTS} \
		${TARFILE} \
		${TARDIR} \
		${DOBJECTS} \
		${PROJECT}-debug \
		.depend

${PROJECT}-debug: ${DOBJECTS}
	$(ECHO) CC -o $@
	$(QUIET)${CC} ${LDFLAGS} -o $@ ${DOBJECTS} ${LIBS}

debug: ${PROJECT}-debug

valgrind: debug
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes \
		./${PROJECT}-debug

gdb: debug
	cgdb ${PROJECT}-debug

dist: clean
	$(QUIET)tar -czf $(TARFILE) --exclude=.gitignore `git ls-files`

install: all
	$(ECHO) installing executable file
	$(QUIET)mkdir -p ${DESTDIR}${PREFIX}/bin
	$(QUIET)cp -f ${PROJECT} ${DESTDIR}${PREFIX}/bin
	$(QUIET)chmod 755 ${PROJECT} ${DESTDIR}${PREFIX}/bin/${PROJECT}
	$(ECHO) installing manual page
	$(QUIET)mkdir -p ${DESTDIR}${MANPREFIX}/man1
	$(QUIET)sed "s/VERSION/${VERSION}/g" < ${PROJECT}.1 > ${DESTDIR}${MANPREFIX}/man1/${PROJECT}.1
	$(QUIET)chmod 644 ${DESTDIR}${MANPREFIX}/man1/${PROJECT}.1

uninstall:
	$(ECHO) removing executable file
	$(QUIET)rm -f ${DESTDIR}${PREFIX}/bin/${PROJECT}
	$(ECHO) removing manual page
	$(QUIET)rm -f ${DESTDIR}${MANPREFIX}/man1/${PROJECT}.1

-include $(wildcard .depend/*.dep)

.PHONY: all options clean debug valgrind gdb dist install uninstall
