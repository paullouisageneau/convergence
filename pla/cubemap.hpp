/***************************************************************************
 *   Copyright (C) 2006-2020 by Paul-Louis Ageneau                         *
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

#ifndef PLA_CUBEMAP_H
#define PLA_CUBEMAP_H

#include "pla/include.hpp"
#include "pla/texture.hpp"

namespace pla {

class DepthCubeMap final : public Texture {
public:
	DepthCubeMap(size_t size = 1024);
	virtual ~DepthCubeMap();

	void bindFramebuffer(int face);
	void unbindFramebuffer();

private:
	GLuint mFramebuffer;
	size_t mSize;
};

} // namespace pla

#endif
