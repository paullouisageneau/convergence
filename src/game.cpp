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

#include "src/game.hpp"

using pla::Pi;
using pla::Epsilon;
using pla::to_hex;

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat2;
using glm::mat3;
using glm::mat4;

namespace convergence
{

Game::Game(void) :
	mIsland(unsigned(time(NULL)))
{

}

Game::~Game(void)
{

}

void Game::onInit(Engine *engine)
{
	/*
	mColorProgram = std::make_shared<Program>(
		std::make_shared<VertexShader>("shader/color.vert"),
		std::make_shared<FragmentShader>("shader/color.frag")
	);

	mColorProgram->bindAttribLocation(0, "position");
	mColorProgram->bindAttribLocation(1, "normal");
	mColorProgram->bindAttribLocation(2, "color");
	mColorProgram->link();
	*/

	mPosition.x = 5.f;
	mPosition.y = 5.f;
	mPosition.z = 60.f;
	mYaw = 0.f;
	mPitch = -Pi/2;
	mGravity = 0.f;
	mAccumulator = 0.f;
	
	mNetworking = std::make_shared<Networking>("ws://127.0.0.1:8080/test");
}

void Game::onCleanup(Engine *engine)
{

}

bool Game::onUpdate(Engine *engine, double time)
{
	if(engine->isKeyDown(KEY_ESCAPE))
		return false;

	mIsland.update(time);

	const float mouseSensitivity = 0.025f;
	double mx, my;
	engine->getMouseMove(&mx, &my);
	mYaw-= mouseSensitivity*mx;
	mPitch-= mouseSensitivity*my;
	mPitch = pla::bounds(mPitch, -Pi/2, Pi/2);

	vec3 move;
	mGravity+= 10.f*time;
	move.z-= mGravity*time;

	const float speed = 10.f;
	vec3 dir = vec3(std::sin(-mYaw), std::cos(-mYaw), 0.f);
	if(engine->isKeyDown(KEY_UP))
		move+= dir*float(time)*speed;
	if(engine->isKeyDown(KEY_DOWN))
		move-= dir*float(time)*speed;

	mat4 camera = mat4(1.0f);
	camera = glm::translate(camera, mPosition);
	camera = glm::rotate(camera, Pi/2, vec3(1, 0, 0));
	camera = glm::rotate(camera, mYaw, vec3(0, 1, 0));
	camera = glm::rotate(camera, mPitch, vec3(1, 0, 0));

	if(engine->isMouseButtonDown(MOUSE_BUTTON_LEFT) || engine->isMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		vec3 front = vec3(camera*vec4(0, 0, -1, 0));
		vec3 intersection;
		if(mIsland.intersect(mPosition, front*20.f, 0.25f, &intersection) <= 1.f)
		{
			bool diggingMode = engine->isMouseButtonDown(MOUSE_BUTTON_RIGHT);
			if(diggingMode) mAccumulator-= 400.f*time;
			else mAccumulator+= 200.f*time;
				
			int delta = int(mAccumulator);
			if(delta)
			{
				mAccumulator-= float(delta);
				if(diggingMode) mIsland.dig(intersection, delta, 2.5f);
				else mIsland.build(intersection, delta);
			}
		}
	}

	vec3 newmove, intersection, normal;
	if(mIsland.collide(mPosition - vec3(0.f, 0.5f, 0.f), move, 1.f, &newmove, &intersection, &normal))
	{
		move = newmove;

		if(normal.z > 0.f)
		{
			mGravity = 0.f;

			// Jump
			if(engine->isKeyDown(KEY_SPACE))
			{
				mGravity = -10.f;
			}
		}
	}

	mPosition+= move;
	return true;
}

int Game::onDraw(Engine *engine)
{
	int count = 0;

	engine->clear(vec4(0.53f, 0.81f, 0.92f, 1.f));

	int width, height;
	engine->getWindowSize(&width, &height);

	mat4 projection = glm::perspective(
		glm::radians(45.0f),
		float(width)/float(height),
		0.1f, 60.0f
	);

	mat4 camera = mat4(1.0f);
	camera = glm::translate(camera, mPosition);
	camera = glm::rotate(camera, Pi/2, vec3(1, 0, 0));
	camera = glm::rotate(camera, mYaw, vec3(0, 1, 0));
	camera = glm::rotate(camera, mPitch, vec3(1, 0, 0));

	Context context(projection, camera);
	context.setUniform("lightPosition", vec3(10000, 10000, 10000));

	count+= mIsland.draw(context);

	return count;
}

void Game::onKey(Engine *engine, int key, bool down)
{

}

void Game::onMouse(Engine *engine, int button, bool down)
{

}

void Game::onInput(Engine *engine, string text)
{

}

}
