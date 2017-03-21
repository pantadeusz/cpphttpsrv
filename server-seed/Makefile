# This are internal config for Make
VER=$(shell hg head | head -n 1 | awk -F'[: \t]+' '{print $$2}')
VER_MAJOR=0
VER_MINOR=0
CPPFLAGS= -fPIC -pthread -std=c++11 -O2 -I./include/cpphttpsrv -fopenmp -DVER=\"$(VER_MAJOR).$(VER_MINOR).$(VER)\"
CLIBS= -std=c++11 -fPIC -fopenmp -ldl -rdynamic
BASEOBJS := 


##################  CONFIG #################
#
APPOBJS := main.o
APPNAME := thttp
#
################## ENDCONDIF ###############

OBJS :=  $(BASEOBJS) $(APPOBJS)

all: $(APPNAME)

serve: $(APPNAME)
	./$(APPNAME)

cpphttpsrv/libcpphttpsrv.so: cpphttpsrv
	rm -rf usr
	cd cpphttpsrv; make -j 8 dist

include: cpphttpsrv/libcpphttpsrv.so
	rm -rf include
	tar -xf cpphttpsrv/cpphttpsrv*.tar.bz2
	mv usr/include ./
	rm -rf usr

cpphttpsrv:
	hg clone https://bitbucket.org/t4deusz/cpphttpsrv

$(APPNAME): $(OBJS)
	g++  -O2 $(OBJS) -o $@  -Lcpphttpsrv -lcpphttpsrv $(CLIBS)

%.o : %.cpp include
	gcc -c $(CPPFLAGS) $<

clean:
	rm -f $(APPNAME) $(OBJS)

distclean: clean
	rm -rf cpphttpsrv include build/*.o build/rest

