CPPFLAGS=-Wall -O2 -I../..
dectshark: dectshark.o gui.o foundinfo.o scanmode_gui.o syncmode_gui.o packetparser.o
	g++ $(CPPFLAGS) dectshark.o gui.o scanmode_gui.o syncmode_gui.o foundinfo.o packetparser.o -o dectshark -lcurses -lpthread

clean:
	rm *.o *~ dectshark
