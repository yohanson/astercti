CXXFLAGS=-std=c++11 `wx-config --cflags` `pkg-config --cflags jsoncpp`
BINARY=astercti
DBGDIR=debug
RELDIR=release
WINDBGDIR=debug_win
WINRELDIR=release_win
WINPATH=/usr/local/libwxmsw3.0/bin

DEBUG_OBJ=$(DBGDIR)/myapp.o $(DBGDIR)/mainframe.o $(DBGDIR)/notificationFrame.o \
	  $(DBGDIR)/taskbaricon.o $(DBGDIR)/controller.o $(DBGDIR)/asterisk.o \
	  $(DBGDIR)/observer.o $(DBGDIR)/events.o

RELEASE_OBJ=$(RELDIR)/myapp.o $(RELDIR)/mainframe.o $(RELDIR)/notificationFrame.o \
	  $(RELDIR)/taskbaricon.o $(RELDIR)/controller.o $(RELDIR)/asterisk.o \
	  $(RELDIR)/observer.o $(RELDIR)/events.o

WINRELEASE_OBJ=$(WINRELDIR)/myapp.o $(WINRELDIR)/mainframe.o $(WINRELDIR)/notificationFrame.o \
	  $(WINRELDIR)/taskbaricon.o $(WINRELDIR)/controller.o $(WINRELDIR)/asterisk.o \
	  $(WINRELDIR)/observer.o $(WINRELDIR)/events.o $(WINRELDIR)/jsoncpp.o $(WINRELDIR)/gitversion.o



OUTDIR=$(RELDIR)

all: release

clean:
	rm -f $(BINARY) *.o
	rm -f gitversion.cpp
	rm -f *.exe
	rm -f $(WINRELDIR)/*.o
	rm -f $(WINDBGDIR)/*.o

$(BINARY): myapp.o mainframe.o notificationFrame.o taskbaricon.o controller.o asterisk.o observer.o events.o gitversion.o ipc.o
	$(CXX) `wx-config --libs` `pkg-config --libs jsoncpp` *.o -o $(BINARY)

debug: CXXFLAGS += -DDEBUG -g
debug: $(BINARY)

release: CXXFLAGS += -s -DNDEBUG -O2
release: $(BINARY) i18n/ru.mo
	strip --strip-all $(BINARY)

messages.po:
	xgettext -C -k_ --omit-header *.cpp

# template:
#%.en.po : %.pot
#	-[ -e $@ ] && msgmerge --width=110 --update $@ $<
#	[ -e $@ ] || cp $< $@

i18n/%.po: messages.po
	msgmerge --width=110 --update $@ $<
	rm -f messages.po

i18n: i18n/ru.mo

i18n/ru.mo:
	msgfmt i18n/ru.po -o i18n/ru.mo

install: release
	cp -r $(BINARY)			$(DESTDIR)/usr/bin/astercti
	cp -r dial.png			$(DESTDIR)/usr/share/astercti/dial.png
	cp -r incoming_answered.png	$(DESTDIR)/usr/share/astercti/incoming_answered.png
	cp -r incoming_unanswered.png	$(DESTDIR)/usr/share/astercti/incoming_unanswered.png
	cp -r outbound_answered.png	$(DESTDIR)/usr/share/astercti/outbound_answered.png
	cp -r outbound_unanswered.png	$(DESTDIR)/usr/share/astercti/outbound_unanswered.png
	cp -r wait.gif			$(DESTDIR)/usr/share/astercti/wait.gif
	cp -r astercti.ini.default	$(DESTDIR)/usr/share/astercti/astercti.ini.default
	cp -r astercti.png		$(DESTDIR)/usr/share/pixmaps/astercti.png
	cp -r i18n/ru.mo		$(DESTDIR)/usr/share/locale/ru/LC_MESSAGES/asterisk.mo
	cp -r astercti.desktop	$(DESTDIR)/usr/share/applications


deb:
	debuild --no-tgz-check -i -us -uc -b

.PHONY: gitversion.cpp i18n/*.mo

gitversion.cpp:
	echo "const char *gitcommit = \"$(shell git rev-parse --short HEAD)\";" > $@
	echo "const char *gitcommitdate = \"$(shell git show -s --format=%ai --date=iso)\";" >> $@
	echo "const char *builddate = \"$(shell date --rfc-3339=date)\";" >> $@

$(WINRELDIR)/%.o: CXX=i686-w64-mingw32-g++
$(WINRELDIR)/%.o: CXXFLAGS=-s -DNDEBUG -O2 -std=c++11 `$(WINPATH)/wx-config --cflags` -I../jsoncpp/dist -I../jsoncpp/include
$(WINRELDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

win: CXX=i686-w64-mingw32-g++
win: LDFLAGS=-static -L/usr/lib `$(WINPATH)/wx-config --libs`
win: $(WINRELEASE_OBJ) i18n/ru.mo
	echo $(CXX)  $(WINRELEASE_OBJ) $(LDFLAGS)  -o $(BINARY).exe
	$(CXX)  $(WINRELEASE_OBJ) $(LDFLAGS)  -o $(BINARY).exe
	strip --strip-all $(BINARY).exe
	makensis windows_install_script.nsis

bump: debianbump versionhbump

debianbump:
	dch -i -m -U

versionhbump: VERSION=$(shell cat debian/changelog | head -n1 | grep -o '[0-9\.]*-' | grep -o '[0-9\.]*')
versionhbump:
	sed -i 's/^#define VERSION .*$$/#define VERSION "$(VERSION)"/' version.h

