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

namespace convergence {

using pla::BinaryFormatter;
using pla::to_hex;

Merkle::Merkle(shared_ptr<Store> store) : mStore(store) {
}

Merkle::~Merkle(void) {}

shared_ptr<Merkle::Node> Merkle::get(const binary &digest) const {
	auto it = mNodes.find(digest);
	return it != mNodes.end() ? it->second : nullptr;
}

shared_ptr<Merkle::Node> Merkle::get(Index index) const {
	return index.length() && mRoot ? mRoot->child(index) : mRoot;
}

shared_ptr<Merkle::Node> Merkle::root(void) const { return mRoot; }

void Merkle::updateRoot(const binary &digest) { mRoot = createNode(nullptr, digest); }

void Merkle::updateData(const Index &index, const binary &data) {
	auto digest = mStore->insert(data);
	if (index.length()) {
		if (!mRoot)
			mRoot = createNode(nullptr);
		Index copy(index);
		mRoot->updateChild(copy, digest);
	} else {
		mRoot = createNode(nullptr, digest);
	}
	processRoot(mRoot->digest());
}

shared_ptr<Merkle::Node> Merkle::createNode(Node *parent) {
	return std::make_shared<Node>(this, parent);
}

shared_ptr<Merkle::Node> Merkle::createNode(Node *parent, const binary &digest) {
	if (auto it = mNodes.find(digest); it != mNodes.end())
		return it->second;

	auto node = std::make_shared<Node>(this, parent, digest);
	mNodes[digest] = node;
	mStore->request(digest, node);
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

void Merkle::Index::push(int child) { mValues.push_back(child); }

Merkle::Index Merkle::Index::parent(void) const {
	return !mValues.empty() ? Index(++mValues.begin(), mValues.end()) : *this;
}

int Merkle::Index::child(void) const { return !mValues.empty() ? mValues.front() : 0; }

Merkle::Node::Node(Merkle *merkle, Node *parent) : mMerkle(merkle), mParent(parent) {}
Merkle::Node::Node(Merkle *merkle, Node *parent, const binary &digest) : Node(merkle, parent) {
	mDigest = digest;
}

Merkle::Node::~Node(void) {}

void Merkle::Node::notify(const binary &digest, shared_ptr<binary> data) {
	if (data) {
		mData = data;

		Index index;
		computeIndex(index);
		if (index.length() < 16) {
			std::cout << "Processing block " << to_hex(digest) << " at level " << index.length()
			          << std::endl;
			mChildren.clear();
			BinaryFormatter formatter(*data);
			binary childDigest(16);
			while (formatter >> childDigest) {
				if (std::any_of(childDigest.begin(), childDigest.end(),
				                [](char c) { return c != 0; })) {
					auto child = mMerkle->createNode(this, childDigest);
					mChildren.push_back(child);
				}
			}
		} else {
			std::cout << "Processing data block " << to_hex(digest) << " at level "
			          << index.length() << std::endl;
			mMerkle->processData(index, *data);
		}
	}
}

void Merkle::Node::updateChild(Index &index, const binary &digest) {
	if (!index.length())
		return;

	int i = index.pop();
	if (i >= mChildren.size())
		mChildren.resize(i + 1, nullptr);

	auto &child = mChildren[i];
	if (index.length()) {
		if (!child)
			child = mMerkle->createNode(this);
		child->updateChild(index, digest);
	} else {
		child = mMerkle->createNode(this, digest);
	}

	// Rebuild data
	BinaryFormatter formatter;
	for (const auto &child : mChildren) {
		formatter << (child ? child->digest() : binary(16, char(0)));
	}

	mDigest = mMerkle->mStore->insert(formatter.data());
	mData = mMerkle->mStore->retrieve(mDigest);
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
	if (mData) {
		return *mData;
	} else {
		throw std::runtime_error("Cannot convert unresolved node to binary");
	}
}

void Merkle::Node::computeIndex(Index &index) {
	if (mParent) {
		int n = -1;
		for (int i = 0; i < mParent->mChildren.size(); ++i) {
			if (mParent->mChildren[i].get() == this) {
				n = i;
				break;
			}
		}
		if (n < 0) {
			std::cerr << "Children array mismatch!" << std::endl;
			n = 0;
		}
		index.push(n);
		mParent->computeIndex(index);
	}
}

} // namespace convergence

