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

#include "src/factory.hpp"
#include "src/types.hpp"
#include "src/volume.hpp"

#include "pla/image.hpp"

namespace convergence {

using pla::bounds;
using pla::Image;

Factory::Factory(const string &filename, float scale, shared_ptr<Program> program)
    : mProgram(program) {
	auto image = std::make_shared<Image>(filename);

	const int3 size(image->width() + 2, image->height() + 2, 3);
	const size_t count = size.x * size.y * size.z;
	const uint8_t *data = image->data();

	auto weights = new uint8_t[count];
	auto materials = new Volume::Material[count];

	int i = 0;
	for (int x = 0; x < size.x; ++x)
		for (int y = 0; y < size.y; ++y)
			for (int z = 0; z < size.z; ++z) {
				int p = bounds(y - 1, 0, int(image->height() - 1)) * image->width() +
				        bounds(x - 1, 0, int(image->width() - 1));
				uint8_t alpha = data[p * 4 + 3];
				uint84 ambient(data[p * 4] / 2, data[p * 4 + 1] / 2, data[p * 4 + 2] / 2, 255);
				uint84 diffuse(data[p * 4], data[p * 4 + 1], data[p * 4 + 2], 255);

				if (x == 0 || x == size.x - 1 || y == 0 || y == size.y - 1 || z == 0 ||
				    z == size.z - 1)
					weights[i] = 0;
				else
					weights[i] = alpha;
				materials[i] = {ambient, diffuse, 128};
				++i;
			}

	mMesh = std::make_shared<Volume>(size, scale, weights, materials);

	delete[] weights;
	delete[] materials;
}

Factory::~Factory() {}

shared_ptr<Object> Factory::build() const { return std::make_shared<Object>(mMesh, mProgram); }

} // namespace convergence

