CXXFLAGS=-std=c++11 `wx-config --cflags` `pkg-config --cflags jsoncpp`
BINARY=astercti
DBGDIR=build/debug
RELDIR=build/release
WINDBGDIR=build/debug_win
WINRELDIR=build/release_win
WINPATH=/usr/local/libwxmsw3.0/bin
JSONPATH=../jsoncpp
CURLPATH=../curl
DEFAULT_DEBIAN_RELEASE=stretch
OBJECTS= \
	asterisk.o \
	calllistctrl.o \
	call.o \
	chanstatus.o \
	debugreport.o \
	events.o \
	executer.o \
	filter.o \
	gitversion.o \
	ipc.o \
	lookup.o \
	mainframe.o \
	myapp.o \
	notificationFrame.o \
	numbershortener.o \
	observer.o \
	taskbaricon.o \
	utils.o
BIGICONS=astercti astercti-missed
SMALLICONS=dial hangup incoming_answered_elsewhere incoming_answered \
	incoming_unanswered outbound_answered outbound_unanswered

DEBUG_OBJ=$(addprefix $(DBGDIR)/, $(OBJECTS))
RELEASE_OBJ=$(addprefix $(RELDIR)/, $(OBJECTS))
WINRELEASE_OBJ=$(addprefix $(WINRELDIR)/, $(OBJECTS) jsoncpp.o resource.o)
WINDEBUG_OBJ=$(addprefix $(WINDBGDIR)/, $(OBJECTS) jsoncpp.o resource.o)

default:
	@echo "Please supply target: debug, release, windebug, winrelease, deb"

debug: $(DBGDIR)/$(BINARY)

release: $(RELDIR)/$(BINARY) i18n/ru.mo

windebug: $(WINDBGDIR)/$(BINARY).exe

winrelease: $(WINRELDIR)/$(BINARY).exe i18n/ru.mo


$(DBGDIR) $(RELDIR) $(WINDBGDIR) $(WINRELDIR):
	mkdir -p $@

clean:
	rm -f $(BINARY)
	rm -f gitversion.cpp
	rm -f *.exe
	rm -f *.rc
	rm -f *.ico
	rm -rf build
	rm -f runner src/runner.cpp

$(DBGDIR)/%.o: CXXFLAGS += -DDEBUG -ggdb -O0
$(DBGDIR)/%.o: src/%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

$(RELDIR)/%.o: CXXFLAGS += -s -DNDEBUG -O2
$(RELDIR)/%.o: src/%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

