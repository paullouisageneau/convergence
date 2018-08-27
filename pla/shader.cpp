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

#include "pla/shader.hpp"

namespace pla
{

Shader::Shader(GLenum type) :
	mType(type)
{
	mShader = glCreateShader(type);
	if(!mShader) throw std::runtime_error("Unable to create shader");
}

Shader::~Shader(void)
{
	glDeleteShader(mShader);
}
	
void Shader::setSource(const std::string &source)
{
	const char *str = source.data();
	GLint len = source.size();
	glShaderSource(mShader, 1, &str, &len);
}

void Shader::loadFile(const std::string &filename)
{
	try {
		std::ifstream file(filename.c_str());
		if(!file.is_open()) throw std::runtime_error("Unable to open file: "+filename);

		std::string source;
		while(file.good())
		{
			std::string line;
			getline(file, line);

			#ifdef USE_OPENGL_ES
			if(line.substr(0, 8) == "#version")
			{
				source+= "#version 100\n";
				source+= "precision highp float;\n";
				continue;
			}
			#endif
			
			source+= line + '\n';
		}

		setSource(source);
	}
	catch(std::exception &e)
	{
		throw std::runtime_error("Shader loading failed: " + string(e.what()));
	}
}

void Shader::compile(void)
{
	glCompileShader(mShader);
	
	GLint status = GL_TRUE;
	glGetShaderiv(mShader, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE)
	{
		GLint logsize;
		glGetShaderiv(mShader, GL_INFO_LOG_LENGTH, &logsize);
		char *buffer = new char[logsize+1];
		glGetShaderInfoLog(mShader, logsize, &logsize, buffer);
		string log(buffer);
		delete[] buffer;
		throw std::runtime_error("Unable to compile shader: \n" + log);
	}
}

}
