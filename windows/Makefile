CXX := g++

VPATH=..

WXDIR := C:/wxWidgets-2.9.1

WXCXXFLAGS := -I$(WXDIR)/lib/gcc_lib/mswu -I$(WXDIR)/include \
-D__WXMSW__ -D_UNICODE -DHAVE_W32API_H

WXLIBS := -L$(WXDIR)/lib/gcc_lib -Wl,--subsystem,windows -mwindows \
-lwxmsw29u_core -lwxbase29u -lwxtiff -lwxjpeg -lwxpng -lwxzlib -lwxregexu \
-lwxexpat -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lwinspool -lwinmm -lshell32 \
-lcomctl32 -lole32 -loleaut32 -luuid -lrpcrt4 -ladvapi32 -lwsock32 -lwininet

CXXFLAGS := $(WXCXXFLAGS) -mthreads -fno-common -Woverloaded-virtual

DEBUG := 1

ifeq ($(DEBUG),1)
	CFLAGS += -g
	CXXFLAGS += -g
else
	CFLAGS += -DNDEBUG -Os
	CXXFLAGS += -DNDEBUG -Os
endif

.PHONY: all
all: scheduler.exe

include ../common.mk

$(cppobjs) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -I../../sqlite-win -c $< -o $@

windows-resources.o : windows-resources.rc
	windres -I$(WXDIR)/include -O coff -o $@ $<

scheduler.exe : $(cppobjs) ../sqlite3.o windows-resources.o
	$(CXX) -o scheduler.exe $(cppobjs) ../sqlite3.o windows-resources.o -mthreads $(WXLIBS) -static -static-libgcc -static-libstdc++

.PHONY: clean
clean:
	del scheduler.exe
	del *.o

.PHONY: run
run:
	scheduler.exe
