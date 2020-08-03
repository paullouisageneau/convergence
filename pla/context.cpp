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

#include "pla/context.hpp"

namespace pla {

Context::Context(const mat4 &projection, const mat4 &camera)
    : mProjection(projection), mView(glm::inverse(camera)), mModel(mat4(1.f)),
      mTransform(mProjection * mView * mModel), mCameraPosition(camera * vec4(0.f, 0.f, 0.f, 1.f)),
      mFrustum(mTransform), mDepthTestEnabled(true), mBlendingEnabled(false),
      mReverseCullingEnabled(false) {

	setUniform("projection", mProjection);
	setUniform("view", mView);
	setUniform("model", mModel);
	setUniform("transform", mTransform);
}

Context::~Context(void) {}

const mat4 &Context::projection(void) const { return mProjection; }

const mat4 &Context::view(void) const { return mView; }

const mat4 &Context::model(void) const { return mModel; }

const mat4 &Context::transform(void) const { return mTransform; }

const vec3 &Context::cameraPosition(void) const { return mCameraPosition; }

const Frustum &Context::frustum(void) const { return mFrustum; }

void Context::enableDepthTest(bool enabled) { mDepthTestEnabled = enabled; }

void Context::enableBlending(bool enabled) { mBlendingEnabled = enabled; }

void Context::enableReverseCulling(bool enabled) { mReverseCullingEnabled = enabled; }

void Context::prepare(sptr<Program> program) const {
	int unit = program->nextTextureUnit();
	for (auto p : mUniforms)
		if (program->hasUniform(p.first))
			p.second->apply(p.first, program, unit); // may increment unit

	if (mDepthTestEnabled) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	if (mBlendingEnabled) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}

	if (mReverseCullingEnabled) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	} else {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
}

Context Context::transform(const mat4 &matrix) const {
	Context sub = *this;
	sub.mModel *= matrix;
	sub.mTransform *= matrix;
	sub.setUniform("model", sub.mModel);
	sub.setUniform("transform", sub.mTransform);
	return sub;
}

} // namespace pla
