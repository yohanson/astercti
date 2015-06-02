CXXFLAGS=-std=c++11 `wx-config --cflags`
BINARY=astercti
DBGDIR=debug
RELDIR=release

DEBUG_OBJ=$(DBGDIR)/myapp.o $(DBGDIR)/mainframe.o $(DBGDIR)/notificationFrame.o \
	  $(DBGDIR)/taskbaricon.o $(DBGDIR)/controller.o $(DBGDIR)/asterisk.o $(DBGDIR)/observer.o

RELEASE_OBJ=$(RELDIR)/myapp.o $(RELDIR)/mainframe.o $(RELDIR)/notificationFrame.o \
	  $(RELDIR)/taskbaricon.o $(RELDIR)/controller.o $(RELDIR)/asterisk.o $(RELDIR)/observer.o


OUTDIR=$(RELDIR)

all: release

clean:
	rm -f $(BINARY) *.o



$(BINARY): myapp.o mainframe.o notificationFrame.o taskbaricon.o controller.o asterisk.o observer.o
	$(CXX) `wx-config --libs` *.o -o $(BINARY)

debug: CXXFLAGS += -DDEBUG -g
debug: $(BINARY)

release: CXXFLAGS += -s -DNDEBUG -O2
release: $(BINARY)

install: release
	cp -r $(BINARY)			$(DESTDIR)/usr/bin/astercti
	cp -r dial.png			$(DESTDIR)/usr/share/astercti/dial.png
	cp -r incoming_answered.png	$(DESTDIR)/usr/share/astercti/incoming_answered.png
	cp -r incoming_unanswered.png	$(DESTDIR)/usr/share/astercti/incoming_unanswered.png
	cp -r outbound_answered.png	$(DESTDIR)/usr/share/astercti/outbound_answered.png
	cp -r outbound_unanswered.png	$(DESTDIR)/usr/share/astercti/outbound_unanswered.png
	cp -r wait.gif			$(DESTDIR)/usr/share/astercti/wait.gif
	cp -r astercti.ini		$(DESTDIR)/usr/share/astercti/astercti.ini
	cp -r astercti.png		$(DESTDIR)/usr/share/pixmaps/astercti.png

archive:
	rm ../astercti_0.0.2.orig.tar.gz
	tar zcf ../astercti_0.0.2.orig.tar.gz ../astercti

deb: archive
	debuild -i -us -uc -b


win:
	i686-w64-mingw32-g++ -std=c++11 `wx-config --libs --cflags` *.cpp -o $(BINARY).exe

