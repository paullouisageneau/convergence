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

#include "pla/font.hpp"

namespace pla {

Font::Font(const std::string &filename) {
	if (FT_Init_FreeType(&mLibrary) != 0)
		throw std::runtime_error("FreeType initialization failed");

	if (FT_New_Face(mLibrary, filename.c_str(), 0, &mFace) != 0)
		throw std::runtime_error("Font loading failed: " + filename);
}

Font::~Font() {
	FT_Done_Face(mFace);
	FT_Done_FreeType(mLibrary);
}

shared_ptr<Image> Font::render(const std::string &text, size_t size, const vec3 &color) {
	if (FT_Set_Pixel_Sizes(mFace, 0, size) != 0)
		throw std::runtime_error("FreeType set size failed");

	size_t width = 0;
	size_t height = 0;
	FT_GlyphSlot slot = mFace->glyph;
	FT_Vector pen;
	pen.x = 0;
	pen.y = 0;
	for (size_t i = 0; i < text.size(); ++i) {
		FT_Set_Transform(mFace, nullptr, &pen); // nullptr = identity
		if (FT_Load_Char(mFace, text[i], FT_LOAD_RENDER) != 0)
			continue;

		width = std::max(width, size_t(slot->bitmap_left + slot->bitmap.width));
		height = std::max(height, size_t(slot->bitmap_top + slot->bitmap.rows));
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}

	width += width % 2 ? 1 : 2; // always increase to prevent clamping at the right
	height += height % 2 ? 1 : 0;

	const uint8_t rgb[3] = {uint8_t(color.x * 255), uint8_t(color.y * 255), uint8_t(color.z * 255)};
	const size_t length = width * height * 4;
	auto *data = new uint8_t[length];
	std::fill(data, data + length, 0);
	pen.x = 0;
	pen.y = 0;
	for (size_t i = 0; i < text.size(); ++i) {
		FT_Set_Transform(mFace, nullptr, &pen); // nullptr = identity
		if (FT_Load_Char(mFace, text[i], FT_LOAD_RENDER) != 0)
			continue;

		blit(slot->bitmap, slot->bitmap_left, slot->bitmap_top, rgb, data, width, height);
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}

	return std::make_shared<Image>(data, width, height);
}

void Font::blit(const FT_Bitmap &bitmap, FT_Int left, FT_Int top, const uint8_t *rgb, uint8_t *dest,
                size_t width, size_t height) {
	if (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
		throw std::runtime_error("Unexpected FreeType pixel mode");

	for (size_t i = 0; i < bitmap.width; ++i)
		for (size_t j = 0; j < bitmap.rows; ++j) {
			size_t u = left + i;
			size_t v = top - j;
			if (u < width && v < height) {
				size_t p = (v * width + u) * 4;
				dest[p + 0] = rgb[0];
				dest[p + 1] = rgb[1];
				dest[p + 2] = rgb[2];
				dest[p + 3] = bitmap.buffer[j * bitmap.width + i];
			}
		}
}

} // namespace pla

