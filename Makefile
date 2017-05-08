# This are internal config for Make
VER=$(shell hg head | head -n 1 | awk -F'[: \t]+' '{print $$2}')
VER_MAJOR=0
VER_MINOR=0
CPPFLAGS= -fPIC -pthread -std=c++11 -O2 -I./src -fopenmp -DVER=\"$(VER_MAJOR).$(VER_MINOR).$(VER)\"
CLIBS= -std=c++11 -fopenmp -ldl -L./build -rdynamic
BASEOBJS := build/http.o build/mime.o build/dhandler.o build/hhelpers.o


##################  CONFIG #################
#
APPOBJS := main.o
APPNAME := thttp
#
################## ENDCONDIF ###############

OBJS := $(APPOBJS) $(BASEOBJS)

all: libcpphttpsrv.a libcpphttpsrv.so $(APPNAME)  build/test-mime

dist: all
	mkdir -p usr/include/cpphttpsrv
	cp src/*.h* usr/include/cpphttpsrv/
	mkdir -p usr/lib
	mkdir -p usr/share/doc/cpphttpsrv
	cp LICENSE README* usr/share/doc/cpphttpsrv/
	cp libcpphttpsrv.* usr/lib/
	tar -cjvf cpphttpsrv-$(VER_MAJOR).$(VER_MINOR).$(VER).tar.bz2 usr
	rm -R usr

libcpphttpsrv.so: $(BASEOBJS)
	g++ -Isrc -std=c++11 -fPIC -shared $(BASEOBJS) -o $@

libcpphttpsrv.a: $(BASEOBJS)
	ar rcs $@ $(BASEOBJS)

serve: $(APPNAME)
	./$(APPNAME)

$(APPNAME): $(OBJS)
	g++ -O2 $^ -o $@ $(CLIBS)

build/%.o : src/%.cpp
	@echo $<
	gcc -c $(CPPFLAGS) $< -o $@

%.o : %.cpp
	gcc -c $(CPPFLAGS) $<


build/test-mime: $(BASEOBJS) src-test/test-mime.cpp
	g++ -c -Wall -Wextra -std=c++11 src-test/test-mime.cpp -o build/test-mime.o -I./src -I./src-test
	g++ $(BASEOBJS) build/test-mime.o -o build/test-mime  $(CLIBS)

build/test-http: $(BASEOBJS) src-test/test-http.cpp
	g++ -c -Wall -Wextra -std=c++11 src-test/test-http.cpp -o build/test-http.o -I./src -I./src-test
	g++ $(BASEOBJS) build/test-http.o -o build/test-http  $(CLIBS)

test-http: build/test-http
	build/test-http

test-mime: build/test-mime
	build/test-mime
	
test-connection: all
	./run-tests

test: test-connection test-mime test-http


clean:
	rm -f $(APPNAME) $(OBJS) cpphttpsrv-*.tar.bz2 libcpphttpsrv.a libcpphttpsrv.so build/test-* build/*.o
