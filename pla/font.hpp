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

#ifndef PLA_FONT_H
#define PLA_FONT_H

#include "pla/image.hpp"
#include "pla/include.hpp"
#include "pla/linalg.hpp"

#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace pla {

class Font {
public:
	Font(const std::string &filename);
	virtual ~Font();

	shared_ptr<Image> render(const std::string &text, size_t size, const vec3 &color);

private:
	static void blit(const FT_Bitmap &bitmap, FT_Int left, FT_Int top, const uint8_t *rgb,
	                 uint8_t *dest, size_t width, size_t height);

	FT_Library mLibrary;
	FT_Face mFace;
};

} // namespace pla

#endif // PLA_FONT_H
