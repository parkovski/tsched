CXX := g++

VPATH=..

WXCXXFLAGS := -I/usr/local/lib/wx/include/gtk2-unicode-static-2.9 \
-I/usr/local/include/wx-2.9 -D_FILE_OFFSET_BITS=64 -D__WXGTK__ -pthread


WXLIBS := -L/usr/local/lib -pthread /usr/local/lib/libwx_gtk2u_xrc-2.9.a \
/usr/local/lib/libwx_gtk2u_html-2.9.a /usr/local/lib/libwx_gtk2u_qa-2.9.a \
/usr/local/lib/libwx_gtk2u_adv-2.9.a /usr/local/lib/libwx_gtk2u_core-2.9.a \
/usr/local/lib/libwx_baseu_xml-2.9.a /usr/local/lib/libwx_baseu_net-2.9.a \
/usr/local/lib/libwx_baseu-2.9.a -pthread -L/usr/lib/i386-linux-gnu \
-lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgio-2.0 -lpangoft2-1.0 \
-lgdk_pixbuf-2.0 -lpango-1.0 -lfreetype -lfontconfig -lgobject-2.0 \
-lgmodule-2.0 -lgthread-2.0 -lrt -lglib-2.0 -lpng -lexpat -lwxregexu-2.9 \
-lwxtiff-2.9 -lwxjpeg-2.9 -lz -ldl -lm

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
all: scheduler

include ../common.mk

$(cppobjs) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

scheduler : $(cppobjs)
	$(CXX) -o scheduler $(cppobjs) $(WXLIBS)

.PHONY: clean
clean:
	rm -f scheduler
	rm -f *.o

.PHONY: run
run:
	./scheduler
