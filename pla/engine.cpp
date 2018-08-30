/***************************************************************************
 *   Copyright (C) 2006-2016 by Paul-Louis Ageneau                         *
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

#include "pla/engine.hpp"
#include "pla/mediamanager.hpp"

namespace pla
{

Engine::Engine(void) : 
	mWindow(NULL)
{
	// GLFW initialization
	if (!glfwInit()) throw std::runtime_error("GLFW initialization failed");

	mResourceManager = std::make_shared<ResourceManager>();
	mMediaManager    = std::make_shared<MediaManager>(mResourceManager);
}

Engine::~Engine(void)
{
	// Clear states stack
	while(!mStates.empty())
		popState();

	// Close window
	closeWindow();

	// GLFW termination
	glfwTerminate();
}

void Engine::openWindow(int width, int height)
{
	// Create window and OpenGL context
	mWindow = glfwCreateWindow(width, height, "", NULL, NULL);
	if (!mWindow) throw std::runtime_error("Window creation failed");

	glfwSetWindowUserPointer(mWindow, this);
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetKeyCallback(mWindow, KeyCallback);
	glfwSetMouseButtonCallback(mWindow, MouseButtonCallback);
	glfwSetCursorPosCallback(mWindow, CursorPosCallback);
	glfwSetScrollCallback(mWindow, ScrollCallback);
	glfwSetCursorEnterCallback(mWindow, CursorEnterCallback);
	glfwSetCharCallback(mWindow, CharCallback);
	glfwSetWindowSizeCallback(mWindow, WindowSizeCallback);

	glfwMakeContextCurrent(mWindow);

#ifndef USE_OPENGL_ES
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	getMousePosition(&mOldCursorx, &mOldCursory);
}

void Engine::closeWindow(void)
{
	if(mWindow)
	{
		glfwDestroyWindow(mWindow);
		mWindow = NULL;
	}
}

void Engine::setWindowTitle(const string &title)
{
	glfwSetWindowTitle(mWindow, title.c_str());
}

void Engine::setWindowSize(int width, int height)
{
	glfwSetWindowSize(mWindow, width, height);
}

void Engine::getWindowSize(int *width, int *height) const
{
	glfwGetWindowSize(mWindow, width, height);
}

void Engine::clear(const vec4 &color)
{
	glClearColor(color.x, color.y, color.z, color.w);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Engine::changeState(sptr<State> state)
{
	popState();
	pushState(state);
}

void Engine::pushState(sptr<State> state)
{
	mStates.push(state);
	state->onInit(this);
	mMesureTime = mOldTime = getTime();
	mMesureFrames=0;
}

void Engine::popState(void)
{
	if(mStates.empty()) return;
	
	mStates.top()->onCleanup(this);
	mStates.pop();

	mMesureTime = mOldTime = getTime();
	mMesureFrames = 0;
}

sptr<Engine::State> Engine::getState(void) const
{
	if(mStates.empty()) return NULL;
	else return mStates.top();
}

sptr<ResourceManager> Engine::resourceManager(void)
{
    return mResourceManager;
}

sptr<MediaManager> Engine::mediaManager(void)
{
	return mMediaManager;
}

bool Engine::update(void)
{
	if(mStates.empty()) return false;
	
	glfwPollEvents();

	double currentTime = getTime();				// Current time
	double elapsed = currentTime - mOldTime;	// Time since last frame
	
	if(elapsed < MIN_FRAME_TIME)
	{
		sleep(MIN_FRAME_TIME-elapsed);
		currentTime = getTime();
		elapsed = currentTime - mOldTime;
	}
	
	mOldTime = currentTime;
	
	while(!mStates.top()->onUpdate(this, elapsed))	// Update current state
	{
		popState();	// If finished, remove it from stack
		if(mStates.empty()) return false;
	}
	
	getMousePosition(&mOldCursorx, &mOldCursory);

	
	// Process sound
	//Sound::Process();
	return true;
}

int Engine::display(void)
{
	if(mStates.empty()) return 0;
	
	glClear(GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	int count = mStates.top()->onDraw(this);
	glfwSwapBuffers(mWindow);

	// Compute FPS
	++mMesureFrames;
	if(mMesureFrames > 10)
	{
		mFps=mMesureFrames/(getTime()-mMesureTime);
		mMesureTime=getTime();
		mMesureFrames=0;
	}
	
	return count;
}

bool Engine::isKeyDown(int key)
{
	return glfwGetKey(mWindow, key) == GLFW_PRESS;
}

bool Engine::isMouseButtonDown(int button)
{
	return glfwGetMouseButton(mWindow, button) == GLFW_PRESS;
}

void Engine::getMousePosition(double *x, double *y) const
{
	glfwGetCursorPos(mWindow, x, y);
}

void Engine::getMouseMove(double *mx, double *my) const
{
	double x, y;
	glfwGetCursorPos(mWindow, &x, &y);
	if(x) *mx = x - mOldCursorx;
	if(y) *my = y - mOldCursory;
}

double Engine::getMouseWheel(void) const
{
	// TODO
	return 0.;
}

double Engine::getTime(void) const
{
	return glfwGetTime();
}

double Engine::getTimeStamp(void) const
{
	return mOldTime;
}

float Engine::getFps(void) const
{
	return mFps;
}

unsigned Engine::getLogicClock(void) const
{
	return mLogicTicks;
}

unsigned Engine::tickLogicClock(void)
{
	mLogicTicks++;
	return mLogicTicks;
}

unsigned Engine::syncLogicClock(unsigned ticks)
{
	mLogicTicks = std::max(ticks, mLogicTicks);
	return mLogicTicks;
}

void Engine::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Engine *engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
	if(!engine) return;

}

void Engine::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	Engine *engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
	if(!engine) return;

}

void Engine::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	Engine *engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
	if(!engine) return;

}

void Engine::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Engine *engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
	if(!engine) return;

}

void Engine::CursorEnterCallback(GLFWwindow* window, int entered)
{
	Engine *engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
	if(!engine) return;

}

void Engine::CharCallback(GLFWwindow* window, unsigned int codepoint)
{
	Engine *engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
	if(!engine) return;

}

void Engine::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

}
