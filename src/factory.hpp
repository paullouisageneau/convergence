/***************************************************************************
 *   Copyright (C) 2015-2020 by Paul-Louis Ageneau                         *
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

#ifndef CONVERGENCE_FACTORY_H
#define CONVERGENCE_FACTORY_H

#include "src/include.hpp"
#include "src/types.hpp"

#include "pla/image.hpp"
#include "pla/mesh.hpp"
#include "pla/object.hpp"
#include "pla/program.hpp"

namespace convergence {

using pla::Mesh;
using pla::Object;
using pla::Program;

class Factory {
public:
	Factory(const string &filename, float scale, shared_ptr<Program> program);
	virtual ~Factory();

	shared_ptr<Object> build() const;

private:
	template <typename T> class Plane {
	public:
		Plane() : mWidth(0), mHeight(0), mValues(nullptr) {}
		Plane(Plane &&p) : Plane() { *this = std::move(p); }
		Plane(size_t width, size_t height)
		    : mWidth(width), mHeight(height), mValues(new T[width * height]) {}
		Plane(size_t width, size_t height, const T &value) : Plane(width, height) {
			std::fill(mValues, mValues + width * height, value);
		}
		~Plane() { delete[] mValues; }

		size_t width() const { return mWidth; }
		size_t height() const { return mHeight; }

		T &operator()(size_t x, size_t y) { return mValues[x * mHeight + y]; }
		const T &operator()(size_t x, size_t y) const { return mValues[y * mHeight + y]; }

		Plane &operator=(Plane &&p) {
			std::swap(mWidth, p.mWidth), std::swap(mHeight, p.mHeight);
			std::swap(mValues, p.mValues);
			return *this;
		}

	private:
		size_t mWidth, mHeight;
		T *mValues;
	};

	static Plane<uint84> createImagePlane(shared_ptr<pla::Image> img);

	shared_ptr<Program> mProgram;
	shared_ptr<Mesh> mMesh;
	int3 mSize;
};

} // namespace convergence

#endif
