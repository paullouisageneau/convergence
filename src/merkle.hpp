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
#include <unordered_set>

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
		Index(int child, const Index &parent);
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

	class Node : public Store::Notifiable, public std::enable_shared_from_this<Node> {
	public:
		static const int ChildrenCount = 64;
		using ChildrenArray = std::array<shared_ptr<Node>, ChildrenCount>;

		Node(binary digest);
		Node(ChildrenArray children, shared_ptr<Store> store);
		virtual ~Node(void);

		shared_ptr<binary> data() { return mData; }

		void populate(shared_ptr<Store> store);
		void notify(const binary &digest, shared_ptr<binary> data, shared_ptr<Store> store);
		shared_ptr<Node> child(Index index);
		shared_ptr<Node> fork(Index index, const binary &digest, Merkle *merkle);
		shared_ptr<Node> merge(Index index, shared_ptr<Node> other, Merkle *merkle);

		using ResolvedCallback = std::function<void(shared_ptr<Node>)>;
		void addResolvedCallback(ResolvedCallback callback);

		void checkResolved(void);
		void markResolved(void);
		bool isResolved(void) const;

		binary digest(void) const;
		binary toBinary(void) const;

		std::optional<ChildrenArray> children(void) const;

	private:
		binary mDigest;
		shared_ptr<binary> mData;
		std::optional<ChildrenArray> mChildren;
		std::list<ResolvedCallback> mResolvedCallbacks;

		bool mResolved = false;
	};

	shared_ptr<Store> store() const;

	shared_ptr<Node> get(Index index) const;
	shared_ptr<Node> root() const;

protected:
	void updateRoot(const binary &digest);
	void mergeRoot(shared_ptr<Node> node);

	void updateData(Index index, const binary &data);
	virtual bool mergeData(const Index &index, binary &data) = 0;

	virtual bool propagateRoot(const binary &digest) = 0;
	virtual bool propagateData(const Index &index, const binary &data) = 0;

private:
	shared_ptr<Node> createNode(binary digest);
	shared_ptr<Node> createNode(Index index, binary digest);

	const shared_ptr<Store> mStore;
	shared_ptr<Node> mRoot;
	std::unordered_map<binary, shared_ptr<Node>, binary_hash> mCandidates;
};
}

#endif

