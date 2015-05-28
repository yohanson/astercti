CXXFLAGS=-std=c++11 `wx-config --cflags`

all: binary

clean:
	rm binary *.o


binary: myapp.o mainframe.o notificationFrame.o
	g++ `wx-config --libs` *.o -o binary

#*.o:
#	echo 
#	#g++ -std=c++11 `wx-config --cflags`
#	g++ -std=c++11 `wx-config --cflags` -c mainframe.cpp
#	g++ -std=c++11 `wx-config --cflags` -c notificationFrame.cpp


win:
	i686-w64-mingw32-g++ -std=c++11 `wx-config --libs --cflags` wxnet.cpp notificationFrame.cpp -o binary.exe

