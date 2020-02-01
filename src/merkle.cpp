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

#include "src/merkle.hpp"
#include "src/include.hpp"

#include <iostream>

namespace convergence {

using namespace std::placeholders;

Merkle::Merkle(shared_ptr<Store> store) : mStore(std::move(store)) {}

Merkle::~Merkle(void) {}

void Merkle::update(double time) {
	std::lock_guard lock(mMutex);
	for (const auto &[index, data] : mChangedData)
			changeData(index, *data);
	mChangedData.clear();
}

void Merkle::updateRoot(const binary &digest) {
	std::lock_guard lock(mMutex);

	if (std::all_of(digest.begin(), digest.end(), [](byte b) { return b == byte(0); }))
		return;
	if (mRoot && mRoot->digest() == digest)
		return;
	if (mCandidates.find(digest) != mCandidates.end())
		return;

	std::cout << "Adding root candidate with digest " << to_hex(digest) << std::endl;
	auto candidate = createNode({}, digest);
	candidate->addResolvedCallback(std::bind(&Merkle::mergeRoot, this, _1));
	mCandidates[digest] = candidate;
}

void Merkle::updateData(Index index, const binary &data, bool change) {
	std::lock_guard lock(mMutex);

	auto digest = mStore->insert(data);
	std::cout << "Updating data with digest " << to_hex(digest) << std::endl;

	mRoot = mRoot ? mRoot->fork(index, digest, change, this)
	              : createNode({}, std::move(index), std::move(digest));
	propagateRoot(mRoot->digest());
}

shared_ptr<Merkle::Node> Merkle::get(Index index) const {
	return index.length() && mRoot ? mRoot->child(index) : mRoot;
}

shared_ptr<Merkle::Node> Merkle::root(void) const { return mRoot; }

binary Merkle::rootDigest() const { return mRoot ? mRoot->digest() : binary(16, byte(0)); }

void Merkle::mergeRoot(shared_ptr<Node> node) {
	std::cout << "Merging root " << to_hex(node->digest()) << std::endl;
	if (mRoot) {
		mRoot = mRoot->merge(node, this);
	} else {
		mRoot = node;
		mRoot->markChangedData(this);
	}
	propagateRoot(mRoot->digest());
}

shared_ptr<Merkle::Node> Merkle::createNode(Index index, binary digest) {
	auto node = std::make_shared<Node>(std::move(index), std::move(digest));
	node->populate(mStore);
	return node;
}

shared_ptr<Merkle::Node> Merkle::createNode(Index index, Index target, binary digest) {
	if (target.length() == 0) {
		return createNode(std::move(index), std::move(digest));
	}

	Node::ChildrenArray children;
	int n = target.pop();
	children[n] = createNode(Index(n, index), std::move(target), std::move(digest));

	auto node = std::make_shared<Node>(std::move(index), std::move(children), mStore);
	node->populate(mStore);
	return node;
}

Merkle::Index::Index(void) {}

Merkle::Index::Index(const binary &data) {
	mValues.resize(data.size());
	std::transform(data.begin(), data.end(), mValues.begin(),
	               [](byte b) { return std::to_integer<uint8_t>(b) & 0x3F; });
}

Merkle::Index::~Index(void) {}

Merkle::Index::Index(int child, const Index &parent) {
	mValues.reserve(1 + parent.mValues.size());
	mValues.push_back(child);
	for (int value : parent.mValues)
		mValues.push_back(value);
}

int Merkle::Index::length(void) const { return mValues.size(); }

int Merkle::Index::pop(void) {
	int tmp = mValues.back();
	mValues.pop_back();
	return tmp;
}

void Merkle::Index::push(int child) { mValues.push_back(child); }

Merkle::Index Merkle::Index::parent(void) const {
	return !mValues.empty() ? Index(++mValues.begin(), mValues.end()) : *this;
}

int Merkle::Index::child(void) const { return !mValues.empty() ? mValues.front() : 0; }

Merkle::Node::Node(Index index, binary digest)
    : mIndex(std::move(index)), mDigest(std::move(digest)) {}

Merkle::Node::Node(Index index, ChildrenArray children, shared_ptr<Store> store)
    : mIndex(std::move(index)) {

	// Build data
	binary data(ChildrenCount * 16, byte(0));
	auto it = data.begin();
	for (int i = 0; i < ChildrenCount; ++i) {
		if (children[i]) {
			const binary &digest = children[i]->digest();
			std::copy(digest.begin(), digest.begin() + 16, it);
		}
		it += 16;
	}

	mDigest = store->insert(data); // populate will retrieve the data
	mChildren.emplace(std::move(children));
}

Merkle::Node::~Node(void) {}

void Merkle::Node::populate(shared_ptr<Store> store) {
	if (std::any_of(mDigest.begin(), mDigest.end(), [](byte b) { return b != byte(0); }))
		store->request(mDigest, shared_from_this()); // will init resolved state
}

void Merkle::Node::notify(const binary &digest, shared_ptr<binary> data, shared_ptr<Store> store) {
	if (!data)
		return;

	mData = data;

	if (!mChildren && mIndex.length() < 16) {
		ChildrenArray children;
		auto it = data->begin();
		for (int i = 0; i < ChildrenCount; ++i) {
			if (std::any_of(it, it + 16, [](byte b) { return b != byte(0); })) {
				children[i] = std::make_shared<Node>(Index(i, mIndex), binary(it, it + 16));
				children[i]->populate(store);
			}
			it += 16;
		}
		mChildren.emplace(std::move(children));
	}

	initResolved();
}

shared_ptr<Merkle::Node> Merkle::Node::child(Index target) {
	if (target.length() == 0)
		return shared_from_this();

	int n = target.pop();
	auto child = mChildren ? mChildren->at(n) : nullptr;
	return child ? child->child(target) : nullptr;
}

shared_ptr<Merkle::Node> Merkle::Node::fork(Index target, const binary &digest, bool markChanged,
                                            Merkle *merkle) {
	if (target.length() == 0) {
		std::cout << "Forking " << to_hex(mDigest) << " to " << to_hex(digest) << std::endl;
		auto node = std::make_shared<Node>(mIndex, digest);
		node->populate(merkle->mStore);
		if (markChanged)
			node->markChangedData(merkle);
		return node;
	}

	ChildrenArray children;
	if (mChildren)
		children = *mChildren;

	int n = target.pop();
	auto &child = children[n];
	child = child ? child->fork(std::move(target), digest, markChanged, merkle)
	              : merkle->createNode(Index(n, mIndex), std::move(target), digest);

	auto node = std::make_shared<Node>(mIndex, std::move(children), merkle->mStore);
	node->populate(merkle->mStore);
	return node;
}

shared_ptr<Merkle::Node> Merkle::Node::merge(shared_ptr<Node> other, Merkle *merkle) {
	if (!other || !other->isResolved() || mIndex != other->mIndex)
		throw std::runtime_error("Illegal node merge");

	if (mDigest == other->mDigest)
		return shared_from_this();

	if (mIndex.length() < 16) {
		ChildrenArray children = *mChildren;
		ChildrenArray &others = *other->mChildren;
		for (int i = 0; i < ChildrenCount; ++i) {
			if (!others[i])
				continue;

			if (children[i]) {
				children[i] = children[i]->merge(others[i], merkle);
			} else {
				children[i] = others[i];
				children[i]->markChangedData(merkle);
			}
		}

		if (children == *mChildren)
			return shared_from_this();

		auto node = std::make_shared<Node>(mIndex, children, merkle->mStore);
		node->populate(merkle->mStore);
		return node;
	} else {
		binary data = *other->data();
		if (mData && !merkle->merge(*mData, data))
			return shared_from_this();

		auto digest = merkle->mStore->insert(data);
		auto node = std::make_shared<Node>(mIndex, digest);
		node->populate(merkle->mStore);
		node->markChangedData(merkle);
		return node;
	}
}

void Merkle::Node::markChangedData(Merkle *merkle) {
	if (mChildren) {
		std::for_each(mChildren->begin(), mChildren->end(), [&](shared_ptr<Node> child) {
			if (child)
				child->markChangedData(merkle);
		});
	} else if (mResolved) {
		std::cout << "Changed data for node " << to_hex(mDigest) << std::endl;
		merkle->mChangedData[mIndex] = mData;
	}
}

void Merkle::Node::addResolvedCallback(ResolvedCallback callback) {
	if (mResolved)
		callback(shared_from_this());
	else
		mResolvedCallbacks.emplace_back(std::move(callback));
}

void Merkle::Node::initResolved(void) {
	checkResolved();
	if (!mResolved && mChildren) {
		std::for_each(mChildren->begin(), mChildren->end(), [&](shared_ptr<Node> child) {
			if (child && !child->isResolved())
				child->addResolvedCallback([weak = weak_from_this()](shared_ptr<Node> node) {
					if (auto shared = weak.lock())
						shared->checkResolved();
				});
		});
	}
}

void Merkle::Node::checkResolved(void) {
	if (!mResolved && mData) {
		if (!mChildren ||
		    std::all_of(mChildren->begin(), mChildren->end(),
		                [](shared_ptr<Node> child) { return !child || child->isResolved(); })) {
			markResolved();
		}
	}
}

void Merkle::Node::markResolved(void) {
	mResolved = true;
	auto self = shared_from_this();
	for (auto callback : mResolvedCallbacks)
		callback(self);
	mResolvedCallbacks.clear();
}

bool Merkle::Node::isResolved(void) const { return mResolved; }

binary Merkle::Node::digest(void) const { return mDigest; }

binary Merkle::Node::toBinary(void) const {
	if (mData) {
		return *mData;
	} else {
		throw std::runtime_error("Cannot convert unresolved node to binary");
	}
}

std::optional<Merkle::Node::ChildrenArray> Merkle::Node::children(void) const { return mChildren; }

} // namespace convergence

