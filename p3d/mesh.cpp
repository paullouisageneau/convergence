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

#include "p3d/mesh.hpp"
#include "p3d/intersection.hpp"

namespace pla
{

Mesh::Mesh(void)
{
	mIndexBuffer = std::make_shared<IndexBuffer>(new IndexBufferObject(true));	// readable
}

Mesh::Mesh(	const index_t *indices,
		size_t nindices,
		const float *vertices,
		size_t nvertices) :
	Mesh()
{
	setIndices(indices, nindices);
	setVertexAttrib(0, vertices, nvertices, 3);
	computeRadius();
}

Mesh::~Mesh(void)
{
	
}

void Mesh::setIndices(const index_t *indices, size_t count)
{
	if(indices)
	{
		mIndexBuffer->fill(indices, count);
	}
	else {
		mIndexBuffer->fill(NULL, 0);
	}
}

void Mesh::setVertexAttrib(unsigned layout, const float *attribs, size_t count, int size, bool normalize)
{
	sptr<Attrib> attrib = getAttribBuffer<float>(layout);
	attrib->size = size;
	attrib->type = GL_FLOAT;
	attrib->normalize = (normalize ? GL_TRUE : GL_FALSE);
	attrib->fill(attribs, count);
	
	if(layout == 0) computeRadius();
}

void Mesh::setVertexAttrib(unsigned layout, const int *attribs, size_t count, int size, bool normalize)
{
	sptr<Attrib> attrib = getAttribBuffer<int>(layout);
	attrib->size = size;
	attrib->type = GL_INT;
	attrib->normalize = (normalize ? GL_TRUE : GL_FALSE);
	attrib->fill(attribs, count);
}

void Mesh::setVertexAttrib(unsigned layout, const char *attribs, size_t count, int size, bool normalize)
{
	sptr<Attrib> attrib = getAttribBuffer<char>(layout);
	attrib->size = size;
	attrib->type = GL_BYTE;
	attrib->normalize = (normalize ? GL_TRUE : GL_FALSE);
	attrib->fill(attribs, count);
}

void Mesh::unsetVertexAttrib(unsigned layout)
{
	mAttribBuffers.erase(layout);
}

size_t Mesh::indicesCount(void) const
{
	return mIndexBuffer->count();
}

size_t Mesh::vertexAttribCount(unsigned layout) const
{
	auto it = mAttribBuffers.find(layout);
	if(it != mAttribBuffers.end()) return it->second->count();
	else return 0;
}

int Mesh::vertexAttribSize(unsigned layout) const
{
	auto it = mAttribBuffers.find(layout);
	if(it != mAttribBuffers.end()) return it->second->size;
	else return 0;
}

void Mesh::optimize(unsigned layout)
{
	auto it = mAttribBuffers.find(layout);
	if(it == mAttribBuffers.end()) 
		return;
	
	sptr<Attrib> vertexBuffer = it->second;
	Assert(vertexBuffer->size == 3);
	Assert(vertexBuffer->type == GL_FLOAT);
	
	const float *vertices = reinterpret_cast<float*>(vertexBuffer->data());
	index_t *indices = mIndexBuffer->data();
	
	index_t newverticesindex = 0;
	for(index_t i=0; i<mIndexBuffer->count(); ++i)
	{
		index_t ii = indices[i];
		vec3 vi = glm::make_vec3(vertices + ii*3);
		
		for(index_t j=i+1; j<mIndexBuffer->count(); ++j)
		{
			index_t ij = indices[j];
			if(ij == ii) continue;
			vec3 vj = glm::make_vec3(vertices + ij*3);
			
			if(glm::l1Norm(vi-vj) <= std::numeric_limits<float>::epsilon())
			{
				indices[j] = ii;
			}
		}
	}

	mIndexBuffer->fill(indices, mIndexBuffer->count());
}

void Mesh::computeNormals(unsigned normalLayout, unsigned layout)
{
	auto it = mAttribBuffers.find(layout);
	if(it == mAttribBuffers.end()) 
	{
		unsetVertexAttrib(normalLayout);
		return;
	}
	
	sptr<Attrib> vertexBuffer = it->second;
	Assert(vertexBuffer->size == 3);
	Assert(vertexBuffer->type == GL_FLOAT);
	
	const float *vertices = reinterpret_cast<float*>(vertexBuffer->data());
	const index_t *indices = mIndexBuffer->data();
	
	float *normals = new float[vertexBuffer->count()];
	std::fill(normals, normals+vertexBuffer->count(), 0.f);
	
	for(index_t i=0; i<mIndexBuffer->count(); i+= 3)
	{
		// The 3 vertices of current face
		vec3 v1 = glm::make_vec3(vertices + indices[i]*3);
		vec3 v2 = glm::make_vec3(vertices + indices[i+1]*3);
		vec3 v3 = glm::make_vec3(vertices + indices[i+2]*3);
		
		// Cross product to get a normal
		vec3 normal = glm::normalize(glm::cross(v2-v1, v3-v1));
		
		// Add face normal to the normal of each vertex
		for(index_t j=i; j<i+3; ++j)
		{
			normals[indices[j]*3]+= normal.x;
			normals[indices[j]*3+1]+= normal.y;
			normals[indices[j]*3+2]+= normal.z;
		}
	}
	
	setVertexAttrib(normalLayout, normals, vertexBuffer->count(), 3);
	delete[] normals;
}

float Mesh::computeRadius(void)
{
	const unsigned layout = 0;
	auto it = mAttribBuffers.find(layout);
	if(it == mAttribBuffers.end()) 
	{
		mRadius = 0.f;
		return 0.f;
	}
	
	sptr<Attrib> vertexBuffer = it->second;
	Assert(vertexBuffer->size == 3);
	Assert(vertexBuffer->type == GL_FLOAT);
	
	const float *vertices = reinterpret_cast<float*>(vertexBuffer->data());
	
	float radius2 = 0.f;
	for(index_t i=0; i<vertexBuffer->count(); i+=3)
	{
		float d2 = glm::length2(vec3(vertices[i], vertices[i+1], vertices[i+2]));
		radius2 = std::max(radius2, d2);
	}
	
	return mRadius = std::sqrt(radius2);
}

float Mesh::getRadius(void) const
{
	return mRadius;
}

int Mesh::drawElements(void)
{
	return drawElements(0, mIndexBuffer->count());
}

int Mesh::drawElements(index_t first, index_t count) const
{
	enableBuffers();
	mIndexBuffer->bind();
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, mIndexBuffer->offset(first));
	disableBuffers();
	return count/3;
}

