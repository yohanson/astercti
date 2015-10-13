CXXFLAGS=-std=c++11 `wx-config --cflags` `pkg-config --cflags jsoncpp`
BINARY=astercti
DBGDIR=debug
RELDIR=release
WINDBGDIR=debug_win
WINRELDIR=release_win
WINPATH=/usr/local/libwxmsw3.0/bin
OBJECTS=myapp.o mainframe.o notificationFrame.o taskbaricon.o controller.o \
	asterisk.o observer.o events.o gitversion.o ipc.o

DEBUG_OBJ=$(addprefix $(DBGDIR)/, $(OBJECTS))
RELEASE_OBJ=$(addprefix $(RELDIR)/, $(OBJECTS))
WINRELEASE_OBJ=$(addprefix $(WINRELDIR)/, $(OBJECTS))
WINDEBUG_OBJ=$(addprefix $(WINDBGDIR)/, $(OBJECTS))

init:
	mkdir -p $(DBGDIR) $(RELDIR) $(WINDBGDIR) $(WINRELDIR)

clean:
	rm -f $(BINARY) *.o
	rm -f gitversion.cpp
	rm -f *.exe
	rm -f $(DBGDIR)/*
	rm -f $(RELDIR)/*
	rm -f $(WINRELDIR)/*
	rm -f $(WINDBGDIR)/*

$(DBGDIR)/%.o: CXXFLAGS += -DDEBUG -g
$(DBGDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

$(RELDIR)/%.o: CXXFLAGS += -s -DNDEBUG -O2
$(RELDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

$(WINDBGDIR)/%.o: CXX=i686-w64-mingw32-g++
$(WINDBGDIR)/%.o: CXXFLAGS=-DDEBUG -std=c++11 `$(WINPATH)/wx-config --cflags` -I../jsoncpp/dist -I../jsoncpp/include
$(WINDBGDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(WINRELDIR)/%.o: CXX=i686-w64-mingw32-g++
$(WINRELDIR)/%.o: CXXFLAGS=-s -DNDEBUG -O2 -std=c++11 `$(WINPATH)/wx-config --cflags` -I../jsoncpp/dist -I../jsoncpp/include
$(WINRELDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


$(DBGDIR)/$(BINARY): $(DEBUG_OBJ)
	$(CXX) `wx-config --libs` `pkg-config --libs jsoncpp` $(DEBUG_OBJ) -o $@
	ln -sf $@ $(BINARY)

$(RELDIR)/$(BINARY): $(RELEASE_OBJ)
	$(CXX) `wx-config --libs` `pkg-config --libs jsoncpp` $(RELEASE_OBJ) -o $@
	strip --strip-all $@
	ln -sf $@ $(BINARY)

$(WINDBGDIR)/$(BINARY).exe: CXX=i686-w64-mingw32-g++
$(WINDBGDIR)/$(BINARY).exe: LDFLAGS=-static -L/usr/lib `$(WINPATH)/wx-config --libs`
$(WINDBGDIR)/$(BINARY).exe: $(WINRELEASE_OBJ) i18n/ru.mo
	$(CXX)  $(WINRELEASE_OBJ) $(LDFLAGS)  -o $@


$(WINRELDIR)/$(BINARY).exe: CXX=i686-w64-mingw32-g++
$(WINRELDIR)/$(BINARY).exe: LDFLAGS=-static -L/usr/lib `$(WINPATH)/wx-config --libs`
$(WINRELDIR)/$(BINARY).exe: $(WINRELEASE_OBJ) i18n/ru.mo
	$(CXX)  $(WINRELEASE_OBJ) $(LDFLAGS)  -o $@
	strip --strip-all $@
	makensis windows_install_script.nsis


debug: $(DBGDIR)/$(BINARY)

release: $(RELDIR)/$(BINARY) i18n/ru.mo

windebug: $(WINDBGDIR)/$(BINARY).exe

winrelease: $(WINRELDIR)/$(BINARY).exe i18n/ru.mo


messages.po:
	xgettext -C -k_ --omit-header *.cpp

i18n/%.po: messages.po
	msgmerge --width=110 --update $@ $<
	rm -f messages.po

i18n: i18n/ru.mo

i18n/ru.mo:
	msgfmt i18n/ru.po -o i18n/ru.mo

install: release
	cp -r $(BINARY)					$(DESTDIR)/usr/bin/astercti
	cp -r dial.png					$(DESTDIR)/usr/share/astercti/dial.png
	cp -r incoming_answered.png		$(DESTDIR)/usr/share/astercti/incoming_answered.png
	cp -r incoming_unanswered.png	$(DESTDIR)/usr/share/astercti/incoming_unanswered.png
	cp -r outbound_answered.png		$(DESTDIR)/usr/share/astercti/outbound_answered.png
	cp -r outbound_unanswered.png	$(DESTDIR)/usr/share/astercti/outbound_unanswered.png
	cp -r wait.gif					$(DESTDIR)/usr/share/astercti/wait.gif
	cp -r astercti.ini.default		$(DESTDIR)/usr/share/astercti/astercti.ini.default
	cp -r astercti.png				$(DESTDIR)/usr/share/pixmaps/astercti.png
	cp -r i18n/ru.mo				$(DESTDIR)/usr/share/locale/ru/LC_MESSAGES/asterisk.mo
	cp -r astercti.desktop			$(DESTDIR)/usr/share/applications


deb:
	debuild --no-tgz-check -i -us -uc -b

.PHONY: gitversion.cpp i18n/*.mo

gitversion.cpp:
	echo "const char *gitcommit = \"$(shell git rev-parse --short HEAD)\";" > $@
	echo "const char *gitcommitdate = \"$(shell git show -s --format=%ai --date=iso)\";" >> $@
	echo "const char *builddate = \"$(shell date --rfc-3339=date)\";" >> $@

bump: debianbump versionhbump

debianbump:
	dch -i -m -U

versionhbump: VERSION=$(shell cat debian/changelog | head -n1 | grep -o '[0-9\.]*-' | grep -o '[0-9\.]*')
versionhbump:
	sed -i 's/^#define VERSION .*$$/#define VERSION "$(VERSION)"/' version.h

