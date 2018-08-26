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

#include "p3d/include.hpp"
#include "p3d/mesh.hpp"
#include "p3d/object.hpp"
#include "p3d/collidable.hpp"
#include "p3d/context.hpp"
#include "p3d/program.hpp"
#include "p3d/shader.hpp"
#include "p3d/perlinnoise.hpp"

using pla::vec3;
using pla::vec4;
using pla::Mesh;
using pla::Object;
using pla::Context;
using pla::Collidable;
using pla::Program;
using pla::VertexShader;
using pla::FragmentShader;
using pla::PerlinNoise;
template<typename T> using sptr = std::shared_ptr<T>;

namespace convergence
{

class Surface : public Collidable
{
public:
	#pragma pack(push, 4) // Align on 4 bytes
	struct int4
	{
		static int8_t blockCoord(int8_t v);
		static int8_t cellCoord(int8_t v);

		int4(int8_t _x = 0, int8_t _y = 0, int8_t _z = 0, int8_t _w = 0);
		int4(const vec4 &v);
		int4(const vec3 &v);

		int4 block(void) const;
		int4 cell(void) const;
		int4 normalize(void) const;

		bool operator==(const int4 &i) const;
		bool operator!=(const int4 &i) const;
		bool operator< (const int4 &i) const;
		bool operator> (const int4 &i) const;
		int4 operator- (void) const;
		int4 operator+ (const int4 &i) const;
		int4 operator- (const int4 &i) const;
		int4 operator* (const float &f) const;

		int8_t x, y, z, w;
	};
	#pragma pack(pop)

	struct value
	{
		value(uint8_t t = 0, int8_t w = -128) : type(t), weight(w) {}
		float w(void) const { return float(weight)/128; }

		uint8_t type;
		int8_t weight;
	};

	Surface(unsigned int seed);
	~Surface(void);

	void update(void);
	int draw(const Context &context);
	float intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection = NULL);

	void setValue(const int4 &p, value v);
	void setValue(const vec3 &p, value v);
	void setType(const int4 &p, uint8_t t);
	value getValue(const int4 &p);
	value getValue(const vec3 &p);

	void addWeight(const vec3 &p, float radius, int weight, int newType = -1);

protected:
	static const int8_t Size = 8;
	static uint16_t EdgeTable[256];
	static int8_t   TriTable[256][16];
	static vec4	MaterialTable[4];

	class Block : public Mesh
	{
	public:
		Block(Surface *surface, const int4 &b);
		~Block(void);

		vec3 center(void) const;

		void setValue(const int4 &p, value v);
		void setType(const int4 &p, uint8_t t);
		value getValue(const int4 &p);
		int4 getGradient(const int4 &p);
		int4 computeGradient(const int4 &p);

		int update(void);

	private:
		int polygonizeCell(const int4 &c,
			std::vector<vec3> &vertices, std::vector<int4> &normals, std::vector<int4> &material,
			std::vector<index_t> &indices);
		vec3 interpolate(vec3 p1, vec3 p2, value v1, value v2);
		int4 interpolate(int4 p1, int4 p2, value v1, value v2);
		int4 material(value v1, value v2);

		Surface *mSurface;
		int4 mPos;

		value mCells[Size*Size*Size];
		int4  mGrads[Size*Size*Size];
		bool  mChanged;

		friend class Surface;
	};

	void changedBlock(const int4 &b);
	sptr<Block> getBlock(const int4 &b);
	void getBlocksRec(const int4 &b, std::set<sptr<Block> > &result, std::set<sptr<Block> > &processed, std::function<bool(sptr<Block>)> check);

	std::map<int4, sptr<Block> > mBlocks;
	sptr<Program> mProgram;
};

}

#endif
