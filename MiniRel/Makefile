.PHONY: all compile clean install

MINIREL_HOME = YOUR_WORKING_DIR

TARGET	= ${MINIREL_HOME}/bin
LTARGET	= ${MINIREL_HOME}/lib
#SUBDIRS = bf pf hf am fe parser minirel datagen
SUBDIRS = bf pf hf am fe
ALLDIRS = h ${SUBDIRS}

CC	= gcc
CFLAGS	= -g -ansi -pedantic
MACROS	= "CC=${CC}" "CFLAGS=${CFLAGS}"

all: compile

compile:
	-@for dir in $(SUBDIRS); do (cd $$dir; $(MAKE) ${MACROS} all); done;

install:
	-@for dir in $(SUBDIRS); do (cp $$dir/lib$$dir.a ${LTARGET}); done;
#	cp bf/libbf.a pf/libpf.a hf/libhf.a am/libam.a fe/libfe.a ${LTARGET}

clean:
	-rm ${LTARGET}/*
	-@for dir in $(SUBDIRS); do (cd $$dir; $(MAKE) clean); done;
