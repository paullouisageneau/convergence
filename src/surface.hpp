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

#ifndef CONVERGENCE_SURFACE_H
#define CONVERGENCE_SURFACE_H

#include "src/include.hpp"

#include "pla/mesh.hpp"
#include "pla/object.hpp"
#include "pla/collidable.hpp"
#include "pla/context.hpp"
#include "pla/program.hpp"
#include "pla/shader.hpp"
#include "pla/perlinnoise.hpp"

#include <vector>
#include <unordered_map>
#include <unordered_set>

using pla::Mesh;
using pla::Object;
using pla::Context;
using pla::Collidable;
using pla::Program;
using pla::VertexShader;
using pla::FragmentShader;
using pla::PerlinNoise;

namespace convergence
{

class Surface : public Collidable
{
public:
	struct int3
	{
		static int blockCoord(int v);
		static int cellCoord(int v);

		int3(int _x = 0, int _y = 0, int _z = 0);
		int3(const vec3 &v);
		int3(const int3 &block, const int3 &cell);

		int3 block(void) const;
		int3 cell(void) const;

		bool operator==(const int3 &i) const;
		bool operator!=(const int3 &i) const;
		int3 operator- (void) const;
		int3 operator+ (const int3 &i) const;
		int3 operator- (const int3 &i) const;
		int3 operator* (int i) const;
		int3 operator* (float f) const;

		int x, y, z;
	};

	struct int3_hash
	{
		std::size_t operator()(const int3 &p) const noexcept
		{
			std::size_t seed = 0;
			hash_combine(seed, p.x);
			hash_combine(seed, p.y);
			hash_combine(seed, p.z);
			return seed;
		}
	};
	
	#pragma pack(push, 4) // Align on 4 bytes
	struct int84
	{
		int84(int8_t _x = 0, int8_t _y = 0, int8_t _z = 0, int8_t _w = 0);
		int84(const vec4 &v);
		int84(const vec3 &v);

		int84 normalize(void) const;

		bool operator==(const int84 &i) const;
		bool operator!=(const int84 &i) const;
		int84 operator- (void) const;
		int84 operator+ (const int84 &i) const;
		int84 operator- (const int84 &i) const;
		int84 operator* (int i) const;
		int84 operator* (float f) const;

		int8_t x, y, z, w;
	};
	#pragma pack(pop)
	
	struct value
	{
		value(uint8_t _type = 0, uint8_t _weight = 0) : type(_type), weight(_weight) {}
		float w(void) const { return float(weight)/255.f; }

		uint8_t type;
		uint8_t weight;
	};

	Surface(unsigned int seed);
	~Surface(void);

	void update(double time);
	int draw(const Context &context);
	float intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection = NULL);

	void setValue(const int3 &p, value v);
	void setValue(const vec3 &p, value v);
	void setType(const int3 &p, uint8_t t);
	value getValue(const int3 &p);
	value getValue(const vec3 &p);
	int84 getGradient(const int3 &p);
	int84 getGradient(const vec3 &p);

	int addWeight(const int3 &p, int weight, int newType = -1);
	int addWeight(const vec3 &p, int weight, int newType = -1);
	void addWeight(const vec3 &p, int weight, float radius, int newType = -1);

protected:
	static const int Size = 8;
	static const int BlocksSize = 256 / Size;

	static uint16_t EdgeTable[256];
	static int8_t TriTable[256][16];
	static vec4 MaterialTable[4];

	class Block : public Mesh
	{
	public:
		Block(Surface *surface, const int3 &b);
		~Block(void);

		int3 position(void) const;
		vec3 center(void) const;
		bool isChanged(void) const;
		
		void setValue(const int3 &p, value v, bool markChanged = true);
		void setType(const int3 &p, uint8_t t);
		value getValue(const int3 &p);
		int84 getGradient(const int3 &p);
		int84 computeGradient(const int3 &p);

		int update(void);

	private:
		int polygonizeCell(const int3 &c,
			std::vector<vec3> &vertices, std::vector<int84> &normals, std::vector<int84> &material,
			std::vector<index_t> &indices);
		vec3 interpolate(vec3 p1, vec3 p2, int84 g1, int84 g2, value v1, value v2, int84 &grad, int84 &mat);

		Surface *mSurface;
		int3 mPos;

		value mCells[Size*Size*Size];
		bool  mChanged;

		friend class Surface;
	};

	void changedBlock(const int3 &b);
	sptr<Block> getBlock(const int3 &b);
	void getBlocksRec(const int3 &b, std::unordered_set<sptr<Block> > &result, std::unordered_set<sptr<Block> > &processed, std::function<bool(sptr<Block>)> check);
	void populateBlock(sptr<Block> block);
	
	std::unordered_map<int3, sptr<Block>, int3_hash> mBlocks;
	PerlinNoise mPerlin;
	sptr<Program> mProgram;
	bool mInit;
};

}

#endif
