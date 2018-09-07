/***************************************************************************
 *   Copyright (C) 2015-2018 by Paul-Louis Ageneau                         *
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
#include "src/player.hpp"

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
	mYaw = 0.f;
	mPitch = -Pi/2;
	mAccumulator = 0.f;
	
	mNetworking = std::make_shared<Networking>("ws://127.0.0.1:8080/test");
	mWorld = std::make_shared<World>(mNetworking->localId());
}

void Game::onCleanup(Engine *engine)
{
	mNetworking.reset();
	mWorld.reset();
}

bool Game::onUpdate(Engine *engine, double time)
{
	if(engine->isKeyDown(KEY_ESCAPE)) return false;

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

	sptr<Player> localPlayer = mWorld->localPlayer();
	localPlayer->rotate(mYaw, mPitch);
	localPlayer->setSpeed(speed);

	if(engine->isKeyDown(KEY_SPACE)) localPlayer->jump();

	mWorld->update(time);

	if(engine->isMouseButtonDown(MOUSE_BUTTON_LEFT) || engine->isMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		sptr<Island> island = mWorld->island();
		
		vec3 position = localPlayer->getPosition();
		vec3 front = localPlayer->getDirection();
		vec3 intersection;
		if(island->intersect(position, front*10.f, 0.25f, &intersection) <= 1.f)
		{
			bool diggingMode = engine->isMouseButtonDown(MOUSE_BUTTON_RIGHT);
			if(diggingMode) mAccumulator-= 200.f*time;
			else mAccumulator+= 200.f*time;
				
			int delta = int(mAccumulator);
			if(delta)
			{
				mAccumulator-= float(delta);
				if(diggingMode) island->dig(intersection, delta, 2.5f);
				else island->build(intersection, delta);
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

	Context context(projection, mWorld->localPlayer()->getTransform());
	context.setUniform("lightPosition", vec3(10000, 10000, 10000));

	count+= mWorld->draw(context);
	
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
