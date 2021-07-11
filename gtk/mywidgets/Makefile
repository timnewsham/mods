
CFLAGS= $(shell gtk-config --cflags) -Wall -g
LIBS= $(shell gtk-config --libs)

OBJ= myknob.o mymodbox.o util.o

KNOBOBJ= test_myknob.o myknob.o util.o
MODOBJ= test_mymodbox.o mymodbox.o util.o

default: libmywidgets.a
all: libmywidgets.a test_myknob test_mymodbox

libmywidgets.a : $(OBJ)
	$(AR) rsv $@ $(OBJ)
	ranlib $@

test_myknob: $(KNOBOBJ)
	$(CC) -o $@ $(KNOBOBJ) $(LIBS)

test_mymodbox: $(MODOBJ)
	$(CC) -o $@ $(MODOBJ) $(LIBS)

clean:
	rm -f $(PROGS) *.o a.out core libmywidgets.a

