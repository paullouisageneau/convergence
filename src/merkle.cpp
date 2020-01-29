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

#include "pla/binaryformatter.hpp"

#include <iostream>

namespace convergence {

using pla::BinaryFormatter;

using namespace std::placeholders;

Merkle::Merkle(shared_ptr<Store> store) : mStore(store) {}

Merkle::~Merkle(void) {}

shared_ptr<Store> Merkle::store() const { return mStore; }

shared_ptr<Merkle::Node> Merkle::get(Index index) const {
	return index.length() && mRoot ? mRoot->child(index) : mRoot;
}

shared_ptr<Merkle::Node> Merkle::root(void) const { return mRoot; }

void Merkle::updateRoot(const binary &digest) {
	if (mRoot && mRoot->digest() == digest)
		return;
	if (mCandidates.find(digest) != mCandidates.end())
		return;
	std::cout << "Adding root candidate " << to_hex(digest) << std::endl;
	auto candidate = createNode(digest);
	candidate->addResolvedCallback(std::bind(&Merkle::mergeRoot, this, _1));
	mCandidates[digest] = candidate;
}

void Merkle::mergeRoot(shared_ptr<Node> node) {
	std::cout << "Merging root " << to_hex(node->digest()) << std::endl;
	mRoot = mRoot ? mRoot->merge({}, node, this) : node;
}

void Merkle::updateData(Index index, const binary &data) {
	auto digest = mStore->insert(data);
	std::cout << "Updating data with digest " << to_hex(digest) << std::endl;

	mRoot = mRoot ? mRoot->fork(index, digest, this) : createNode(index, digest);
	propagateRoot(mRoot->digest());
}

shared_ptr<Merkle::Node> Merkle::createNode(binary digest) {
	auto node = std::make_shared<Node>(digest);
	node->populate(mStore);
	return node;
}

shared_ptr<Merkle::Node> Merkle::createNode(Index index, binary digest) {
	if (index.length() == 0) {
		std::cout << "Creating leaf node " << to_hex(digest) << std::endl;
		return createNode(digest);
	}

	int level = index.length();

	Node::ChildrenArray children;
	int n = index.pop();
	children[n] = createNode(index, digest);

	auto node = std::make_shared<Node>(children, mStore);
	std::cout << "Creating node " << to_hex(node->digest()) << " at level " << level << std::endl;
	return node;
}

Merkle::Index::Index(void) {}

Merkle::Index::Index(const binary &data) {
	BinaryFormatter formatter(data);

	uint8_t length = 0;
	formatter >> length;

	while (length--) {
		uint8_t byte = 0;
		formatter >> byte;
		mValues.push_back(byte & 0x3F);
	}
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

Merkle::Node::Node(binary digest) : mDigest(std::move(digest)) {}

Merkle::Node::Node(ChildrenArray children, shared_ptr<Store> store) {
	mChildren.emplace(std::move(children));

	// Build data
	BinaryFormatter formatter;
	formatter << uint8_t(0); // TODO: constant
	std::for_each(mChildren->begin(), mChildren->end(), [&](shared_ptr<Node> child) {
		formatter << (child ? child->digest() : binary(16, byte(0)));
	});
	mDigest = store->insert(formatter.data());
	mData = store->retrieve(mDigest);
	mResolved = true;
}

Merkle::Node::~Node(void) {}

void Merkle::Node::populate(shared_ptr<Store> store) {
	store->request(mDigest, shared_from_this());
}

void Merkle::Node::notify(const binary &digest, shared_ptr<binary> data, shared_ptr<Store> store) {
	if (!data)
		return;

	mData = data;

	BinaryFormatter formatter(*mData);
	uint8_t type = 0;
	if (formatter >> type && type == 0) { // TODO: constant
		mChildren.emplace();

		auto it = mChildren->begin();
		binary childDigest(16);
		while (it != mChildren->end() && formatter >> childDigest) {
			if (std::any_of(childDigest.begin(), childDigest.end(),
			                [](byte b) { return b != byte(0); })) {
				auto node = std::make_shared<Node>(childDigest);
				node->populate(store);
				*it = node;
			}
			++it;
		}

		auto func = [&](shared_ptr<Node> child) {
			if (!child || child->isResolved())
				return true;
			child->addResolvedCallback([this](shared_ptr<Node> node) { checkResolved(); });
			return false;
		};
		if (std::all_of(mChildren->begin(), mChildren->end(), func))
			markResolved();
	} else {
		markResolved();
	}
}

shared_ptr<Merkle::Node> Merkle::Node::child(Index index) {
	if (index.length() == 0)
		return shared_from_this();

	int n = index.pop();
	auto child = mChildren ? mChildren->at(n) : nullptr;
	return child ? child->child(index) : nullptr;
}

shared_ptr<Merkle::Node> Merkle::Node::fork(Index index, const binary &digest, Merkle *merkle) {
	if (index.length() == 0) {
		std::cout << "Forking " << to_hex(mDigest) << " to " << to_hex(digest) << std::endl;
		return merkle->createNode(digest);
	}

	ChildrenArray children;
	if (mChildren)
		children = *mChildren;

	int n = index.pop();
	auto &child = children[n];
	child = child ? child->fork(index, digest, merkle) : merkle->createNode(index, digest);

	return std::make_shared<Node>(children, merkle->store());
}

shared_ptr<Merkle::Node> Merkle::Node::merge(Index index, shared_ptr<Node> other, Merkle *merkle) {
	if (mChildren) {
		bool changed = false;
		ChildrenArray children = *mChildren;
		if (other->mChildren) {
			auto &otherChildren = *other->mChildren;
			for (int i = 0; i < ChildrenCount; ++i) {
				if (children[i] != otherChildren[i]) {
					children[i] =
					    children[i] ? children[i]->merge(Index(i, index), otherChildren[i], merkle)
					                : otherChildren[i];
					changed = true;
				}
			}
		}
		return changed ? std::make_shared<Node>(children, merkle->store()) : shared_from_this();
	} else {
		binary data = *other->data();
		if (merkle->mergeData(index, data)) {
			auto digest = merkle->store()->insert(data);
			return merkle->createNode(digest);
		} else {
			return shared_from_this();
		}
	}
}

void Merkle::Node::addResolvedCallback(ResolvedCallback callback) {
	if (mResolved)
		callback(shared_from_this());
	else
		mResolvedCallbacks.emplace_back(std::move(callback));
}

void Merkle::Node::checkResolved(void) {
	if (mData && (!mChildren ||
	              std::all_of(mChildren->begin(), mChildren->end(), [](shared_ptr<Node> child) {
		              return !child || child->isResolved();
	              })))
		markResolved();
}

void Merkle::Node::markResolved(void) {
	mResolved = true;
	for (auto callback : mResolvedCallbacks)
		callback(shared_from_this());
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

