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

#ifndef PLA_ENGINE_H
#define PLA_ENGINE_H

#include "pla/include.hpp"
#include "pla/opengl.hpp"
#include "pla/linalg.hpp"
#include "pla/resourcemanager.hpp"
#include "pla/mediamanager.hpp"
#include "pla/string.hpp"

#include <stack>

#define MIN_FRAME_TIME  1./60.	// 60 frames/sec max

namespace pla
{

class Engine
{
	
public:
	Engine(void);
	~Engine(void);

	void openWindow(int width, int height);
    void closeWindow(void);
	void setWindowTitle(const string &title);
	void setWindowSize(int width, int height);
	void getWindowSize(int *width, int *height) const;

	void clear(const vec4 &color);
	
	bool isKeyDown(int key);
	bool isMouseButtonDown(int button);
	void getMousePosition(double *x, double *y) const;
	void getMouseMove(double *mx, double *my) const;
	double getMouseWheel(void) const;

	double getTime(void) const;
	double getTimeStamp(void) const;
	float  getFps(void) const;

	unsigned getLogicClock(void) const;
	unsigned tickLogicClock(void);
	unsigned syncLogicClock(unsigned ticks);

	class State
	{
	public:
		virtual void onInit(Engine *engine) = 0;
		virtual void onCleanup(Engine *engine) = 0;
		
		virtual bool onUpdate(Engine *engine, double time) = 0;
		virtual int  onDraw(Engine *engine) = 0;
	
		virtual void onKey(Engine *engine, int key, bool down) {}
		virtual void onMouse(Engine *engine, int button, bool down) {}
		virtual void onInput(Engine *engine, std::string text) {}
	};

	void changeState(sptr<State> state);
	void pushState(sptr<State> state);
	void popState(void);
	sptr<State> getState(void) const;
	
	sptr<ResourceManager> resourceManager(void);
	sptr<MediaManager> mediaManager(void);

	bool update(void);
	int  display(void);

private:
	GLFWwindow* mWindow;
    
	sptr<ResourceManager> mResourceManager;
	sptr<MediaManager> mMediaManager;

	// Engine states stack
	std::stack<sptr<State> > mStates;
	
	// Cursor diff
	double mOldCursorx = 0;
	double mOldCursory = 0;
	
	bool mHasFocus = false;
	
	// Time handling
	double mOldTime = 0., mMesureTime = 0.;
	unsigned long mMesureFrames = 0;
	float mFps = 0.f;
	unsigned mLogicTicks = 0;
	
	// Callbacks
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void CursorEnterCallback(GLFWwindow* window, int entered);
	static void CharCallback(GLFWwindow* window, unsigned int codepoint);
	static void WindowSizeCallback(GLFWwindow* window, int width, int height);
	static void WindowFocusCallback(GLFWwindow* window, int focus);
};

#define MOUSE_BUTTON_LEFT	GLFW_MOUSE_BUTTON_LEFT
#define MOUSE_BUTTON_RIGHT	GLFW_MOUSE_BUTTON_RIGHT
#define MOUSE_BUTTON_MIDDLE	GLFW_MOUSE_BUTTON_MIDDLE

#define KEY_SPACE			GLFW_KEY_SPACE
#define KEY_ESCAPE			GLFW_KEY_ESCAPE
#define KEY_ENTER			GLFW_KEY_ENTER
#define KEY_RETURN			GLFW_KEY_ENTER
#define KEY_TAB				GLFW_KEY_TAB
#define KEY_BACKSPACE		GLFW_KEY_BACKSPACE
#define KEY_INSERT			GLFW_KEY_INSERT
#define KEY_DELETE			GLFW_KEY_DELETE
#define KEY_RIGHT			GLFW_KEY_RIGHT
#define KEY_LEFT			GLFW_KEY_LEFT
#define KEY_DOWN			GLFW_KEY_DOWN 
#define KEY_UP				GLFW_KEY_UP
#define KEY_PAGE_UP			GLFW_KEY_PAGE_UP
#define KEY_PAGE_DOWN		GLFW_KEY_PAGE_DOWN
#define KEY_HOME			GLFW_KEY_HOME
#define KEY_END				GLFW_KEY_END
#define KEY_CAPS_LOCK		GLFW_KEY_CAPS_LOCK
#define KEY_SCROLL_LOCK		GLFW_KEY_SCROLL_LOCK
#define KEY_NUM_LOCK		GLFW_KEY_NUM_LOCK
#define KEY_PRINT_SCREEN	GLFW_KEY_PRINT_SCREEN
#define KEY_PAUSE			GLFW_KEY_PAUSE
#define KEY_F1				GLFW_KEY_F1
#define KEY_F2				GLFW_KEY_F2
#define KEY_F3				GLFW_KEY_F3
#define KEY_F4				GLFW_KEY_F4
#define KEY_F5				GLFW_KEY_F5
#define KEY_F6				GLFW_KEY_F6
#define KEY_F7				GLFW_KEY_F7
#define KEY_F8				GLFW_KEY_F8
#define KEY_F9				GLFW_KEY_F9
#define KEY_F10				GLFW_KEY_F10
#define KEY_F11				GLFW_KEY_F11
#define KEY_F12				GLFW_KEY_F12
#define KEY_KP_0			GLFW_KEY_KP_0
#define KEY_KP_1			GLFW_KEY_KP_1
#define KEY_KP_2			GLFW_KEY_KP_2
#define KEY_KP_3			GLFW_KEY_KP_3
#define KEY_KP_4			GLFW_KEY_KP_4
#define KEY_KP_5			GLFW_KEY_KP_5
#define KEY_KP_6			GLFW_KEY_KP_6
#define KEY_KP_7			GLFW_KEY_KP_7
#define KEY_KP_8			GLFW_KEY_KP_8
#define KEY_KP_9			GLFW_KEY_KP_9
#define KEY_KP_DECIMAL		GLFW_KEY_KP_DECIMAL
#define KEY_KP_DIVIDE		GLFW_KEY_KP_DIVIDE
#define KEY_KP_MULTIPLY		GLFW_KEY_KP_MULTIPLY
#define KEY_KP_SUBTRACT		GLFW_KEY_KP_SUBTRACT
#define KEY_KP_ADD			GLFW_KEY_KP_ADD
#define KEY_KP_ENTER		GLFW_KEY_KP_ENTER
#define KEY_KP_EQUAL		GLFW_KEY_KP_EQUAL
#define KEY_LEFT_SHIFT		GLFW_KEY_LEFT_SHIFT
#define KEY_LEFT_CONTROL	GLFW_KEY_LEFT_CONTROL
#define KEY_LEFT_ALT		GLFW_KEY_LEFT_ALT
#define KEY_LEFT_SUPER		GLFW_KEY_LEFT_SUPER
#define KEY_RIGHT_SHIFT		GLFW_KEY_RIGHT_SHIFT
#define KEY_RIGHT_CONTROL	GLFW_KEY_RIGHT_CONTROL
#define KEY_RIGHT_ALT		GLFW_KEY_RIGHT_ALT
#define KEY_RIGHT_SUPER		GLFW_KEY_RIGHT_SUPER
#define KEY_MENU			GLFW_KEY_MENU

}

#endif
