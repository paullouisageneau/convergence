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

namespace convergence
{

Game::Game(void)
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

	mYaw = 0.f;
	mPitch = -Pi/2;
	mAccumulator = 0.f;
	
	mNetworking = std::make_shared<Networking>("ws://127.0.0.1:8080/test");
	mIsland = std::make_shared<Island>(unsigned(time(NULL)));
	mLocalPlayer = std::make_shared<Player>(mNetworking->localId());
}

void Game::onCleanup(Engine *engine)
{
	mNetworking.reset();
	mIsland.reset();
	mLocalPlayer.reset();
	mPlayers.clear();
}

bool Game::onUpdate(Engine *engine, double time)
{
	if(engine->isKeyDown(KEY_ESCAPE)) return false;

	mIsland->update(time);
	
	const float mouseSensitivity = 0.025f;
	double mx, my;
	engine->getMouseMove(&mx, &my);
	mYaw-= mouseSensitivity*mx;
	mPitch-= mouseSensitivity*my;
	mPitch = pla::bounds(mPitch, -Pi/2, Pi/2);

	const float walkSpeed = 5.f;
	float speed = 0.f;
	if(engine->isKeyDown(KEY_UP)) speed+= walkSpeed;
	if(engine->isKeyDown(KEY_DOWN)) speed-= walkSpeed;

	mLocalPlayer->rotate(mYaw, mPitch);
	mLocalPlayer->setSpeed(speed);

	if(engine->isKeyDown(KEY_SPACE)) mLocalPlayer->jump();
	
	for(const auto &p : mPlayers)
	{
		sptr<Player> player = p.second;
		player->update(mIsland, time);
	}
	
	mLocalPlayer->update(mIsland, time);
	
	if(engine->isMouseButtonDown(MOUSE_BUTTON_LEFT) || engine->isMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		vec3 position = mLocalPlayer->getPosition();
		vec3 front = mLocalPlayer->getDirection();
		vec3 intersection;
		if(mIsland->intersect(position, front*10.f, 0.25f, &intersection) <= 1.f)
		{
			bool diggingMode = engine->isMouseButtonDown(MOUSE_BUTTON_RIGHT);
			if(diggingMode) mAccumulator-= 200.f*time;
			else mAccumulator+= 200.f*time;
				
			int delta = int(mAccumulator);
			if(delta)
			{
				mAccumulator-= float(delta);
				if(diggingMode) mIsland->dig(intersection, delta, 2.5f);
				else mIsland->build(intersection, delta);
			}
		}
	}
	
	return true;
}

int Game::onDraw(Engine *engine)
{
	int count = 0;

	engine->clear(vec4(0.9f, 0.9f, 0.9f, 1.f));

	int width, height;
	engine->getWindowSize(&width, &height);

	mat4 projection = glm::perspective(
		glm::radians(45.0f),
		float(width)/float(height),
		0.1f, 60.0f
	);

	Context context(projection, mLocalPlayer->getTransform());
	context.setUniform("lightPosition", vec3(10000, 10000, 10000));

	count+= mIsland->draw(context);

	for(const auto &p : mPlayers)
	{
		sptr<Player> player = p.second;
		count+= player->draw(context);
	}
	
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
