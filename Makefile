NAME=convergence

CPPFLAGS=-std=c++14 -g -O1
LDFLAGS=-g -O1
LDLIBS=-lpthread -lGL -lglfw
INCLUDE=-I. -Iglm
BUNDLE=shader
EMFLAGS=-D USE_OPENGL_ES -s USE_GLFW=3 -s TOTAL_MEMORY=256MB -s BINARYEN_METHOD=native-wasm -s DISABLE_EXCEPTION_CATCHING=0 -s BINARYEN_TRAP_MODE='clamp'

BUILDDIR=build/$(ARCH)
OBJDIR=$(BUILDDIR)
OUTPUT=$(BUILDDIR)/$(NAME)

ARCH:=$(shell $(CC) -dumpmachine|cut -f 1 -d -)
SRCS:=$(shell printf "%s " pla/*.cpp src/*.cpp)
JSLIBS:=$(shell printf "%s " emscripten/js/*.js)
BUNDLEDFILES:=$(shell printf "%s " $(addsuffix /*,$(BUNDLE)))

ifeq ($(notdir $(CXX)),em++)
DIR:=emscripten
CPPFLAGS+=$(EMFLAGS)
LDFLAGS+=$(EMFLAGS)
LDLIBS+=$(addprefix --js-library ,$(JSLIBS))
LDLIBS+=$(addprefix --preload-file ,$(BUNDLE))
OUTPUT:=$(OUTPUT).html
else
DIR:=native
endif

INCLUDE+=-I$(DIR)
SRCS+=$(shell printf "%s " $(DIR)/**/*.cpp $(DIR)/*.cpp)
OBJS:=$(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

all: $(OUTPUT)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(INCLUDE) -MMD -MP -c -o $@ $<

-include $(subst .o,.d,$(OBJS))

$(OUTPUT): $(OBJS) $(JSLIBS) $(BUNDLEDFILES) | $(BUILDDIR)
	$(CXX) $(LDFLAGS) -o $(OUTPUT) $(OBJS) $(LDLIBS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	-rm -f $(BUILDDIR)/**/*.o $(BUILDDIR)/**/*.d

dist-clean: clean
	-rm -rf build
