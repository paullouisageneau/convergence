/***************************************************************************
 *   Copyright (C) 2017-2020 by Paul-Louis Ageneau                         *
 *   paul-louis (at) ageneau (dot) org                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "pla/engine.hpp"
#include "src/game.hpp"

#include <iostream>
#include <memory>

using convergence::Engine;
using convergence::Game;

std::unique_ptr<Engine> engine;

void loop();

int main() {
	try {
		std::cout << "Starting..." << std::endl;
		engine = std::make_unique<Engine>();
		engine->openWindow("Convergence", 640, 480);
		engine->pushState(std::make_shared<Game>());

#ifdef __EMSCRIPTEN__
		emscripten_set_main_loop(loop, 0, 1);
#else
		loop();
#endif
	} catch (const std::exception &e) {
		std::cout << "Error: " << e.what() << std::endl;
		engine.reset();
		return 1;
	}

	engine.reset();
	return 0;
}

void loop() {
#ifdef __EMSCRIPTEN__
	try {
		if (!engine->update()) {
			emscripten_cancel_main_loop();
			return;
		}

		engine->display();

	} catch (const std::exception &e) {
		std::cout << "Error: " << e.what() << std::endl;
		emscripten_cancel_main_loop();
	}
#else
	while (engine->update())
		engine->display();
#endif
}

