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

#ifndef P3D_SHADER_H
#define P3D_SHADER_H

#include "p3d/include.hpp"
#include "p3d/resource.hpp"

namespace pla
{

class Shader : public Resource
{  
public:
	// type = GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
	Shader(GLenum type = GL_VERTEX_SHADER);
	virtual ~Shader(void);
	
	void setSource(const std::string &source);
	void loadFile(const std::string &filename);
	void compile(void);
	
private:
	GLuint mShader;
	GLenum mType;
	
	friend class Program;
};

class VertexShader : public Shader
{  
public:
	VertexShader(const std::string &filename = "") : Shader(GL_VERTEX_SHADER)
		{ if(!filename.empty()) loadFile(filename); }
	~VertexShader(void) {}
};

class FragmentShader : public Shader
{  
public:
	FragmentShader(const std::string &filename = "") : Shader(GL_FRAGMENT_SHADER)
		{ if(!filename.empty()) loadFile(filename); }
	~FragmentShader(void) {}
};

}

#endif
