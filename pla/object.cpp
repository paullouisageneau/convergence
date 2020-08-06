/***************************************************************************
 *   Copyright (C) 2006-2016 by Paul-Louis Ageneau                         *
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

#include "pla/object.hpp"

#include <unordered_map>

namespace pla {

Object::Object(sptr<Mesh> mesh, sptr<Program> program) : mMesh(mesh) {
	if (program)
		setProgram(program, 0);
}

Object::Object(const index_t *indices, size_t nindices, const float *vertices, size_t nvertices,
               sptr<Program> program)
    : mMesh(std::make_shared<Mesh>(indices, nindices, vertices, nvertices)) {
	if (program)
		setProgram(program, 0);
}

Object::~Object(void) {}

void Object::setMesh(sptr<Mesh> mesh) { mMesh = mesh; }

void Object::setProgram(sptr<Program> program, size_t firstIndex) {
	if (program)
		mPrograms.insert(std::make_pair(firstIndex, program));
	else
		mPrograms.erase(firstIndex);
}

void Object::unsetProgram(size_t firstIndex) { mPrograms.erase(firstIndex); }

int Object::draw(const Context &context) const {
	int count = 0;
	if (mMesh) {
		auto it = mPrograms.begin();
		while (it != mPrograms.end()) {
			size_t first = it->first;
			size_t last = mMesh->indicesCount();
			auto program = it->second;

			if (++it != mPrograms.end())
				last = it->first;

			if (program) {
				context.render(program, [&] { count += mMesh->drawElements(first, last - first); });
			}
		}
	}
	return count;
}

Sphere::Sphere(int resolution, sptr<Program> program) : Object(Build(resolution), program) {}

sptr<Mesh> Sphere::Build(int resolution) {
	static std::unordered_map<int, sptr<Mesh>> cache;

	if (auto it = cache.find(resolution); it != cache.end())
		return it->second;

	std::vector<index_t> indices;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texcoords;

	const float radius = 0.5f;
	const int N = 64;
	const int M = N / 2 + 1;
	index_t k = 0;

	indices.reserve(N * M * 6);
	vertices.reserve(N * M * 3);
	normals.reserve(N * M * 3);
	texcoords.reserve(N * M * 2);

	for (int i = 0; i < N; ++i) {
		float t1 = float(i) / float(N);
		float a = 2.f * Pi * t1;
		for (int j = 0; j < M; ++j) {
			float t2 = float(j) / float(M - 1);
			float b = Pi * t2;
			vec3 n = vec3(std::cos(a) * std::sin(b), std::sin(a) * std::sin(b), std::cos(b));
			vec3 v = n * radius;
			vec2 t = vec2(t1, t2);
			vertices.push_back(v.x);
			vertices.push_back(v.y);
			vertices.push_back(v.z);
			normals.push_back(n.x);
			normals.push_back(n.y);
			normals.push_back(n.z);
			texcoords.push_back(t.x);
			texcoords.push_back(t.y);
			++k;
			auto l = (k + M) % (N * M);
			indices.push_back(k - 1);
			indices.push_back(k);
			indices.push_back(l);
			indices.push_back(l);
			indices.push_back(l - 1);
			indices.push_back(k - 1);
		}
	}

	auto mesh = std::make_shared<Mesh>();
	mesh->setIndices(indices.data(), indices.size());
	mesh->setVertexAttrib(0, vertices.data(), vertices.size(), 3);
	mesh->setVertexAttrib(1, normals.data(), normals.size(), 3);
	mesh->setVertexAttrib(2, texcoords.data(), texcoords.size(), 2);

	cache.emplace(resolution, mesh);
	return mesh;
}

Quad::Quad(sptr<Program> program) : Object(Build(), program) {}

sptr<Mesh> Quad::Build() {
	static const float vertices[] = {-1.f, -1.f, 0.f, 1.f,  -1.f, 0.f,
	                                 1.f,  1.f,  0.f, -1.f, 1.f,  0.f};
	static const index_t indices[] = {0, 1, 2, 2, 3, 0};
	static const sptr<Mesh> mesh = std::make_shared<Mesh>(indices, 2 * 3, vertices, 2 * 3);
	return mesh;
}

} // namespace pla
