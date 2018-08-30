/***************************************************************************
 *   Copyright (C) 2015-2016 by Paul-Louis Ageneau                         *
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

#ifndef CONVERGENCE_GAME_H
#define CONVERGENCE_GAME_H

#include "src/include.hpp"
#include "src/island.hpp"
#include "src/networking.hpp"

#include "pla/engine.hpp"
#include "pla/context.hpp"
#include "pla/program.hpp"
#include "pla/shader.hpp"

namespace convergence
{

using pla::string;
using pla::Engine;
using pla::Context;
using pla::Program;
using pla::VertexShader;
using pla::FragmentShader;
using std::shared_ptr;

template<typename T> using sptr = shared_ptr<T>;

class Game : public Engine::State
{
public:
	Game(void);
	~Game(void);

	void onInit(Engine *engine);
	void onCleanup(Engine *engine);
		
	bool onUpdate(Engine *engine, double time);
	int  onDraw(Engine *engine);
	
	void onKey(Engine *engine, int key, bool down);
	void onMouse(Engine *engine, int button, bool down);
	void onInput(Engine *engine, string text);

private:
	Island mIsland;
	
	sptr<Networking> mNetworking;
	
	vec3 mPosition;
	float mYaw, mPitch;
	float mGravity;
	float mAccumulator;
};

}

#endif

