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
#include "src/light.hpp"
#include "src/player.hpp"

#include <array>
#include <vector>

namespace convergence {

using pla::Quad;
using std::vector;

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

	mWorld = std::make_shared<World>(mMessageBus);
	mMessageBus->registerListener(Message::EntityTransform, mWorld);

	auto program = std::make_shared<Program>(std::make_shared<VertexShader>("shader/font.vect"),
	                                         std::make_shared<FragmentShader>("shader/font.frag"));

	auto font = std::make_shared<Font>("res/ttf/DejaVuSansMono.ttf");
	mMessages.push_back(std::make_shared<Text>(font, program, "Hello world"));

	mDepthProgram =
	    std::make_shared<Program>(std::make_shared<VertexShader>("shader/depth.vect"),
	                              std::make_shared<FragmentShader>("shader/depth.frag"));
}

void Game::onCleanup(Engine *engine) {
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
	localPlayer->pivot(mYaw, mPitch);
	localPlayer->walk(speed);

	if (engine->isKeyDown(KEY_SPACE))
		localPlayer->jump();

	if (engine->isKeyDown(KEY_RETURN)) {
		if (!mReturnPressed) {
			mReturnPressed = true;
			if (!localPlayer->isPicking())
				mWorld->localPick();
			else
				localPlayer->release();
		}
	} else {
		mReturnPressed = false;
	}

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
	Light::Collection lights;
	mWorld->collect(lights);
	for (auto l : lights.vector()) {
		vec3 pos = l->position();
		vector<mat4> transforms(6);
		transforms[0] = glm::lookAt(pos, pos + vec3(1.f, 0.f, 0.f), vec3(0.f, -1.f, 0.f));
		transforms[1] = glm::lookAt(pos, pos + vec3(-1.f, 0.f, 0.f), vec3(0.f, -1.f, 0.f));
		transforms[2] = glm::lookAt(pos, pos + vec3(0.f, 1.f, 0.f), vec3(0.f, 0.f, 1.f));
		transforms[3] = glm::lookAt(pos, pos + vec3(0.f, -1.f, 0.f), vec3(0.f, 0.f, -1.f));
		transforms[4] = glm::lookAt(pos, pos + vec3(0.f, 0.f, 1.f), vec3(0.f, -1.f, 0.f));
		transforms[5] = glm::lookAt(pos, pos + vec3(0.f, 0.f, -1.f), vec3(0.f, -1.f, 0.f));

		for (int i = 0; i < 6; ++i) {
			l->bindDepth(i);
			engine->clear(vec4(0.f, 0.f, 0.f, 1.f));

			mat4 proj = glm::perspective(glm::radians(90.0f), 1.f, 0.01f, 10.f);
			mat4 camera = glm::inverse(transforms[i]);
			Context context(proj, camera);
			context.setOverrideProgram(mDepthProgram);
			context.setUniform("nearPlane", 0.01f);
			context.setUniform("farPlane", 10.f);
			context.setUniform("lightPosition", l->position());

			count += mWorld->draw(context);

			l->unbindDepth();
		}
	}

	engine->clear(vec4(0.f, 0.f, 0.f, 1.f));

	int width, height;
	engine->getWindowSize(&width, &height);

	glViewport(0, 0, width, height);
	float ratio = float(width) / float(height);
	mat4 proj = glm::perspective(glm::radians(45.0f), ratio, 0.01f, 40.f);
	Context context(proj, mWorld->localPlayer()->getTransform());
	context.setUniform("nearPlane", 0.01f);
	context.setUniform("farPlane", 10.f);
	context.setUniform("border", 0.02f);

	context.setUniform("lightsCount", lights.count());
	context.setUniform("lightsPositions", lights.positions());
	context.setUniform("lightsColors", lights.colors());
	context.setUniform("lightsPowers", lights.powers());
	context.setUniform("lightsDepthCubeMaps", lights.depthCubeMaps());

	count += mWorld->draw(context);

	engine->clearDepth();
	mWorld->localPlayer()->draw(context);

	const float size = 20.f; // em
	mat4 hudProj = glm::ortho(-size * ratio, size * ratio, -size, size, -size, size);
	Context hudContext(hudProj, glm::translate(glm::mat4(1.f), glm::vec3(size * ratio, size, 0.f)));
	for (auto text : mMessages) {
		text->draw(hudContext.transform(glm::translate(glm::mat4(1.f), glm::vec3(0.5f, 1.f, 0.f))));
	}

	return count;
}

void Game::onKey(Engine *engine, int key, bool down) {}

void Game::onMouse(Engine *engine, int button, bool down) {}

void Game::onInput(Engine *engine, string text) {}

} // namespace convergence
