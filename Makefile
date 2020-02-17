NAME=convergence

CPPFLAGS=-g -O0 -fPIC -Wall -Wno-reorder -Wno-sign-compare -Wno-switch
CXXFLAGS=-std=c++17
LDFLAGS=
LDLIBS=-lGL -lglfw
LOCALLIBS=
BUNDLES=
INCLUDES=-I. -Ideps/glm

ARCH:=$(shell $(CC) -dumpmachine | cut -f 1 -d -)
BUILDDIR:=build/$(ARCH)
OBJDIR:=$(BUILDDIR)
OUTPUT:=$(BUILDDIR)/$(NAME)

ifeq ($(notdir $(CXX)),em++)
DIR=emscripten
EMFLAGS=-s WASM=1 -s BINARYEN_METHOD=native-wasm -s TOTAL_MEMORY=256MB -s DISABLE_EXCEPTION_CATCHING=1 -s BINARYEN_TRAP_MODE=clamp -s USE_PTHREADS=0 -s USE_GLFW=3 -s USE_SDL_IMAGE=2
JSLIBS=$(shell printf "%s " emscripten/js/*.js)
BUNDLEDIRS+=shader res
CPPFLAGS+=$(EMFLAGS) -DUSE_OPENGL_ES
LDFLAGS+=$(EMFLAGS) --closure 1
LDFLAGS+=$(addprefix --js-library ,$(JSLIBS)) --pre-js emscripten/pre.js
LDFLAGS+=$(addprefix --preload-file ,$(BUNDLEDIRS)) --use-preload-plugins
BUNDLES+=emscripten/index.html $(JSLIBS) emscripten/pre.js $(shell printf "%s " $(addsuffix /*,$(BUNDLEDIRS)))
OUTPUT:=$(OUTPUT).js
else
DIR=native
INCLUDES+=-Ideps/libdatachannel/include -Ideps/libdatachannel/deps/plog/include
LOCALLIBS+=$(BUILDDIR)/libdatachannel.a $(BUILDDIR)/libjuice.a $(BUILDDIR)/libusrsctp.a
CPPFLAGS+= $(shell pkg-config freetype2 --cflags)
LDFLAGS+=-pthread
LDLIBS+=-lGLEW -lIL -lnettle -lhogweed -lgmp -lgnutls -largon2
LDLIBS+= $(shell pkg-config freetype2 --libs)
endif

INCLUDES+=-I$(DIR)

SRCS=$(shell printf "%s " pla/*.cpp src/*.cpp)
SRCS+=$(shell printf "%s " $(DIR)/pla/*.cpp $(DIR)/net/*.cpp $(DIR)/*.cpp)
OBJS=$(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

all: $(OUTPUT)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -MMD -MP -c -o $@ $<

-include $(SRCS:.cpp=.d)

$(OUTPUT): $(LOCALLIBS) $(OBJS) $(BUNDLES) | $(BUILDDIR)
	$(CXX) $(LDFLAGS) -o $(OUTPUT) $(OBJS) $(LOCALLIBS) $(LDLIBS)
ifeq ($(notdir $(CXX)),em++)
	cp emscripten/index.html $(BUILDDIR)/index.html
endif

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	-rm *.d {pla,src}/*.d $(DIR)/*.d $(DIR)/{pla,src}/*.d
	-rm -r $(BUILDDIR)/$(DIR) $(BUILDDIR)/pla $(BUILDDIR)/src
	-cd deps/libdatachannel && make clean

dist-clean: clean
	-rm -r build

$(BUILDDIR)/libdatachannel.a $(BUILDDIR)/libjuice.a $(BUILDDIR)/libusrsctp.a: | $(BUILDDIR)
	cd deps/libdatachannel && $(MAKE) USE_JUICE=1 USE_GNUTLS=1
	cp deps/libdatachannel/libdatachannel.a $(BUILDDIR)
	cp deps/libdatachannel/libjuice.a $(BUILDDIR)
	cp deps/libdatachannel/libusrsctp.a $(BUILDDIR)

