NAME=convergence

CPPFLAGS=-std=c++17 -g -O1 -fPIC -Wall -Wno-reorder -Wno-sign-compare -Wno-switch
LDFLAGS=-g -O1
LDLIBS=-lpthread -lGL -lglfw
INCLUDE=-I. -Iglm
BUNDLE=shader
EMFLAGS=-D USE_OPENGL_ES -s USE_GLFW=3 -s WASM=1 -s BINARYEN_METHOD=native-wasm -s TOTAL_MEMORY=256MB -s DISABLE_EXCEPTION_CATCHING=1 -s BINARYEN_TRAP_MODE=clamp

BUILDDIR=build/$(ARCH)
OBJDIR=$(BUILDDIR)
OUTPUT=$(BUILDDIR)/$(NAME)

ARCH:=$(shell $(CC) -dumpmachine | cut -f 1 -d -)
SRCS:=$(shell printf "%s " pla/*.cpp src/*.cpp)
JSLIBS:=$(shell printf "%s " emscripten/js/*.js)
BUNDLEDFILES:=$(shell printf "%s " $(addsuffix /*,$(BUNDLE)))

ifeq ($(notdir $(CXX)),em++)
DIR:=emscripten
LIBS:=$(JSLIBS)
CPPFLAGS+=$(EMFLAGS)
LDFLAGS+=$(EMFLAGS)
LDLIBS+=$(addprefix --js-library ,$(JSLIBS))
LDLIBS+=$(addprefix --preload-file ,$(BUNDLE))
OUTPUT:=$(OUTPUT).html
else
DIR:=native
LIBS:=$(DIR)/librtcdcpp/librtcdcpp.a $(DIR)/librtcdcpp/libusrsctp.a
INCLUDE+=-I$(DIR)/librtcdcpp/include
INCLUDE+=-I$(DIR)/librtcdcpp/spdlog/include
CPPFLAGS+=-DSPDLOG_DISABLED
LDLIBS+=$(shell pkg-config --libs glib-2.0 gobject-2.0 nice) -lGLEW -lnettle -lhogweed -lgmp -lgnutls -largon2
LDLIBS+=$(LIBS)
endif

INCLUDE+=-I$(DIR)
SRCS+=$(shell printf "%s " $(DIR)/net/*.cpp $(DIR)/pla/*.cpp $(DIR)/*.cpp)
OBJS:=$(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

all: $(OUTPUT)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(INCLUDE) -MMD -MP -c -o $@ $<

-include $(OBJS:.o=.d)

$(OUTPUT): $(OBJS) $(LIBS) $(BUNDLEDFILES) | $(BUILDDIR)
	$(CXX) $(LDFLAGS) -o $(OUTPUT) $(OBJS) $(LDLIBS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	-rm -r $(BUILDDIR)/src $(BUILDDIR)/pla $(BUILDDIR)/net

dist-clean: clean
	-rm -r build

native/librtcdcpp/librtcdcpp.a:
	cd native/librtcdcpp && $(MAKE) USE_GNUTLS=1 USE_SPDLOG=0