float Mesh::intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection)
{
	// If empty, there is no intersection
	if(mIndexBuffer->count() == 0)
		return std::numeric_limits<float>::infinity();

	// If out of bounding sphere, there is no intersection
	if(mRadius >= 0.f
		&& glm::length(pos) > mRadius
		&& intersectSphere(pos, move, vec3(0.f, 0.f, 0.f), radius) > 1.f)
		return std::numeric_limits<float>::infinity();

	const unsigned layout = 0;
	auto it = mAttribBuffers.find(layout);
	if(it == mAttribBuffers.end()) 
		return std::numeric_limits<float>::infinity();
	
	sptr<Attrib> vertexBuffer = it->second;
	Assert(vertexBuffer->size == 3);
	Assert(vertexBuffer->type == GL_FLOAT);
	
	const float *vertices = reinterpret_cast<float*>(vertexBuffer->data());
	const index_t *indices = mIndexBuffer->data();

	float nearest = std::numeric_limits<float>::infinity();
	vec3 nearestintersection;
	for(index_t i=0; i<mIndexBuffer->count(); ++i)
	{
		vec3 v1 = glm::make_vec3(vertices + indices[i]*3);
		vec3 v2 = glm::make_vec3(vertices + indices[++i]*3);
		vec3 v3 = glm::make_vec3(vertices + indices[++i]*3);
		
		float t = intersectFace(pos, move, radius, v1, v2, v3, intersection);
		if(t < nearest)
		{
			nearest = t;
			if(intersection) nearestintersection = *intersection;
		}
	}
	
	if(intersection) *intersection = nearestintersection;
	return nearest;
}

void Mesh::enableBuffers(void) const
{
	for(auto &p : mAttribBuffers)
	{
		unsigned layout = p.first;
		glEnableVertexAttribArray(layout);
		
		auto attrib = p.second;
		attrib->bind();
		glVertexAttribPointer(
			layout,				// layout
			attrib->size,		// size
			attrib->type,		// type
			attrib->normalize,	// normalize
			0,					// stride
			NULL);
	}
}

void Mesh::disableBuffers(void) const
{
	for(auto &p : mAttribBuffers)
	{
		unsigned layout = p.first;
		glDisableVertexAttribArray(layout);
	}
}

}
