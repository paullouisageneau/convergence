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

#include "pla/texture.hpp"

namespace pla {

Texture::Texture(GLenum type) : mType(type), mClampingEnabled(false) {
#ifndef USE_OPENGL_ES
	glEnable(mType);
#endif
	glGenTextures(1, &mTexture);
}

Texture::Texture(shared_ptr<Image> img, GLenum type) : Texture(type) { setImage(img); }

Texture::Texture(const std::string &filename, GLenum type)
    : Texture(std::make_shared<Image>(filename), type) {}

Texture::~Texture() { glDeleteTextures(1, &mTexture); }

void Texture::activate(int unit) const {
	glActiveTexture(GL_TEXTURE0 + unit);
	bind();
}

void Texture::deactivate(int unit) const {
	glActiveTexture(GL_TEXTURE0 + unit);
	unbind();
}

void Texture::bind() const {
	glBindTexture(mType, mTexture);
	glTexParameteri(mType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(mType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	if (mClampingEnabled) {
		glTexParameteri(mType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(mType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(mType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	} else {
		glTexParameteri(mType, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexParameteri(mType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(mType, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
}

void Texture::unbind() const { glBindTexture(mType, 0); }

void Texture::enableClamping(bool enabled) { mClampingEnabled = enabled; }

void Texture::setImage(shared_ptr<Image> img) {
	setImage(img->data(), img->width(), img->height());
}

void Texture::setImage(const uint8_t *data, size_t width, size_t height) {
	bind_guard guard(this);
	glTexImage2D(mType, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(mType);
}

void Texture::setImage(const uint8_t *data, size_t width, size_t height, size_t depth) {
	bind_guard guard(this);
	glTexImage3D(mType, 0, GL_RGBA, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(mType);
}

} // namespace pla

