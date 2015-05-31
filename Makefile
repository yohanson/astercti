CXXFLAGS=-std=c++11 -ggdb `wx-config --cflags`
BINARY=astercti

all: $(BINARY)

clean:
	rm -f $(BINARY) *.o


$(BINARY): myapp.o mainframe.o notificationFrame.o taskbaricon.o controller.o asterisk.o
	g++ `wx-config --libs` *.o -o $(BINARY)


win:
	i686-w64-mingw32-g++ -std=c++11 `wx-config --libs --cflags` *.cpp -o $(BINARY).exe

