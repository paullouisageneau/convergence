/***************************************************************************
 *   Copyright (C) 2017-2018 by Paul-Louis Ageneau                         *
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

#include <iostream>

#include "pla/engine.hpp"
#include "src/game.hpp"

#include "server.hpp"

using convergence::Server;
using convergence::Game;

int main(int argc, char **argv)
{
	try {
		// TODO
		//Server server;
		//server.start();
		
		pla::Engine engine;
		engine.openWindow(1024, 768);
		engine.pushState(std::make_shared<Game>());
		
		while(engine.update())
		{
			//int count = engine.display();
			//std::cout << count << std::endl;
			engine.display();
		}
	}
	catch(const std::exception &e) {
		std::cout << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

