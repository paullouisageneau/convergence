/***************************************************************************
 *   Copyright (C) 2015-2016 by Paul-Louis Ageneau                         *
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

#include "src/world.hpp"

#include "pla/binaryformatter.hpp"

namespace convergence {

using namespace std::placeholders;

Terrain::Terrain(shared_ptr<Store> store, int seed)
    : Merkle(store), mPerlin(seed), mSurface(std::bind(&Terrain::getBlock, this, _1)) {}

Terrain::~Terrain(void) {}

Surface::value Terrain::addWeight(const int3 &p, int weight, int newType) {
	int3 b = Block::blockCoord(p);
	int3 c = Block::cellCoord(p);

	shared_ptr<Block> block = getBlock(b);
	Surface::value v = block->getValue(c);
	weight = pla::bounds(int(v.weight) + weight, 0, 255);
	v.weight = uint8_t(weight);
	if (newType >= 0)
		v.type = uint8_t(newType);
	block->writeValue(c, v);
	return v;
}

void Terrain::update(double time) { mSurface.update(time); }

int Terrain::draw(const Context &context) {
	return mSurface.draw(context);
}

float Terrain::intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection) {
	return mSurface.intersect(pos, move, radius, intersection);
}

void Terrain::build(const vec3 &p, int weight) {
	if (weight == 0)
		return;

	const int3 i(p + vec3(0.5f));
	const int type = 0;

	addWeight(i, weight, type);
}

void Terrain::dig(const vec3 &p, int weight, float radius) {
	if (weight == 0 || radius <= 0.f)
		return;

	const int3 origin = p + vec3(0.5f);
	const int d = int(radius) + 1;

	for (int dx = -d; dx <= d + 1; ++dx) {
		for (int dy = -d; dy <= d + 1; ++dy) {
			for (int dz = -d; dz <= d + 1; ++dz) {
				int3 i = origin + int3(dx, dy, dz);
				vec3 c = vec3(i.x, i.y, i.z) + vec3(0.5f);
				float t = 1.f - glm::distance(p, c) / radius;
				if (t > 0.f) {
					int w = int(weight * t);
					addWeight(i, w, -1);
				}
			}
		}
	}
}

bool Terrain::processData(const Index &index, const binary &data) {
	TerrainIndex terrainIndex(index);
	auto block = getBlock(terrainIndex.position());
	return block->update(data);
}

sptr<Terrain::Block> Terrain::getBlock(const int3 &b) {
	if (auto it = mBlocks.find(b); it != mBlocks.end())
		return it->second;

	auto block = std::make_shared<Block>(this, b);
	mBlocks[b] = block;
	populateBlock(block);
	return block;
}

Surface::value Terrain::getValue(const int3 &p) {
	sptr<Block> block = getBlock(Block::blockCoord(p));
	return block->readValue(Block::cellCoord(p));
}

void Terrain::setValue(const int3 &p, Surface::value v) {
	sptr<Block> block = getBlock(Block::blockCoord(p));
	block->writeValue(Block::cellCoord(p), v);
}

void Terrain::setType(const int3 &p, uint8_t t) {
	sptr<Block> block = getBlock(Block::blockCoord(p));
	int3 c = Block::cellCoord(p);
	Surface::value v = block->readValue(c);
	v.type = t;
	block->writeValue(c, v);
}

void Terrain::populateBlock(sptr<Block> block) {
	// Layer 0
	const float f1 = 0.15f;
	const float f2 = 0.03f;
	for (int x = 0; x < Block::Size; ++x) {
		for (int y = 0; y < Block::Size; ++y) {
			bool inside = false;
			for (int z = -1; z < Block::Size; ++z) {
				int3 pos = block->position();
				const float ax = float(pos.x * Block::Size + x);
				const float ay = float(pos.y * Block::Size + y);
				const float az = float(pos.z * Block::Size + z);
				const float d2 = ax * ax + ay * ay + az * az;
				const float noise1 = mPerlin.noise(ax * f1, ay * f1, az * f1 * 0.1f);
				const float noise2 = mPerlin.noise(ax * f2, ay * f2, az * f2 * 4.f);
				const float noise =
				    noise1 * noise1 * 0.53f + (noise2 - 0.5f) * 2.f * 0.47f - 20.f / d2;
				uint8_t weight = uint8_t(pla::bounds(int(noise * 10000.f), 0, 255));

				if (z >= 0)
					block->writeValue(int3(x, y, z), Surface::value(0, weight), false);

				// Material 1 on top
				if (weight != 0) {
					inside = true;
				} else if (inside) {
					inside = false;
					if (z >= 0) {
						block->writeType(int3(x, y, z), 1, false);
						setType(int3(ax, ay, az - 1), 1);
					}
				}
			}
		}
	}

	block->markChanged();
}

void Terrain::markChangedBlock(const int3 &b) {
	if (auto it = mBlocks.find(b); it != mBlocks.end()) {
		it->second->markChanged();
	}
}

Terrain::Block::Block(Terrain *terrain, const int3 &b)
    : Surface::Block(b, std::bind(&Terrain::getBlock, terrain, _1)), mTerrain(terrain) {}

Terrain::Block::~Block(void) {}

bool Terrain::Block::update(const binary &data) {
	if (data.size() != Size * Size * Size * 2)
		return false;
	int c = 0;
	auto it = data.begin();
	for (int x = 0; x < Size; ++x)
		for (int y = 0; y < Size; ++y)
			for (int z = 0; z < Size; ++z) {
				auto &cell = mCells[c++];
				cell.type = *(it++);
				cell.weight = *(it++);
			}

	mChanged = true;
	return true;
}

bool Terrain::Block::hasChanged(void) const {
	bool tmp = false;
	std::swap(tmp, mChanged);
	return tmp;
}

Surface::value Terrain::Block::readValue(const int3 &c) const {
	if (c.x >= 0 && c.y >= 0 && c.z >= 0 && c.x < Size && c.y < Size && c.z < Size) {
		return mCells[(c.x * Size + c.y) * Size + c.z];
	} else {
		throw std::runtime_error("Read block value out of bounds");
	}
}

void Terrain::Block::markChanged(void) { mChanged = true; }

void Terrain::Block::writeValue(const int3 &c, Surface::value v, bool markChanged) {
	if (c.x >= 0 && c.y >= 0 && c.z >= 0 && c.x < Size && c.y < Size && c.z < Size) {
		mCells[(c.x * Size + c.y) * Size + c.z] = v;

		if (markChanged) {
			mChanged = true;

			// Mark neighboring blocks as changed
			int3 pos = position();
			for (int dx = -1; dx <= 1; ++dx) {
				if ((c.x != 0 && dx == -1) || (c.x != Size - 1 && dx == 1))
					continue;
				for (int dy = -1; dy <= 1; ++dy) {
					if ((c.y != 0 && dy == -1) || (c.y != Size - 1 && dy == 1))
						continue;
					for (int dz = -1; dz <= 1; ++dz) {
						if ((c.z != 0 && dz == -1) || (c.z != Size - 1 && dz == 1))
							continue;
						if (dx == 0 && dy == 0 && dz == 0)
							continue;
						mTerrain->markChangedBlock(int3(pos.x + dx, pos.y + dy, pos.z + dz));
					}
				}
			}
		}
	} else {
		throw std::runtime_error("Write block value out of bounds");
	}
}

void Terrain::Block::writeType(const int3 &c, uint8_t t, bool markChanged) {
	if (c.x >= 0 && c.y >= 0 && c.z >= 0 && c.x < Size && c.y < Size && c.z < Size) {
		mCells[(c.x * Size + c.y) * Size + c.z].type = t;
		if (markChanged)
			mChanged = true;
	} else {
		throw std::runtime_error("Write block value out of bounds");
	}
}

Terrain::TerrainIndex::TerrainIndex(const Index &index) : Index(index) {}

Terrain::TerrainIndex::TerrainIndex(int3 pos) {
	// Circumvent modulo implementation for negative values with an offset
	const unsigned offset = 0x80000000;
	pos += int3(offset, offset, offset);
	while (pos.x && pos.y && pos.z) {
		int n = 0;
		n += ((offset + pos.x) % 4);
		n += ((offset + pos.y) % 4) << 2;
		n += ((offset + pos.z) % 4) << 4;
		pos /= 4;
		mValues.push_back(n);
	}
}

Terrain::TerrainIndex::~TerrainIndex(void) {}

int3 Terrain::TerrainIndex::position(void) const {
	int3 pos;
	std::vector<int> values(mValues);
	while (values.size() > 0) {
		int n = values.back();
		pos *= 4;
		pos.x += (n + 2) % 4 - 2;
		pos.y += ((n >> 2) + 2) % 4 - 2;
		pos.z += ((n >> 4) + 2) % 4 - 2;
		values.pop_back();
	}
	return pos;
}

} // namespace convergence

