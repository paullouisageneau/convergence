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
#include "src/sha3.hpp"

#include "pla/binaryformatter.hpp"

namespace convergence {

using pla::BinaryFormatter;
using pla::to_hex;

Merkle::Merkle(shared_ptr<Store> store) : mStore(store) {}

Merkle::~Merkle(void) {}

void Merkle::init(void) {}

void Merkle::update(void) {}

shared_ptr<Merkle::Node> Merkle::get(const binary &digest) const {
	auto it = mNodes.find(digest);
	return it != mNodes.end() ? it->second : nullptr;
}

shared_ptr<Merkle::Node> Merkle::get(Index index) const {
	return index.length() ? mRoot->child(index) : mRoot;
}

shared_ptr<Merkle::Node> Merkle::root(void) const { return mRoot; }

void Merkle::setRoot(const binary &digest) { mRoot = createNode(digest); }

void Merkle::updateData(const Index &index, const binary &data) {
	auto digest = mStore->insert(data);
	auto node = std::make_shared<Node>(this, digest);
	Index copy(index);
	mRoot->updateChild(copy, node);
}

shared_ptr<Merkle::Node> Merkle::createNode(const binary &digest) {
	auto it = mNodes.find(digest);
	if (it != mNodes.end())
		return it->second;

	auto node = std::make_shared<Node>(this, digest);
	mStore->request(digest, node);
	mNodes[digest] = node;
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

int Merkle::Index::length(void) const { return mValues.size(); }

int Merkle::Index::pop(void) {
	int tmp = mValues.back();
	mValues.pop_back();
	return tmp;
}

Merkle::Index Merkle::Index::parent(void) const {
	return !mValues.empty() ? Index(++mValues.begin(), mValues.end()) : *this;
}

int Merkle::Index::child(void) const { return !mValues.empty() ? mValues.front() : 0; }

Merkle::Node::Node(Merkle *merkle) : mMerkle(merkle) {}

Merkle::Node::Node(Merkle *merkle, const binary &digest) : mMerkle(merkle), mDigest(digest) {}

Merkle::Node::~Node(void) {}

void Merkle::Node::notify(const binary &digest, shared_ptr<binary> data) { mData = data; }

void Merkle::Node::updateChild(Index &index, shared_ptr<Node> node) {
	if (!index.length())
		return;

	int i = index.pop();
	if (index.length()) {
		if (i < mChildren.size())
			mChildren.resize(i + 1, nullptr);
		if (index.length()) {
			if (!mChildren[i])
				mChildren[i] = shared_ptr<Node>(this);
		}
		mChildren[i]->updateChild(index, node);
	} else {
		mChildren[i] = node;
	}
}

shared_ptr<Merkle::Node> Merkle::Node::child(Index &index) {
	if (index.length()) {
		int i = index.pop();
		if (i < mChildren.size() && mChildren[i]) {
			return index.length() ? mChildren[i]->child(index) : mChildren[i];
		}
	}
	return nullptr;
}

void Merkle::Node::markChanged(void) { mDigest.clear(); }

binary Merkle::Node::digest(void) const { return mDigest; }

binary Merkle::Node::toBinary(void) const {
	if (auto data_ptr = mMerkle->mStore->retrieve(mDigest)) {
		return *data_ptr;
	} else {
		throw std::runtime_error("Cannot convert unresolved node to binary");
	}
}

} // namespace convergence

