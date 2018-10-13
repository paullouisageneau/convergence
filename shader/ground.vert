#version 110

uniform mat4 transform;
uniform mat4 modelview;
uniform vec3 lightPosition;

attribute vec3 position;
attribute vec3 normal;
attribute vec4 material;

varying vec3 fragPosition;
varying vec3 fragNormal;
varying vec3 fragLight;
varying vec4 fragMaterial;

void main() 
{
	fragPosition = position;
	fragNormal = normal;
	fragLight = normalize(position - lightPosition);
	fragMaterial = material;
	gl_Position = transform*vec4(position, 1.0);
}

