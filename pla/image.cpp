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

#include "pla/image.hpp"

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL_image.h>
#else
#include <IL/il.h>
#endif

namespace pla {

Image::Image(const std::string &filename) {
#ifdef __EMSCRIPTEN__
	SDL_Surface *image = IMG_Load(filename.c_str());
	if (!image)
		throw std::runtime_error("Failed to load image: " + filename);

	// Pixel format is reversed because of endianess
	SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
	SDL_Surface *converted = SDL_ConvertSurface(image, format, 0);
	SDL_FreeFormat(format);
	SDL_FreeSurface(image);

	if (!converted)
		throw std::runtime_error("Failed to convert image: " + filename);

	mWidth = converted->w;
	mHeight = converted->h;
	const size_t size = mWidth * mHeight * 4;
	mData = new uint8_t[size];
	uint8_t *dest = mData;
	const uint8_t *src = reinterpret_cast<uint8_t *>(converted->pixels);
	for (size_t i = 0; i < mHeight; ++i) {
		std::copy(src, src + mWidth * 4, dest);
		dest += mWidth * 4;
		src += converted->pitch;
	}
	SDL_FreeSurface(converted);
#else
	ILuint id = 0;
	ilGenImages(1, &id);
	ilBindImage(id);

	if (!ilLoadImage(filename.c_str()))
		throw std::runtime_error("Failed to load image: " + filename);

	mWidth = ilGetInteger(IL_IMAGE_WIDTH);
	mHeight = ilGetInteger(IL_IMAGE_HEIGHT);
	const size_t size = mWidth * mHeight * 4;
	mData = new uint8_t[size];
	ilCopyPixels(0, 0, 0, mWidth, mHeight, 1, IL_RGBA, IL_UNSIGNED_BYTE, mData);

	ilBindImage(0);
	ilDeleteImage(id);
#endif
}

Image::Image(uint8_t *data, size_t width, size_t height, bool ownership)
    : mWidth(width), mHeight(height) {
	if (ownership) {
		mData = data;
	} else {
		const size_t size = mWidth * mHeight * 4;
		mData = new uint8_t[size];
		std::copy(data, data + size, mData);
	}
}

Image::~Image() { delete[] mData; }

size_t Image::width() const { return mWidth; }

size_t Image::height() const { return mHeight; }

const uint8_t *Image::data() const { return mData; }

} // namespace pla

