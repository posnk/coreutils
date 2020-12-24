#
# 'make depend' uses makedepend to automatically generate dependencies
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#
TARGET=i386-pc-posnk

# define the C compiler to use
CC = @echo " [  CC  ]	" $< ; $(TARGET)-gcc
LD = @echo " [  LD  ]	" $@ ; $(TARGET)-gcc

# define any compile-time flags
CFLAGS = -Wall -g

# define any directories containing header files other than /usr/include
#
INCLUDES = -Iinc

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS =

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname
#   option, something like (this will link in libmylib.so and libm.so:
LIBS =

# define the C source files
MOUNT_OBJS = $(BUILDDIR)mount/mount.o
SYNC_OBJS  = $(BUILDDIR)sync/sync.o
PS_OBJS    = $(BUILDDIR)ps/ps.o $(BUILDDIR)lib/proc.o
DBG_OBJS   = $(BUILDDIR)dbg/dbg.o $(BUILDDIR)lib/proc.o

# define the C object files
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#

all: $(BUILDDIR)bin/mount $(BUILDDIR)bin/sync $(BUILDDIR)bin/ps $(BUILDDIR)bin/dbg

$(BUILDDIR)bin:
	mkdir $(BUILDDIR)bin

$(BUILDDIR)mount:
	mkdir $(BUILDDIR)mount

$(BUILDDIR)sync:
	mkdir $(BUILDDIR)sync

$(BUILDDIR)ps:
	mkdir $(BUILDDIR)ps

$(BUILDDIR)lib:
	mkdir $(BUILDDIR)lib

$(BUILDDIR)dbg:
	mkdir $(BUILDDIR)dbg

$(BUILDDIR)bin/mount: $(BUILDDIR)bin $(BUILDDIR)mount $(MOUNT_OBJS)
	$(LD) $(LFLAGS) $(LIBS) -o $(BUILDDIR)bin/mount $(MOUNT_OBJS)

$(BUILDDIR)bin/sync: $(BUILDDIR)bin $(BUILDDIR)sync $(SYNC_OBJS)
	$(LD) $(LFLAGS) $(LIBS) -o $(BUILDDIR)bin/sync $(SYNC_OBJS)

$(BUILDDIR)bin/ps: $(BUILDDIR)bin $(BUILDDIR)lib $(BUILDDIR)ps $(PS_OBJS)
	$(LD) $(LFLAGS) $(LIBS) -o $(BUILDDIR)bin/ps $(PS_OBJS)

$(BUILDDIR)bin/dbg: $(BUILDDIR)bin $(BUILDDIR)lib $(BUILDDIR)dbg $(DBG_OBJS)
	$(LD) $(LFLAGS) $(LIBS) -o $(BUILDDIR)bin/dbg $(DBG_OBJS)

install: $(BUILDDIR)bin/mount $(BUILDDIR)bin/sync $(BUILDDIR)bin/ps $(BUILDDIR)bin/dbg
	install $(BUILDDIR)bin/mount $(DESTDIR)/sbin/mount
	install $(BUILDDIR)bin/sync $(DESTDIR)/sbin/sync
	install $(BUILDDIR)bin/ps $(DESTDIR)/bin/ps
	install $(BUILDDIR)bin/dbg $(DESTDIR)/bin/dbg

.PHONY: depend clean

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
$(BUILDDIR)%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@
$(BUILDDIR)lib/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) $(BUILDDIR)*.o $(BUILDDIR)*~ $(BUILDDIR)bin/*

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

