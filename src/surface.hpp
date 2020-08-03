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
#include "src/types.hpp"
#include "src/volume.hpp"

#include "pla/collidable.hpp"
#include "pla/context.hpp"
#include "pla/object.hpp"
#include "pla/program.hpp"
#include "pla/shader.hpp"

#include <unordered_map>
#include <unordered_set>
#include <vector>

using pla::Collidable;
using pla::Context;
using pla::FragmentShader;
using pla::Object;
using pla::Program;
using pla::VertexShader;

namespace convergence {

class Surface : public Collidable {
public:
#pragma pack(push, 1) // pack tightly
	struct value {
		value(uint8_t _type = 0, uint8_t _weight = 0) : type(_type), weight(_weight) {}
		float w(void) const { return float(weight) / 255.f; }

		bool operator==(const value &v) const { return type == v.type && weight == v.weight; }
		bool operator!=(const value &v) const { return type != v.type || weight != v.weight; }

		uint8_t type;
		uint8_t weight;
	};
#pragma pack(pop)

	using Material = Volume::Material;

	class Block : public Volume {
	public:
		static const int Size = 8;
		static const int CellsCount = Size * Size * Size;

		static int blockCoord(int v);
		static int3 blockCoord(const int3 &p);
		static int cellCoord(int v);
		static int3 cellCoord(const int3 &p);
		static int3 fullCoord(const int3 &b, const int3 &c);

		Block(const int3 &b, std::function<shared_ptr<Block>(const int3 &b)> retrieveFunc);
		~Block(void);

		virtual bool hasChanged(void) const = 0;
		virtual value readValue(const int3 &c) const = 0;

		int3 position(void) const;
		vec3 center(void) const;

		value getValue(const int3 &c);
		int84 getGradient(const int3 &c);
		int84 computeGradient(const int3 &c);

		int prepare(void);

	private:
		void computeGradients(const uint8_t *weights, int84 *grads) override;

		std::function<shared_ptr<Block>(const int3 &b)> mRetrieveFunc;
		int3 mPos;
	};

	Surface(std::function<shared_ptr<Block>(const int3 &b)> retrieveFunc);
	~Surface(void);

	void update(double time);
	int draw(const Context &context);
	float intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection = NULL);

protected:
	void getBlocksRec(const int3 &b, std::unordered_set<sptr<Block>> &result,
	                  std::unordered_set<sptr<Block>> &processed,
	                  std::function<bool(sptr<Block>)> check) const;

	shared_ptr<Program> mProgram;
	shared_ptr<Program> mInkProgram;
	std::function<shared_ptr<Block>(const int3 &b)> mRetrieveFunc;

private:
	static Material MaterialTable[4];
};

} // namespace convergence

#endif
