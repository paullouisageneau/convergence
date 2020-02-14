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

#ifndef PLA_TEXTURE_H
#define PLA_TEXTURE_H

#include "pla/image.hpp"
#include "pla/include.hpp"
#include "pla/linalg.hpp"
#include "pla/opengl.hpp"

#include <string>

namespace pla {

class Texture {
public:
	Texture(GLenum type = GL_TEXTURE_2D);
	Texture(shared_ptr<Image> img, GLenum type = GL_TEXTURE_2D);
	Texture(const std::string &filename, GLenum type = GL_TEXTURE_2D);
	virtual ~Texture();

	void activate(int unit) const;
	void deactivate(int unit) const;

	void bind() const;
	void unbind() const;

	void setImage(shared_ptr<Image> img);
	void setImage(const uint8_t *data, size_t width);
	void setImage(const uint8_t *data, size_t width, size_t height);
	void setImage(const uint8_t *data, size_t width, size_t height, size_t depth);

private:
	GLuint mTexture;
	GLenum mType;
};

} // namespace pla

#endif // PLA_TEXTURE_H
