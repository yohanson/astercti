all:
	g++ -std=c++11 `wx-config --libs --cflags` wxnet.cpp notificationFrame.cpp -o binary

