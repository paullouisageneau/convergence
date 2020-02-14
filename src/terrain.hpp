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

#include "include.hpp"
#include "merkle.hpp"
#include "messagebus.hpp"
#include "store.hpp"
#include "surface.hpp"

#include "pla/context.hpp"
#include "pla/perlinnoise.hpp"

namespace convergence {

using pla::PerlinNoise;

class Terrain : public Merkle, public MessageBus::Listener, public Collidable {
public:
	Terrain(shared_ptr<MessageBus> messageBus, shared_ptr<Store> store, int seed);
	virtual ~Terrain(void);

	void update(double time);
	int draw(const Context &context);
	float intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection = NULL);

	void dig(const vec3 &p, int weight, float radius);

	void broadcast();

protected:
	class TerrainIndex : public Index {
	public:
		TerrainIndex(const Index &index);
		TerrainIndex(int3 pos);
		~TerrainIndex(void);

		int3 position(void) const;
	};

	void onMessage(const Message &message);

	bool replaceData(const int3 &pos, const binary &data);
	bool mergeData(const int3 &pos, binary &data);
	void commitData(const int3 &pos, const binary &data);

	bool merge(const binary &a, binary &b);
	bool changeData(const Index &index, const binary &data);

	bool propagateRoot(const binary &digest);
	bool propagateData(const int3 &pos, const binary &data);

private:
	class Block : public Surface::Block {
	public:
		static bool Merge(const Surface::value *a, Surface::value *b);

		Block(Terrain *terrain, const int3 &b);
		~Block(void);

		bool replace(const binary &data);
		bool merge(binary &data);
		void commit(void);

		bool hasChanged(void) const;
		void markChanged(void);

		Surface::value readValue(const int3 &c) const;
		bool writeValue(const int3 &c, Surface::value v, bool markChanged = true);
		bool writeType(const int3 &c, uint8_t t, bool markChanged = true);

	private:
		Surface::value readValueImpl(const int3 &c) const;
		bool writeValueImpl(const int3 &c, Surface::value v, bool markChanged);
		bool writeTypeImpl(const int3 &c, uint8_t t, bool markChanged);

		Terrain *mTerrain;
		Surface::value mCells[CellsCount] = {};

		mutable bool mChanged = true;
	};

	shared_ptr<Block> getBlock(const int3 &b);
	Surface::value getValue(const int3 &p);
	bool setValue(const int3 &p, Surface::value v);
	bool setType(const int3 &p, uint8_t t);

	void populateBlock(shared_ptr<Block> block);
	void markChangedBlock(const int3 &b);

	std::unordered_map<int3, shared_ptr<Block>, int3::hash> mBlocks;

	shared_ptr<MessageBus> mMessageBus;
	PerlinNoise mNoise;
	Surface mSurface;
};
} // namespace convergence

#endif
