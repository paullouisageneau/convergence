/***************************************************************************
 *   Copyright (C) 2017-2018 by Paul-Louis Ageneau                         *
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

#ifndef CONVERGENCE_MERKLE_H
#define CONVERGENCE_MERKLE_H

#include "src/include.hpp"
#include "src/store.hpp"

#include <unordered_map>

namespace convergence
{

class Merkle {
public:
	Merkle(shared_ptr<Store> store);
	~Merkle(void);

	class Index {
	public:
		Index(void);
		Index(const binary &data);
		template <typename Iterator> Index(Iterator begin, Iterator end) : mValues(begin, end) {}
		~Index(void);

		int length(void) const;
		int pop(void);
		void push(int child);

		Index parent(void) const;
		int child(void) const;

	protected:
		std::vector<int> mValues;
	};

	class Node : public Store::Notifiable {
	public:
		Node(Merkle *merkle, Node *parent);
		Node(Merkle *merkle, Node *parent, const binary &digest);
		~Node(void);

		void notify(const binary &digest, shared_ptr<binary> data);
		void updateChild(Index &index, const binary &digest);
		shared_ptr<Node> child(Index &index);

		void markResolved(void);
		bool isResolved(void);

		binary digest(void) const;
		binary toBinary(void) const;

	private:
		void computeIndex(Index &index);
		void checkResolved(void);

		Merkle *mMerkle;
		Node *mParent;
		binary mDigest;
		shared_ptr<binary> mData;
		std::vector<shared_ptr<Node>> mChildren;

		bool mResolved = false;
	};

	shared_ptr<Node> get(const binary &digest) const;
	shared_ptr<Node> get(Index index) const;
	shared_ptr<Node> root(void) const;

protected:
	void updateRoot(const binary &digest);
	virtual bool processRoot(const binary &digest) = 0;

	void updateData(const Index &index, const binary &data);
	virtual bool processData(const Index &index, const binary &data) = 0;
private:
	shared_ptr<Node> createNode(Node *parent);
	shared_ptr<Node> createNode(Node *parent, const binary &digest);

	const shared_ptr<Store> mStore;
	shared_ptr<Node> mRoot;
	std::unordered_map<binary, shared_ptr<Node>, binary_hash> mCandidates;
	std::unordered_map<binary, shared_ptr<Node>, binary_hash> mNodes;
};
}

#endif

