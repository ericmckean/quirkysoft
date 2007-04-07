.PHONY: clean all

SUBDIRS:= $(shell ls | egrep -v '^Main')

all:
	@for i in $(SUBDIRS); do if test -e $$i/Makefile ; then $(MAKE) -C $$i ; fi; done;

clean:
	rm -f *.o
	@for i in $(SUBDIRS); do if test -e $$i/Makefile ; then $(MAKE) -C $$i clean ; fi; done;
