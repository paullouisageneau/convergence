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

#ifndef CONVERGENCE_TERRAIN_H
#define CONVERGENCE_TERRAIN_H

#include "src/include.hpp"
#include "src/merkle.hpp"
#include "src/store.hpp"
#include "src/surface.hpp"

#include "pla/context.hpp"

namespace convergence
{

class Terrain : public Merkle, public Collidable {
public:
	Terrain(shared_ptr<Store> store, int seed);
	~Terrain(void);

	Surface::value addWeight(const int3 &p, int weight, int newType = -1);

	void update(double time);
	int draw(const Context &context);
	float intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection = NULL);

	void build(const vec3 &p, int weight);
	void dig(const vec3 &p, int weight, float radius);

protected:
	class TerrainIndex : public Index {
	public:
		TerrainIndex(const Index &index);
		TerrainIndex(int3 pos);
		~TerrainIndex(void);

		int3 position(void) const;
	};

	bool processData(const Index &index, const binary &data);

private:
	class Block : public Surface::Block {
	public:
        Block(Terrain *terrain, const int3 &b);
        ~Block(void);

		bool update(const binary &data);

		bool hasChanged(void) const;
		Surface::value readValue(const int3 &c) const;

		void markChanged(void);
        void writeValue(const int3 &c, Surface::value v, bool markChanged = true);
        void writeType(const int3 &c, uint8_t t, bool markChanged = true);

    private:
		Terrain *mTerrain;
		Surface::value mCells[Size*Size*Size];

		mutable bool mChanged;
	};

	sptr<Block> getBlock(const int3 &b);
	Surface::value getValue(const int3 &p);
    void setValue(const int3 &p, Surface::value v);
    void setType(const int3 &p, uint8_t t);

	void populateBlock(sptr<Block> block);
	void markChangedBlock(const int3 &b);

	std::unordered_map<int3, sptr<Block>, int3_hash> mBlocks;

	PerlinNoise mPerlin;
	Surface mSurface;
};
}

#endif
