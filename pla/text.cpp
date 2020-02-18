/***************************************************************************
 *   Copyright (C) 2006-2020 by Paul-Louis Ageneau                         *
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

#include "pla/text.hpp"

namespace pla {

Text::Text(shared_ptr<Font> font, shared_ptr<Program> program, std::string content,
           size_t resolution)
    : mFont(font), mProgram(program), mContent(std::move(content)) {
	setProgram(program);
	generate(resolution);
}

Text::~Text() {}

std::string Text::content() const { return mContent; }

void Text::setContent(std::string content, size_t resolution) {
	if (content != mContent) {
		mContent = std::move(content);
		generate(resolution);
	}
}

int Text::draw(const Context &context) const {
	Context subContext = context;
	subContext.setUniform("color", mTexture);
	subContext.enableBlending(true);
	return Object::draw(subContext);
}

void Text::generate(size_t resolution) {
	auto image = mFont->render(mContent, resolution, vec3(0.f, 0.f, 0.f));
	mTexture = std::make_shared<Texture>(image);
	mTexture->enableClamping(true);

	const float w = float(image->width()) / resolution;
	const float h = float(image->height()) / resolution;
	const vec3 vertices[4] = {{-w * 0.5f, -h * 0.5f, 0.f},
	                          {w * 0.5f, -h * 0.5f, 0.f},
	                          {w * 0.5f, h * 0.5f, 0.f},
	                          {-w * 0.5f, h * 0.5f, 0.f}};
	const vec2 texCoord[4] = {{0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}};
	const index_t indices[6] = {0, 1, 2, 0, 2, 3};

	auto mesh = std::make_shared<Mesh>();
	mesh->setIndices(indices, 6);
	mesh->setVertexAttrib(0, glm::value_ptr(vertices[0]), 4 * 3, 3);
	mesh->setVertexAttrib(1, glm::value_ptr(texCoord[0]), 4 * 2, 2);
	setMesh(mesh);
}

} // namespace pla

