
all : xtramake synth

C++ = g++
CPPFLAGS = -g -Wall
#CPPFLAGS = -g -Wall -O3

-include Make.dbg
include Make.incl


include gtk/Make.incl
include curses/Make.incl
include linux/Make.incl


SRCS += $(MODS)
OBJS= $(SRCS:.cpp=.o)

xtramake : $(XTRAMAKE)

synth : $(OBJS)
	$(C++) -o $@ $(OBJS) $(LIBS)

clean: $(XTRACLEAN)
	rm -f synth $(OBJS) a.out core
	rm -f *.o

depend: $(SRCS)
	$(C++) -MM $(CPPFLAGS) $^ > .depends

-include .depends
