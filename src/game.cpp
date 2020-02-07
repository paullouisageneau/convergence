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

namespace convergence {

Game::Game(void) {
	mYaw = 0.f;
	mPitch = 0.f;
	mAccumulator = 0.f;

	mUpdateCount = 0;
}

Game::~Game(void) {}

void Game::onInit(Engine *engine) {
	const string url = "ws://127.0.0.1:8080/test";

	mMessageBus = std::make_shared<MessageBus>();

	mNetworking = std::make_shared<Networking>(mMessageBus, url);
	mMessageBus->registerTypeListener(Message::Description, mNetworking);

	mWorld = std::make_shared<World>(mMessageBus);
	mMessageBus->registerTypeListener(Message::PlayerPosition, mWorld);
}

void Game::onCleanup(Engine *engine) {
	mNetworking.reset();
	mWorld.reset();
}

bool Game::onUpdate(Engine *engine, double time) {
	if (engine->isKeyDown(KEY_ESCAPE))
		return false;

	const float mouseSensitivity = 0.025f;
	double mx, my;
	engine->getMouseMove(&mx, &my);
	mYaw -= mouseSensitivity * mx;
	mPitch -= mouseSensitivity * my;
	mPitch = pla::bounds(mPitch, -Pi / 2, Pi / 2);

	const float walkSpeed = 5.f;
	float speed = 0.f;
	if (engine->isKeyDown(KEY_UP))
		speed += walkSpeed;
	if (engine->isKeyDown(KEY_DOWN))
		speed -= walkSpeed;

	sptr<Player> localPlayer = mWorld->localPlayer();
	localPlayer->rotate(mYaw, mPitch);
	localPlayer->move(speed);

	if (engine->isKeyDown(KEY_SPACE))
		localPlayer->jump();

	mWorld->update(time);

	if (engine->isMouseButtonDown(MOUSE_BUTTON_LEFT) || mAccumulator >= 0.5) {
		sptr<Terrain> terrain = mWorld->terrain();
		vec3 position = localPlayer->getPosition();
		vec3 front = localPlayer->getDirection();
		vec3 intersection;
		if (terrain->intersect(position, front * 4.f, 0.25f, &intersection) <= 1.f) {
			mAccumulator += 2. * time;
			if (mAccumulator >= 1.) {
				mAccumulator = -1.;
				terrain->dig(intersection, 50, 2.f);
				localPlayer->jolt(1.f);
			}
		}
	} else if (mAccumulator > 0.) {
		mAccumulator -= std::min(mAccumulator, 2. * time);
	}

	if (mAccumulator < 0.) {
		mAccumulator += std::min(-mAccumulator, 2. * time);
	}

	localPlayer->action(mAccumulator);

	++mUpdateCount;
	return true;
}

int Game::onDraw(Engine *engine) {
	int count = 0;

	engine->clear(vec4(0.f, 0.f, 0.f, 1.f));

	int width, height;
	engine->getWindowSize(&width, &height);

	mat4 projection =
	    glm::perspective(glm::radians(45.0f), float(width) / float(height), 0.01f, 40.0f);

	Context context(projection, mWorld->localPlayer()->getTransform());
	context.setUniform("lightPosition", mWorld->localPlayer()->getPosition());

	count += mWorld->draw(context);

	engine->clearDepth();
	mWorld->localPlayer()->draw(context);
	return count;
}

void Game::onKey(Engine *engine, int key, bool down) {}

void Game::onMouse(Engine *engine, int button, bool down) {}

void Game::onInput(Engine *engine, string text) {}

} // namespace convergence
