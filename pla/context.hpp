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

#include "pla/frustum.hpp"
#include "pla/include.hpp"
#include "pla/linalg.hpp"
#include "pla/program.hpp"
#include "pla/texture.hpp"

#include <vector>

namespace pla {

// Rendering context
class Context {
public:
	Context(const mat4 &projection, const mat4 &camera);
	~Context(void);

	const mat4 &projection(void) const;
	const mat4 &view(void) const;
	const mat4 &model(void) const;
	const mat4 &transform(void) const;
	const vec3 &cameraPosition(void) const;
	const Frustum &frustum(void) const;
	void prepare(sptr<Program> program) const; // set uniforms in program

	void enableDepthTest(bool enabled);
	void enableBlending(bool enabled);
	void enableReverseCulling(bool enabled);

	Context transform(const mat4 &matrix) const;

	template <typename T> void setUniform(const string &name, const T &value);
	template <typename T> void setUniform(const string &name, const T *value, int count);
	template <typename T> void setUniform(const string &name, std::vector<T> values);

private:
	mat4 mProjection, mView, mModel, mTransform;
	vec3 mCameraPosition;
	Frustum mFrustum;
	bool mDepthTestEnabled;
	bool mBlendingEnabled;
	bool mReverseCullingEnabled;

	class UniformContainer {
	public:
		virtual void apply(const string &name, sptr<Program> program, int &unit) const = 0;
	};

	template <typename T> class UniformContainerImpl;

	std::map<string, sptr<UniformContainer>> mUniforms;
};

template <typename T> class Context::UniformContainerImpl final : public Context::UniformContainer {
public:
	UniformContainerImpl(T v) { value = std::move(v); }
	void apply(const string &name, sptr<Program> program, int &unit) const {
		program->setUniform(name, value);
	}

private:
	T value;
};

template <typename T>
class Context::UniformContainerImpl<std::vector<T>> final : public Context::UniformContainer {
public:
	UniformContainerImpl(std::vector<T> v) { values = std::move(v); }
	void apply(const string &name, sptr<Program> program, int &unit) const {
		program->setUniform(name, values.data(), values.size());
	}

private:
	std::vector<T> values;
};

template <>
class Context::UniformContainerImpl<shared_ptr<Texture>> final : public Context::UniformContainer {
public:
	UniformContainerImpl(shared_ptr<Texture> t) { texture = t; }
	void apply(const string &name, sptr<Program> program, int &unit) const {
		texture->activate(unit);
		program->setUniform(name, unit);
		++unit;
	}

private:
	shared_ptr<Texture> texture;
};

template <typename T> void Context::setUniform(const string &name, const T &value) {
	auto p = std::make_shared<UniformContainerImpl<T>>(value);
	mUniforms[name] = p;
}

template <typename T> void Context::setUniform(const string &name, const T *value, int count) {
	std::vector<T> v(value, value + count);
	setUniform(name, std::move(v));
}

template <typename T> void Context::setUniform(const string &name, std::vector<T> values) {
	auto p = std::make_shared<UniformContainerImpl<std::vector<T>>>(std::move(values));
	mUniforms[name] = p;
}

} // namespace pla

#endif
