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

#ifndef PLA_OPENGL_H
#define PLA_OPENGL_H

#ifdef USE_OPENGL_ES
#define GLFW_INCLUDE_ES2
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <memory>

namespace pla {

template <typename T> class bind_guard {
public:
	bind_guard(const T *bound) : mBound(bound) { mBound->bind(); }
	bind_guard(std::shared_ptr<T> bound) : bind_guard(bound.get()) {}
	~bind_guard() { mBound->unbind(); }

private:
	const T *mBound;
};

} // namespace pla

#endif // PLA_OPENGL_H
