CPPFLAGS=-Wall -O2 -I../.. -lcurses -lpthread
dectshark: dectshark.o gui.o foundinfo.o
	g++ $(CPPFLAGS) dectshark.cpp gui.cpp foundinfo.cpp -o dectshark

clean:
	rm *.o dectshark
