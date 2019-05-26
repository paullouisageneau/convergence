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

#ifndef PLA_PROGRAM_H
#define PLA_PROGRAM_H

#include "pla/include.hpp"
#include "pla/opengl.hpp"
#include "pla/resource.hpp"
#include "pla/shader.hpp"
#include "pla/buffer.hpp"
#include "pla/string.hpp"
#include "pla/linalg.hpp"

namespace pla
{

class Program final : public Resource {
public:
	Program(void);
	Program(sptr<Shader> vertexShader, sptr<Shader> fragmentShader, bool mustLink = false);
	~Program(void);

	void attachShader(sptr<Shader> shader);
	void detachShader(sptr<Shader> shader);
	void bindAttribLocation(unsigned index, const string &name);
	void link(void);
	void bind(void);
	void unbind(void);

	bool hasUniform(const string &name);
	bool hasVertexAttrib(const string &name);

	int getUniformLocation(const string &name);
	int getAttribLocation(const string &name);

	void setUniform(const string &name, float value);
	void setUniform(const string &name, int value);
	void setUniform(const string &name, const float *values, int count);
	void setUniform(const string &name, const int *values, int count);
	void setUniform(const string &name, const vec3 &value);
	void setUniform(const string &name, const vec4 &value);
	void setUniform(const string &name, const mat4 &value);

	void setVertexAttrib(const string &name, float value);
	void setVertexAttrib(const string &name, const float *values);
	void setVertexAttrib(const string &name, const vec3 &value);
	void setVertexAttrib(const string &name, const vec4 &value);

private:
	GLuint mProgram;

	std::set<sptr<Shader> > mShaders;
	std::map<string, int> mUniformLocations;	// Cache
	std::map<string, int> mAttribLocations;		// Cache
};
}

#endif
