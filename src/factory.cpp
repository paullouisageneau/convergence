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

#include <cassert>
#include <filesystem>
#include <list>

namespace {

template <typename T> bool in_interval(T value, T min, T max, float &t) {
	if (value >= min && value <= max) {
		t = float(max - value) / float(max - min);
		return true;
	} else {
		return false;
	}
}

template <typename T> T interpolate(T a, T b, float t) { return a * (1.f - t) + b * t; }

} // namespace

namespace convergence {

using pla::bounds;
using pla::Image;

namespace fs = std::filesystem;

Factory::Factory(const string &name, float scale, shared_ptr<Program> program) : mProgram(program) {

	enum ImageIndexes : int { Front = 0, Back = 1, Left = 2, Right = 3, Top = 4, Bottom = 5 };
	string imageNames[6] = {"front", "back", "left", "right", "top", "bottom"};

	const fs::path basePath = fs::path("res") / fs::path(name);
	shared_ptr<Image> images[6];
	for (int i = 0; i < 6; ++i) {
		const fs::path filePath = basePath / fs::path(imageNames[i] + ".png");
		if (fs::exists(filePath)) {
			std::cout << "Loaded " << filePath << std::endl;
			images[i] = std::make_shared<Image>(filePath);
		}
	}

	for (int i = 0; i < 6; ++i) {
		if (!images[i]) {
			if (i % 2 == 0) {
				if (images[i + 1]) {
					std::cout << "Using " << imageNames[i + 1] << " for " << imageNames[i]
					          << std::endl;
					images[i] = images[i + 1];
				}
			} else {
				if (images[i - 1]) {
					std::cout << "Using " << imageNames[i - 1] << " for " << imageNames[i]
					          << std::endl;
					images[i] = images[i - 1];
				}
			}
		}
	}

	if (auto nullImage = std::find(images, images + 6, nullptr); nullImage - images < 6)
		throw std::runtime_error("Missing image: " + imageNames[nullImage - images]);

	Plane<uint84> planes[6];
	Plane<int> zplanes[6];
	for (int i = 0; i < 6; ++i) {
		planes[i] = createImagePlane(images[i]);
		zplanes[i] = Plane<int>(images[i]->width() + 2, images[i]->height() + 2, -1);
	}

	int width = 2 + std::min({
	                    images[Front]->width(), //
	                    images[Back]->width(),  //
	                    images[Top]->width(),   //
	                    images[Bottom]->width() //
	                });
	int height = 2 + std::min({
	                     images[Front]->height(), //
	                     images[Back]->height(),  //
	                     images[Left]->height(),  //
	                     images[Right]->height()  //
	                 });
	int depth = 2 + std::min({
	                    images[Left]->width(),   //
	                    images[Right]->width(),  //
	                    images[Top]->height(),   //
	                    images[Bottom]->height() //
	                });

	// First step: compute z planes
	for (int x = 0; x < width; ++x)
		for (int y = 0; y < height; ++y) {
			if (planes[Front](x, y).w) {
				for (int z = 0; z < depth; ++z) {
					if (planes[Left](z, y).w && planes[Right](z, y).w && planes[Top](x, z).w &&
					    planes[Bottom](x, z).w) {
						zplanes[Front](x, y) = z;
						break;
					}
				}
			}
			if (planes[Back](x, y).w) {
				for (int z = depth - 1; z >= 0; --z) {
					if (planes[Left](z, y).w && planes[Right](z, y).w && planes[Top](x, z).w &&
					    planes[Bottom](x, z).w) {
						zplanes[Back](x, y) = z;
						break;
					}
				}
			}
		}
	for (int z = 0; z < depth; ++z)
		for (int y = 0; y < height; ++y) {
			if (planes[Left](z, y).w) {
				for (int x = 0; x < width; ++x) {
					if (planes[Front](x, y).w && planes[Back](x, y).w && planes[Top](x, z).w &&
					    planes[Bottom](x, z).w) {
						zplanes[Left](z, y) = x;
						break;
					}
				}
			}
			if (planes[Right](z, y).w) {
				for (int x = width - 1; x >= 0; --x) {
					if (planes[Front](x, y).w && planes[Back](x, y).w && planes[Top](x, z).w &&
					    planes[Bottom](x, z).w) {
						zplanes[Right](z, y) = x;
						break;
					}
				}
			}
		}
	for (int x = 0; x < width; ++x)
		for (int z = 0; z < depth; ++z) {
			if (planes[Bottom](x, z).w) {
				for (int y = 0; y < height; ++y) {
					if (planes[Front](x, y).w && planes[Back](x, y).w && planes[Left](z, y).w &&
					    planes[Right](z, y).w) {
						zplanes[Top](x, z) = y;
						break;
					}
				}
			}
			if (planes[Top](x, z).w) {
				for (int y = height - 1; y >= 0; --y) {
					if (planes[Front](x, y).w && planes[Back](x, y).w && planes[Left](z, y).w &&
					    planes[Right](z, y).w) {
						zplanes[Bottom](x, z) = y;
						break;
					}
				}
			}
		}

	mSize = int3(width, height, depth);
	const size_t count = mSize.x * mSize.y * mSize.z;
	auto weights = new uint8_t[count];
	auto materials = new Volume::Material[count];
	std::fill(weights, weights + count, 0);
	std::fill(materials, materials + count, Volume::Material());

	int i = 0;
	for (int x = 0; x < width; ++x)
		for (int y = 0; y < height; ++y)
			for (int z = 0; z < depth; ++z) {
				int3 zpmin(zplanes[Left](z, y), zplanes[Top](x, z), zplanes[Front](x, y));
				int3 zpmax(zplanes[Right](z, y), zplanes[Bottom](x, z), zplanes[Back](x, y));
				vec3 t;
				uint84 c(0, 0, 0, 0);
				if (in_interval(x, zpmin.x, zpmax.x, t.x) &&
				    in_interval(y, zpmin.y, zpmax.y, t.y) &&
				    in_interval(z, zpmin.z, zpmax.z, t.z)) {
					// Inside the object
					std::list<uint84> l;
					if (x == zpmin.x)
						l.push_back(planes[Left](z, y));
					if (x == zpmax.x)
						l.push_back(planes[Right](z, y));
					if (y == zpmin.y)
						l.push_back(planes[Top](x, z));
					if (y == zpmax.y)
						l.push_back(planes[Bottom](x, z));
					if (z == zpmin.z)
						l.push_back(planes[Front](x, y));
					if (z == zpmax.z)
						l.push_back(planes[Back](x, y));
					if (l.size() > 0) {
						// On the border, so average raw values
						const float f = 1.f / l.size();
						for (auto v : l)
							c += v * f;
					} else {
						// Not on the border, so interpolate
						c += interpolate(planes[Left](z, y), planes[Right](z, y), t.x);
						c += interpolate(planes[Top](x, z), planes[Bottom](x, z), t.y);
						c += interpolate(planes[Front](x, y), planes[Back](x, y), t.z);
					}
				} else {
					// Outside the object
					std::list<uint84> l;
					if (x == zpmin.x - 1)
						l.push_back(planes[Left](z, y));
					if (x == zpmax.x + 1)
						l.push_back(planes[Right](z, y));
					if (y == zpmin.y - 1)
						l.push_back(planes[Top](x, z));
					if (y == zpmax.y + 1)
						l.push_back(planes[Bottom](x, z));
					if (z == zpmin.z - 1)
						l.push_back(planes[Front](x, y));
					if (z == zpmax.z + 1)
						l.push_back(planes[Back](x, y));
					if (l.size() > 0) {
						const float f = 1.f / l.size();
						for (auto v : l)
							c += v * f;
					}
					c.w = 0;
				}
				weights[i] = c.w;
				materials[i] = Volume::Material{uint84(c.x / 4, c.y / 4, c.z / 4, 255),
				                                uint84(c.x, c.y, c.z, 255), 128};

				++i;
			}

	mMesh = std::make_shared<Volume>(mSize, scale, weights, materials);

	delete[] weights;
	delete[] materials;
}

Factory::~Factory() {}

shared_ptr<Object> Factory::build() const { return std::make_shared<Object>(mMesh, mProgram); }

Factory::Plane<uint84> Factory::createImagePlane(shared_ptr<Image> img) {
	Plane<uint84> plane(img->width() + 2, img->height() + 2);
	auto d = reinterpret_cast<const uint84 *>(img->data());
	for (int x = 0; x < img->width() + 2; ++x)
		for (int y = 0; y < img->height() + 2; ++y)
			if (x != 0 && y != 0 && x != img->width() + 1 && y != img->height() + 1)
				plane(x, y) = d[(y - 1) * img->width() + (x - 1)];
			else
				plane(x, y) = uint84(0, 0, 0, 0);

	return plane;
}

} // namespace convergence
