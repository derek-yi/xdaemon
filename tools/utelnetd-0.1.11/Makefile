# Makefile for utelnetd
# 
# Configure this with the following environment variables:
#

# where to install
INSTDIR		:= /usr/local/bin/

# where to find login program
ifneq ("", "$(BSD)")
LOGIN		:= /usr/bin/login
else
LOGIN		:= /bin/login
endif

ifneq ("", "$(BSD)")
CORE		:= utelnetd.core
else
CORE		:= core
endif

# nothing to configure below this line... ---8<---8<---8<---

PROGS     = utelnetd

INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

OBJS      = utelnetd.o

CC        = $(CROSS_COMPILE)gcc
INSTALL   = install

CFLAGS	 += -I. -pipe -DSHELLPATH=\"$(LOGIN)\" -Wall

ifneq ("","$(DEBUG)")
CFLAGS   += -DDEBUG -g -Os
STRIP	  = \#
else
CFLAGS	 += -fomit-frame-pointer
STRIP	  = $(CROSS_COMPILE)strip 
endif

ifeq ("1", "$(BSD)")
CFLAGS   += -DBSD
endif


all: $(PROGS)

$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LDLIBS) -o $@
	$(STRIP) --remove-section=.comment --remove-section=.note $@

.PHONY: install
install: $(PROGS)
	$(INSTALL) -d $(DESTDIR)$(INSTDIR)
	$(INSTALL) -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) $(PROGS) $(DESTDIR)$(INSTDIR)

.PHONY: clean
clean:
	rm -f $(PROGS) *.o $(CORE)
