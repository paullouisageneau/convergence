prefix=/usr
DESTDIR=

CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-pthread -std=c++14 -Wall -Wno-sign-compare -Og -g
LDFLAGS=-pthread
LDLIBS=$(shell pkg-config --libs glib-2.0 gobject-2.0 nice) -lGL -lGLEW -lglfw -lnettle -lhogweed -lgmp -lgnutls -largon2

SRCS=$(shell printf "%s " net/*.cpp pla/*.cpp ../pla/*.cpp ../p3d/*.cpp ../src/*.cpp *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

OUTPUT=convergence

all: $(OUTPUT)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -I. -I.. -I../glm -Ilibrtcdcpp/include -Ilibrtcdcpp/spdlog/include -MMD -MP -o $@ -c $<

-include $(subst .o,.d,$(OBJS))

$(OUTPUT): librtcdcpp/librtcdcpp.a $(OBJS)
	$(CXX) $(LDFLAGS) -o $(OUTPUT) $(OBJS) $(LDLIBS) librtcdcpp/librtcdcpp.a librtcdcpp/libusrsctp.a

librtcdcpp/librtcdcpp.a:
	cd librtcdcpp && $(MAKE) USE_GNUTLS=1

clean:
	cd librtcdcpp && $(MAKE) dist-clean
	$(RM) net/*.o net/*.d pla/*.o pla/*.d 
	$(RM) ../pla/*.o ../pla/*.d ../p3d/*.o ../p3d/*.d ../src/*.o ../src/*.d *.o *.d

dist-clean: clean
	$(RM) $(OUTPUT)
	$(RM) net/*~ pla/*~
	$(RM) ../pla/*~ ../p3d/*~ ../src/*~
