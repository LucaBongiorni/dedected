CPPFLAGS=-Wall -O2 -I../..
dectshark: dectshark.o gui.o foundinfo.o scanmode_gui.o syncmode_gui.o packetparser.o packetsaver.o
	g++ $(CPPFLAGS) dectshark.o gui.o scanmode_gui.o syncmode_gui.o foundinfo.o packetparser.o packetsaver.o -o dectshark -lcurses -lpthread -lpcap

clean:
	rm *.o *~ dectshark
