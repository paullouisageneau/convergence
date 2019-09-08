
#include <emscripten/emscripten.h>
#include <iostream>

#include "pla/engine.hpp"
#include "src/game.hpp"

#include "net/webrtc.hpp"

void loop();

pla::Engine engine;

int main() {
	try {
		std::cout << "Started..." << std::endl;
		engine.openWindow(640, 480);
		engine.pushState(std::make_shared<convergence::Game>());
	} catch (const std::exception &e) {
		std::cout << "Error: " << e.what() << std::endl;
		return 1;
	}

	emscripten_set_main_loop(loop, 0, 1);
	return 0;
}

void loop() {
	try {
		if (!engine.update()) {
			emscripten_cancel_main_loop();
			return;
		}

		// int count = engine.display();
		// std::cout << count << std::endl;
		engine.display();
	} catch (const std::exception &e) {
		std::cout << "Error: " << e.what() << std::endl;
		emscripten_cancel_main_loop();
	}
}
