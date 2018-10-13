
CC=emcc
CXX=em++
RM=rm -f
EMFLAGS=-D USE_OPENGL_ES -s USE_GLFW=3 -s TOTAL_MEMORY=256MB -s BINARYEN_METHOD=native-wasm -s DISABLE_EXCEPTION_CATCHING=0 -s BINARYEN_TRAP_MODE='clamp'
CPPFLAGS=-std=c++14 -g -O2 $(EMFLAGS)
LDFLAGS=-g -O0 $(EMFLAGS)
LDLIBS=-lpthread -lGL -lglfw --js-library webrtc.js --js-library websocket.js --preload-file ../shader

SRCS=$(shell printf "%s " net/*.cpp ../pla/*.cpp ../src/*.cpp *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

OUTPUT=convergence.html

all: $(OUTPUT)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -I. -I.. -I../glm -MMD -MP -o $@ -c $<

-include $(subst .o,.d,$(OBJS))

$(OUTPUT): $(OBJS) webrtc.js websocket.js ../shader/*
	$(CXX) $(LDFLAGS) -o $(OUTPUT) $(OBJS) $(LDLIBS) 

clean:
	$(RM) net/*.o net/*.d
	$(RM) ../pla/*.o ../pla/*.d ../src/*.o ../src/*.d *.o *.d

dist-clean: clean
	$(RM) $(OUTPUT)
	$(RM) net/*~
	$(RM) ../pla/*~ ../src/*~
