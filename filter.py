#!/usr/bin/python
"""
This is a simple example of a pygtk gui.  The application lets
you view the response curve of several filter types often used
in audio processing.
"""

import pygtk
pygtk.require('2.0')
import gtk
import math

math.pi = 3.14159
MAXVAL = 2.0
GRIDW,GRIDUP = 1000,0.5
XSIZE,YSIZE = 400,300
STEPS = XSIZE
SAMPFREQ = 44100

drawarea = None
filter = None
funcidx = 0
filtertypes = ["Low Shelf", "High Shelf", "Peaking"]

class filt :
	def __init__(self) :
		a1,a2 = None,None
		b0,b1,b2 = None,None,None

	def coefs(self, level, freq) :
		w = 2.0 * math.pi * freq / SAMPFREQ
		cw = math.cos(w)
		sw = math.sin(w)
		A = math.pow(10, level / 40)
		return A,cw,sw
		
	def lowshelf(self, level, locut) :
		A,cw,sw = self.coefs(level, locut)
		beta = math.sqrt(A)
		a0 = (A+1) + (A-1) * cw + beta * sw
		self.b0 = A * ((A+1) - (A-1) * cw + beta * sw) / a0
		self.b1 = 2.0 * A * ((A-1) - (A+1) * cw) / a0
		self.b2 = A * ((A+1) - (A-1) * cw - beta * sw) / a0
		self.a1 = -2.0 * ((A-1) + (A+1) * cw) / a0
		self.a2 = ((A+1) + (A-1) * cw - beta * sw) / a0

	def highshelf(self, level, hicut) :
		A,cw,sw = self.coefs(level, hicut)
		beta = math.sqrt(A)
		a0 = (A+1) + (A-1) * cw + beta * sw
		self.b0 = A * ((A+1) + (A-1) * cw + beta * sw) / a0
		self.b1 = -2.0 * A * ((A-1) + (A+1) * cw) / a0
		self.b2 = A * ((A+1) + (A-1) * cw - beta * sw) / a0
		self.a1 = 2.0 * ((A-1) - (A+1) * cw) / a0
		self.a2 = ((A+1) - (A-1) * cw - beta * sw) / a0
		
	def peaking(self, level, freq) :
		A,cw,sw = self.coefs(level, freq)
		alpha = sw / 2.0
		a0 = 1.0 + alpha / A
		self.b0 = (1.0 + alpha * A) / a0
		self.b1 = (-2.0 * cw) / a0
		self.b2 = (1.0 - alpha * A) / a0
		self.a1 = (-2.0 * cw) / a0
		self.a2 = (1.0 - alpha / A) / a0

def drconfig(w, ev) :
	w,h = w.win.get_size()
	pixmap = gtk.gdk.Pixmap(w.win, w,h)
	pixmap.draw_rectangle(w.get_style().white_gc, gtk.TRUE, 0, 0, w, h)

# make a scale with a label, and return the gui object and the adjustment
def makescale(name, val, min, max, func) :
	box = gtk.HBox()
	box.show()

	l = gtk.Label(name)
	l.show()
	box.pack_start(l, gtk.FALSE, gtk.FALSE, 15)

	adj = gtk.Adjustment(lower=min, upper=max, value=val)
	h = gtk.HScale(adjustment=adj)
	h.show()
	box.pack_start(h)

	adj.connect("value_changed", func)
	return box,adj

def drawgrid(win, gc) :
	XSIZE,YSIZE =win.get_size()
	w = 0.0
	while w < math.pi :
		x = int(w * XSIZE / math.pi)
		win.draw_line(gc, x,0, x,YSIZE-1)
		w += GRIDW * 2.0 * math.pi / 44100
	win.draw_line(gc, XSIZE-1,0, XSIZE-1,YSIZE-1)

	v = 0.0
	while v < MAXVAL :
		y = int(YSIZE - v * YSIZE / MAXVAL)
		win.draw_line(gc, 0,y, XSIZE,y)
		v += GRIDUP
	win.draw_line(gc, 0,YSIZE-1, XSIZE,YSIZE-1)

def drawcurve(win, gc, filt) :
	XSIZE,YSIZE =win.get_size()
	w = 0.0
	while w < math.pi :
		z = complex(math.cos(w), math.sin(w))
		z2 = z * z
		v = (filt.b2 + filt.b1 * z + filt.b0 * z2) / \
				(filt.a2 + filt.a1 * z + z2)
		v2 = abs(v * v)
		win.draw_point(gc, int(w * XSIZE/math.pi), int(YSIZE - v2 * YSIZE/MAXVAL))
		w += math.pi / STEPS
	
def redraw(area) :
	style = area.get_style()
	gcgrid = style.mid_gc[gtk.STATE_NORMAL]
	gcnorm = style.fg_gc[gtk.STATE_NORMAL]
	win = area.window

	funcs[funcidx](gainadj.value, freqadj.value)

	win.clear()
	drawgrid(win, gcgrid)
	drawcurve(win, gcnorm, filter)

def adjchanged(adj) :
	redraw(drawarea)

def update(area, event) :
	redraw(area)

def selectfilt(w, data) :
	global funcidx
	funcidx = filtertypes.index(data)
	redraw(drawarea)

def setupgui() :
	global drawarea, freqadj, gainadj

	# main window with items packed into a vbox
	win = gtk.Window()
	win.set_name("Shelfing Filter")
	win.connect("destroy", gtk.mainquit)
	winvb = gtk.VBox()
	win.add(winvb)
	winvb.show()

	dr = gtk.DrawingArea()
	drawarea = dr
	dr.set_size_request(XSIZE, YSIZE)
	winvb.pack_start(dr)
	#dr.connect("configure_event", drconfig)
	dr.show()

	x,freqadj = makescale("freq", 1, 1, SAMPFREQ / 2, adjchanged)
	winvb.pack_start(x, gtk.FALSE, gtk.FALSE)
	x,gainadj = makescale("gain", 0, -15, 15, adjchanged)
	winvb.pack_start(x, gtk.FALSE, gtk.FALSE)

	butbox = gtk.HBox()
	winvb.pack_start(butbox, gtk.FALSE, gtk.FALSE)
	butbox.show()
	group = None
	for name in filtertypes :
		but = gtk.RadioButton(group, name)
		if group == None :
			group = but
		butbox.pack_start(but)
		but.connect("toggled", selectfilt, name)
		but.show()

	dr.connect("expose_event", update)
	win.show()

def main() :
	global filter, funcs, funcidx
	filter = filt()
	funcs = [filter.lowshelf, filter.highshelf, filter.peaking]
	funcidx = 0

	win = gtk.Window()
	setupgui()
	gtk.main()
	
if __name__ == "__main__" :
	main()

