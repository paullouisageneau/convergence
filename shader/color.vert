#version 110

uniform mat4 transform;
uniform mat4 modelview;
uniform vec3 lightPosition;

attribute vec3 position;
attribute vec3 normal;
attribute vec3 color;

varying vec3 fragNormal;
varying vec3 fragLight;
varying vec3 fragColor;

void main() 
{
	fragNormal = normal;
	fragLight = normalize(position - lightPosition);
	//fragColor = color;
	fragColor = vec3(0.933, 0.510, 0.933);
	gl_Position = transform*vec4(position, 1.0);
}

