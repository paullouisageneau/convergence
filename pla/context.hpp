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

#ifndef PLA_CONTEXT_H
#define PLA_CONTEXT_H

#include "pla/include.hpp"
#include "pla/program.hpp"
#include "pla/frustum.hpp"
#include "pla/linalg.hpp"

namespace pla
{

// Rendering context
class Context
{
public:
	Context(const mat4 &projection,  const mat4 &camera);
	~Context(void);
	
	const mat4 &projection(void) const;
	const mat4 &modelview(void) const;
	const mat4 &transform(void) const;
	const vec3 &cameraPosition(void) const;
	const Frustum &frustum(void) const;
	void prepare(sptr<Program> program) const;	// set uniforms in program
	
	template<typename T> void setUniform(const string &name, const T &value);
	
private:
	mat4 mProjection, mModelview, mTransform;
	vec3 mCameraPosition;
	Frustum mFrustum;
	
	class UniformContainer
	{
	public:
		virtual void apply(const string &name, sptr<Program> program) const = 0;
	};
	
	template<typename T>
	class UniformContainerImpl : public UniformContainer
	{
	public:
		UniformContainerImpl(const T &v) { value = v; }
		void apply(const string &name, sptr<Program> program) const { program->setUniform(name, value); }
		
	private:
		T value;
	};
	
	std::map<string, sptr<UniformContainer> > mUniforms;
};

template<typename T> 
void Context::setUniform(const string &name, const T &value)
{
	sptr<UniformContainer> p = std::make_shared<UniformContainerImpl<T> >(value);
	mUniforms.insert(std::make_pair(name, p));
}

}

#endif

