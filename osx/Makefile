CXX := g++-4.6

VPATH=..

WXCXXFLAGS := -I/usr/local/lib/wx/include/osx_cocoa-unicode-static-2.9 \
-I/usr/local/include/wx-2.9 -D_FILE_OFFSET_BITS=64 -D__WXMAC__ -D__WXOSX__ \
-D__WXOSX_COCOA__

WXLIBS := -L/usr/local/lib -framework IOKit -framework Carbon -framework Cocoa \
-framework AudioToolbox -framework System -framework OpenGL \
-framework QuickTime /usr/local/lib/libwx_osx_cocoau_xrc-2.9.a \
/usr/local/lib/libwx_osx_cocoau_html-2.9.a \
/usr/local/lib/libwx_osx_cocoau_qa-2.9.a \
/usr/local/lib/libwx_osx_cocoau_adv-2.9.a \
/usr/local/lib/libwx_osx_cocoau_core-2.9.a \
/usr/local/lib/libwx_baseu_xml-2.9.a /usr/local/lib/libwx_baseu_net-2.9.a \
/usr/local/lib/libwx_baseu-2.9.a -framework WebKit -lexpat -lwxregexu-2.9 \
-lwxtiff-2.9 -lwxjpeg-2.9 -lwxpng-2.9 -lz -lpthread -liconv -lsqlite3

CXXFLAGS := $(WXCXXFLAGS) -fno-common -Woverloaded-virtual

DEBUG := 1

ifeq ($(DEBUG),1)
	CFLAGS += -g
	CXXFLAGS += -g
else
	CFLAGS += -DNDEBUG -O2
	CXXFLAGS += -DNDEBUG -O2
endif

.PHONY: all
all: Scheduler.app/Contents/MacOS/scheduler

include ../common.mk

Scheduler.app/Contents/MacOS/scheduler : scheduler
	mkdir -p Scheduler.app/Contents/MacOS
	cp scheduler Scheduler.app/Contents/MacOS

$(cppobjs) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

scheduler : $(cppobjs)
	$(CXX) -o scheduler $(cppobjs) $(WXLIBS)

.PHONY: depend
depend: depend.mk

depend.mk : $(patsubst %.o,%.cpp,$(cppobjs))
	$(CXX) $(CXXFLAGS) -MM $<

.PHONY: clean
clean:
	rm -rf Scheduler.app
	rm -f scheduler
	rm -f *.o

.PHONY: run
run:
	open Scheduler.app
# DO NOT DELETE

../app.o: ../app.h ../frame.h
