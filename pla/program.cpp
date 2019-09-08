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

#include "pla/program.hpp"

namespace pla {

Program::Program(void) {
	mProgram = glCreateProgram();
	if (!mProgram)
		throw std::runtime_error("Unable to create shader program");
}

Program::Program(sptr<Shader> vertexShader, sptr<Shader> fragmentShader, bool mustLink) {
	mProgram = glCreateProgram();
	if (!mProgram)
		throw std::runtime_error("Unable to create shader program");

	vertexShader->compile();
	fragmentShader->compile();

	attachShader(vertexShader);
	attachShader(fragmentShader);

	if (mustLink)
		link();
}

Program::~Program(void) {
	glDeleteProgram(mProgram);
	mShaders.clear();
}

void Program::attachShader(sptr<Shader> shader) {
	if (mShaders.insert(shader).second)
		glAttachShader(mProgram, shader->mShader);
}

void Program::detachShader(sptr<Shader> shader) {
	if (mShaders.erase(shader))
		glDetachShader(mProgram, shader->mShader);
}

void Program::bindAttribLocation(unsigned index, const string &name) {
	glBindAttribLocation(mProgram, index, name.c_str());
}

void Program::link(void) {
	mUniformLocations.clear();
	mAttribLocations.clear();

	glLinkProgram(mProgram);

	GLint status = GL_TRUE;
	glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		GLint logsize;
		glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &logsize);
		char *log = new char[logsize + 1];
		glGetProgramInfoLog(mProgram, logsize, &logsize, log);
		throw std::runtime_error("Unable to link shader: \n" + string(log));
	}
}

void Program::bind(void) { glUseProgram(mProgram); }

void Program::unbind(void) { glUseProgram(0); }

bool Program::hasUniform(const string &name) { return (getUniformLocation(name) != -1); }

bool Program::hasVertexAttrib(const string &name) { return (getAttribLocation(name) != -1); }

int Program::getUniformLocation(const string &name) {
	auto it = mUniformLocations.find(name);
	if (it != mUniformLocations.end())
		return it->second;

	int location = glGetUniformLocation(mProgram, name.c_str());
	mUniformLocations[name] = GLuint(location);
	return location;
}

int Program::getAttribLocation(const string &name) {
	auto it = mAttribLocations.find(name);
	if (it != mAttribLocations.end())
		return it->second;

	int location = glGetAttribLocation(mProgram, name.c_str());
	mUniformLocations[name] = location;
	return location;
}

void Program::setUniform(const string &name, float value) {
	bind();
	glUniform1f(getUniformLocation(name.c_str()), value);
}

void Program::setUniform(const string &name, int value) {
	bind();
	glUniform1i(getUniformLocation(name.c_str()), value);
}

void Program::setUniform(const string &name, const float *values, int count) {
	bind();
	glUniform1fv(getUniformLocation(name.c_str()), count, values);
}

void Program::setUniform(const string &name, const int *values, int count) {
	bind();
	glUniform1iv(getUniformLocation(name.c_str()), count, values);
}

void Program::setUniform(const string &name, const vec3 &value) {
	bind();
	glUniform3fv(getUniformLocation(name.c_str()), 1, glm::value_ptr(value));
}

void Program::setUniform(const string &name, const vec4 &value) {
	bind();
	glUniform4fv(getUniformLocation(name.c_str()), 1, glm::value_ptr(value));
}

void Program::setUniform(const string &name, const mat4 &value) {
	bind();
	glUniformMatrix4fv(getUniformLocation(name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Program::setVertexAttrib(const string &name, float value) {
	bind();
	glVertexAttrib1f(getAttribLocation(name.c_str()), value);
}

void Program::setVertexAttrib(const string &name, const float *values) {
	bind();
	glVertexAttrib1fv(getAttribLocation(name.c_str()), values);
}

void Program::setVertexAttrib(const string &name, const vec3 &value) {
	bind();
	glVertexAttrib3fv(getAttribLocation(name.c_str()), glm::value_ptr(value));
}

void Program::setVertexAttrib(const string &name, const vec4 &value) {
	bind();
	glVertexAttrib4fv(getAttribLocation(name.c_str()), glm::value_ptr(value));
}

} // namespace pla
