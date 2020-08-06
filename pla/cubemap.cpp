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

#include "pla/cubemap.hpp"

namespace pla {

DepthCubeMap::DepthCubeMap(size_t size) : Texture(GL_TEXTURE_CUBE_MAP), mSize(size) {
	bind();
	for (int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24, mSize, mSize, 0,
		             GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(mType, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	unbind();

	glGenFramebuffers(1, &mFramebuffer);
}

DepthCubeMap::~DepthCubeMap() { glDeleteFramebuffers(1, &mFramebuffer); }

void DepthCubeMap::bindFramebuffer(int face) {
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
	                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Framebuffer status check failed");

	glViewport(0, 0, mSize, mSize);
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void DepthCubeMap::unbindFramebuffer() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

} // namespace pla
