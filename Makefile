
native:
	cd native && $(MAKE)

emscripten:
	cd emscripten && $(MAKE)

clean:
	cd native && $(MAKE) clean
	cd emscripten && $(MAKE) clean

dist-clean:
	cd native && $(MAKE) dist-clean
	cd emscripten && $(MAKE) dist-clean

.PHONY: native emscripten
