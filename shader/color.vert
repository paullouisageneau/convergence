#version 130

uniform mat4 transform;
uniform mat4 modelview;
uniform vec3 lightPosition;

in vec3 position;
in vec3 normal;
in vec3 color;

out vec3 fragNormal;
out vec3 fragLight;
out vec3 fragColor;

void main() 
{
	fragNormal = normal;
	fragLight = normalize(position-lightPosition);
	fragColor = color;
	
	gl_Position = transform*vec4(position, 1.0);
}

