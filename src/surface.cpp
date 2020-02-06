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

#include "src/surface.hpp"

namespace convergence {

Surface::Surface(std::function<shared_ptr<Block>(const int3 &b)> retrieveFunc)
    : mRetrieveFunc(retrieveFunc) {
	mProgram = std::make_shared<Program>(std::make_shared<VertexShader>("shader/ground.vect"),
	                                     std::make_shared<FragmentShader>("shader/ground.frag"));

	mProgram->bindAttribLocation(0, "position");
	mProgram->bindAttribLocation(1, "normal");
	mProgram->bindAttribLocation(2, "ambient");
	mProgram->bindAttribLocation(3, "diffuse");
	mProgram->bindAttribLocation(4, "smoothness");
	mProgram->link();
}

Surface::~Surface(void) {}

void Surface::update(double time) {}

int Surface::draw(const Context &context) {
	const vec3 pos = context.cameraPosition();
	const int3 b = Block::blockCoord(int3(pos));
	const float d = (float(Block::Size) + 1.f) * 0.5f * pla::Sqrt2;

	std::unordered_set<sptr<Block>> blocks, processed;
	getBlocksRec(b, blocks, processed, [context, b, d](sptr<Block> blk) {
		if (blk->position() == b)
			return true;
		const vec3 p0 = blk->center();
		return context.frustum().testSphere(p0, d);
	});

	context.prepare(mProgram);

	mProgram->bind();

	int count = 0;
	for (auto blk : blocks) {
		blk->prepare();
		count += blk->drawElements();
	}

	mProgram->unbind();
	return count;
}

float Surface::intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection) {
	const vec3 p1 = pos;
	const vec3 p2 = pos + move;
	const vec3 n = glm::normalize(move);
	const float d = (float(Block::Size) + 1.f) * 0.5f * pla::Sqrt2;
	const float r = d + radius;
	const float r2 = r * r;

	int3 b = Block::blockCoord(int3(pos));
	std::unordered_set<sptr<Block>> blocks, processed;
	getBlocksRec(b, blocks, processed, [p1, p2, n, r2](sptr<Block> blk) {
		vec3 p0 = blk->center();
		vec3 p0p1 = p1 - p0;
		vec3 p2p0 = p0 - p2;
		float c = glm::dot(n, p0p1);
		if (c > 0.f)
			return glm::length2(p0p1) <= r2;
		if (glm::dot(n, p2p0) > 0.f)
			return glm::length2(p2p0) <= r2;
		return glm::length2(p0p1 - n * c) <= r2;
	});

	float nearest = std::numeric_limits<float>::infinity();
	vec3 nearestIntersection;
	for (auto blk : blocks) {
		blk->prepare();

		float t = blk->intersect(pos, move, radius, intersection);
		if (t < nearest) {
			nearest = t;
			if (intersection)
				nearestIntersection = *intersection;
		}
	}

	if (intersection)
		*intersection = nearestIntersection;
	return nearest;
}

void Surface::getBlocksRec(const int3 &b, std::unordered_set<sptr<Block>> &result,
                           std::unordered_set<sptr<Block>> &processed,
                           std::function<bool(sptr<Block>)> check) const {
	sptr<Block> block = mRetrieveFunc(b);
	if (processed.find(block) != processed.end())
		return;

	processed.insert(block);
	if (!check(block))
		return;

	result.insert(block);
	for (int dx = -1; dx <= 1; ++dx)
		for (int dy = -1; dy <= 1; ++dy)
			for (int dz = -1; dz <= 1; ++dz)
				if (dx != 0 || dy != 0 || dz != 0)
					getBlocksRec(int3(b.x + dx, b.y + dy, b.z + dz), result, processed, check);
}

int Surface::Block::blockCoord(int v) {
	// Circumvent modulo implementation for negative values with an offset
	const unsigned offset = 0x80000000;
	const unsigned offsetBySize = offset / Block::Size;
	return int(unsigned(offset + v) / Block::Size) - offsetBySize;
}

int3 Surface::Block::blockCoord(const int3 &p) {
	return int3(blockCoord(p.x), blockCoord(p.y), blockCoord(p.z));
}

int Surface::Block::cellCoord(int v) {
	// Circumvent modulo implementation for negative values with an offset
	const unsigned offset = 0x80000000;
	return int(unsigned(offset + v) % Block::Size);
}

int3 Surface::Block::cellCoord(const int3 &p) {
	return int3(cellCoord(p.x), cellCoord(p.y), cellCoord(p.z));
}

int3 Surface::Block::fullCoord(const int3 &b, const int3 &c) { return b * Block::Size + c; }

Surface::Block::Block(const int3 &b, std::function<shared_ptr<Block>(const int3 &b)> retrieveFunc)
    : Volume({Size + 1, Size + 1, Size + 1}, 1.f), mPos(b), mRetrieveFunc(retrieveFunc) {}

Surface::Block::~Block(void) {}

int3 Surface::Block::position(void) const { return mPos; }

vec3 Surface::Block::center(void) const {
	return vec3((float(mPos.x) + 0.5f) * Size, (float(mPos.y) + 0.5f) * Size,
	            (float(mPos.z) + 0.5f) * Size);
}

Surface::value Surface::Block::getValue(const int3 &c) {
	if (c.x >= 0 && c.y >= 0 && c.z >= 0 && c.x < Size && c.y < Size && c.z < Size) {
		return readValue(c);
	} else {
		int3 p = fullCoord(mPos, c);
		sptr<Block> block = mRetrieveFunc(blockCoord(p));
		return block->readValue(cellCoord(p));
	}
}

int Surface::Block::prepare(void) {
	if (!hasChanged())
		return indicesCount() / 3;

	const size_t count = (Size + 1) * (Size + 1) * (Size + 1);

	uint8_t weights[count];
	Material mats[count];
	int i = 0;
	for (int x = 0; x < Size + 1; ++x)
		for (int y = 0; y < Size + 1; ++y)
			for (int z = 0; z < Size + 1; ++z) {
				value v = getValue(int3(x, y, z));
				weights[i] = v.weight;
				mats[i] = MaterialTable[v.type];
				++i;
			}
	return polygonize(weights, mats, center());
}

void Surface::Block::computeGradients(const uint8_t *weights, int84 *grads) {
	for (int x = 0; x < Size + 1; ++x)
		for (int y = 0; y < Size + 1; ++y)
			for (int z = 0; z < Size + 1; ++z)
				grads[getIndex({x, y, z})] = int84(
				    (getValue(int3(x + 1, y, z)).weight - getValue(int3(x - 1, y, z)).weight) / 2,
				    (getValue(int3(x, y + 1, z)).weight - getValue(int3(x, y - 1, z)).weight) / 2,
				    (getValue(int3(x, y, z + 1)).weight - getValue(int3(x, y, z - 1)).weight) / 2,
				    0);
}

Surface::Material Surface::Material::operator+(const Material &m) const {
	return {ambient + m.ambient, diffuse + m.diffuse, uint8_t(smoothness + m.smoothness)};
}

Surface::Material Surface::Material::operator*(float f) const {
	return {ambient * f, diffuse * f, uint8_t(std::floor(float(smoothness) * f))};
}

Surface::Material Surface::MaterialTable[4] = {
	{{5, 5, 5, 255}, {50, 50, 50, 255}, 0}, // ambient, diffuse, smoothness
	{{5, 15, 5, 255}, {50, 128, 50, 255}, 200},
	{{15, 15, 5, 255}, {150, 150, 25, 255}, 230},
	{{128, 128, 0, 255}, {200, 200, 0, 255}, 128}};

} // namespace convergence