#$(WINDBGDIR)/%.o: CXXFLAGS=-DDEBUG -g -std=c++11 `$(WINPATH)/wx-config --cflags` -I$(JSONPATH)/include -I$(JSONPATH)/dist -I$(CURLPATH)/include
$(WINDBGDIR)/%.o: CXX=i686-w64-mingw32-g++
$(WINDBGDIR)/%.o: CXXFLAGS=-DDEBUG -g -std=c++11 `$(WINPATH)/wx-config --cflags` -I$(JSONPATH)/dist -I$(CURLPATH)/include
$(WINDBGDIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

resource.rc: Makefile $(addsuffix .ico, $(BIGICONS) $(SMALLICONS))
	> resource.rc
	$(foreach icon,$(BIGICONS) $(SMALLICONS), echo $(icon) ICON \"$(addsuffix .ico,$(icon))\" >> resource.rc;)

%/resource.o: resource.rc
	`$(WINPATH)/wx-config --rescomp` -o $@ $<

#Big icons:
asterct%.ico: img/asterct%.png
	convert $< -define icon:auto-resize=32,16 $@

#Small icons:
%.ico: img/%.png
	convert $< -define icon $@

#$(WINRELDIR)/%.o: CXXFLAGS=-s -DNDEBUG -O2 -std=c++11 `$(WINPATH)/wx-config --cflags` -I$(JSONPATH)/include -I$(JSONPATH)/dist -I$(CURLPATH)/include
$(WINRELDIR)/%.o: CXX=i686-w64-mingw32-g++
$(WINRELDIR)/%.o: CXXFLAGS=-s -DNDEBUG -O2 -std=c++11 `$(WINPATH)/wx-config --cflags` -I$(JSONPATH)/dist -I$(CURLPATH)/include
$(WINRELDIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(DBGDIR)/$(BINARY): $(DBGDIR) $(DEBUG_OBJ)
	$(CXX) $(LDFLAGS) -g1 `wx-config --libs` `pkg-config --libs jsoncpp libcurl` $(DEBUG_OBJ) -o $@
	ln -sf $@ $(BINARY)

$(RELDIR)/$(BINARY): $(RELDIR) $(RELEASE_OBJ)
	$(CXX) $(LDFLAGS) `wx-config --libs` `pkg-config --libs jsoncpp libcurl` $(RELEASE_OBJ) -o $@
	strip --strip-all $@
	ln -sf $@ $(BINARY)

$(WINDBGDIR)/$(BINARY).exe: VERSION=$(shell cat src/version.h | grep VERSION | grep -o '"[0-9a-z\.-]*"' | grep -o '[0-9a-z\.-]*')
$(WINDBGDIR)/$(BINARY).exe: CXX=i686-w64-mingw32-g++
$(WINDBGDIR)/$(BINARY).exe: LDFLAGS+=-static -L/usr/lib -L/usr/local/lib `$(WINPATH)/wx-config --libs` -llibcurl -L.
$(WINDBGDIR)/$(BINARY).exe: $(WINDBGDIR) $(WINDEBUG_OBJ) i18n/ru.mo
	$(CXX) $(WINDEBUG_OBJ) $(LDFLAGS) -o $@
	makensis windows_install_debug.nsis
	mv astercti_debug_installer.exe pkg/astercti_$(VERSION)_debug_installer.exe

$(WINRELDIR)/$(BINARY).exe: VERSION=$(shell cat src/version.h | grep VERSION | grep -o '"[0-9a-z\.-]*"' | grep -o '[0-9a-z\.-]*')
$(WINRELDIR)/$(BINARY).exe: CXX=i686-w64-mingw32-g++
$(WINRELDIR)/$(BINARY).exe: LDFLAGS+=-static -L/usr/lib -L/usr/local/lib `$(WINPATH)/wx-config --libs`
$(WINRELDIR)/$(BINARY).exe: $(WINRELDIR) $(WINRELEASE_OBJ) i18n/ru.mo
	$(CXX) $(WINRELEASE_OBJ) libcurl.dll $(LDFLAGS) -o $@
	strip --strip-all $@
	makensis windows_install_script.nsis
	mv astercti_installer.exe pkg/astercti_$(VERSION)_installer.exe


debug: $(DBGDIR)/$(BINARY)

release: $(RELDIR)/$(BINARY) i18n/ru.mo

windebug: $(WINDBGDIR)/$(BINARY).exe

winrelease: $(WINRELDIR)/$(BINARY).exe i18n/ru.mo

src/runner.cpp: src/*_test.h
	cxxtestgen --error-printer -o src/runner.cpp src/*_test.h

runner: src/runner.cpp
	$(CXX) $(CXXFLAGS) src/runner.cpp -o runner

test: CXXFLAGS+=-DTEST
test: runner
	./runner
	

messages.po:
	xgettext -C -k_ -kwxPLURAL:1,2 --omit-header src/*.cpp

i18n/%.po: messages.po
	msgmerge --width=110 --update $@ $<
	rm -f messages.po

i18n: i18n/ru.mo

i18n/ru.mo:
	msgfmt i18n/ru.po -o i18n/ru.mo

install: release
	cp -r $(RELDIR)/$(BINARY)					$(DESTDIR)/usr/bin/astercti
	cp -r img/dial.png							$(DESTDIR)/usr/share/astercti/dial.png
	cp -r img/hangup.png						$(DESTDIR)/usr/share/astercti/hangup.png
	cp -r img/incoming_answered.png				$(DESTDIR)/usr/share/astercti/incoming_answered.png
	cp -r img/incoming_unanswered.png			$(DESTDIR)/usr/share/astercti/incoming_unanswered.png
	cp -r img/incoming_answered_elsewhere.png	$(DESTDIR)/usr/share/astercti/incoming_answered_elsewhere.png
	cp -r img/outbound_answered.png				$(DESTDIR)/usr/share/astercti/outbound_answered.png
	cp -r img/outbound_unanswered.png			$(DESTDIR)/usr/share/astercti/outbound_unanswered.png
	cp -r img/wait.gif							$(DESTDIR)/usr/share/astercti/wait.gif
	cp -r img/astercti.png						$(DESTDIR)/usr/share/pixmaps/astercti.png
	cp -r img/astercti.png						$(DESTDIR)/usr/share/astercti/astercti.png
	cp -r img/astercti-missed.png				$(DESTDIR)/usr/share/astercti/astercti-missed.png
	cp -r astercti.ini.default					$(DESTDIR)/usr/share/astercti/astercti.ini.default
	cp -r i18n/ru.mo							$(DESTDIR)/usr/share/locale/ru/LC_MESSAGES/astercti.mo
	cp -r astercti.desktop						$(DESTDIR)/usr/share/applications/astercti.desktop
	cp -r astercti.1							$(DESTDIR)/usr/share/man/man1/astercti.1
	gzip										$(DESTDIR)/usr/share/man/man1/astercti.1



deb:
	debuild --no-tgz-check -i -us -uc -b
	mkdir -p pkg/$(shell lsb_release -sc)
	mv ../astercti_* pkg/$(shell lsb_release -sc)/

.PHONY: src/gitversion.cpp i18n/*.mo

src/gitversion.cpp:
	echo "const char *gitcommit = \"$(shell git rev-parse --short HEAD)\";" > $@
	echo "const char *gitcommitdate = \"$(shell git show -s --format=%ai --date=iso)\";" >> $@
	echo "const char *builddate = \"$(shell date -Iseconds)\";" >> $@

bump: debianbump versionhbump

debianbump:
	dch -i -m -U -D unstable

versionhbump: VERSION=$(shell cat debian/changelog | head -n1 | grep -o '[0-9\.]*-' | grep -o '[0-9\.]*')
versionhbump:
	sed -i 's/^#define VERSION .*$$/#define VERSION "$(VERSION)"/' src/version.h

docker-build: docker-debian-stretch docker-debian-jessie docker-windows

docker-debian-stretch: DEBIAN_RELEASE=stretch
docker-debian-stretch: docker-debian

docker-debian-jessie: DEBIAN_RELEASE=jessie
docker-debian-jessie: docker-debian

docker-debian-image: DOCKER_IMAGE=astercti-build-debian-$(DEBIAN_RELEASE)
docker-debian-image: image_timestamp=$(shell docker image inspect -f '{{json .Metadata.LastTagTime }}' $(DOCKER_IMAGE) | xargs date +%s -d)
docker-debian-image: dockerfile_timestamp=$(shell stat -c%Y Dockerfile)
docker-debian-image:
	@test -n "$(DEBIAN_RELEASE)" || ( echo "DEBIAN_RELEASE variable must be defined"; false )

	if [ "$(image_timestamp)" -lt "$(dockerfile_timestamp)" ]; then \
		docker rm -f $(DOCKER_IMAGE) || true ; \
		docker build --tag=webex . ; \
		docker build --build-arg DEBIAN_RELEASE=$(DEBIAN_RELEASE) -t $(DOCKER_IMAGE) . ; \
	fi


docker-debian: docker-debian-image
	docker run -it --rm -v $(shell pwd):/build/astercti astercti-build-debian-$(DEBIAN_RELEASE) make deb

docker-windows-image: DEBIAN_RELEASE=$(DEFAULT_DEBIAN_RELEASE)
docker-windows-image: docker-debian-image
	docker inspect astercti-build-windows >/dev/null 2>&1 || \
	docker build --build-arg DEBIAN_RELEASE=$(DEBIAN_RELEASE) -f Dockerfile.win -t astercti-build-windows .

docker-windows: docker-windows-image
	docker run -it --rm -v $(shell pwd):/build/astercti astercti-build-windows make winrelease

