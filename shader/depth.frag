#version 330
precision highp float;

uniform float nearPlane;
uniform float farPlane;
uniform vec3 lightPosition;

in vec3 fragPosition;

out vec4 fragColor;

float map(float value, float min1, float max1, float min2, float max2) {
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main() {
    gl_FragDepth = map(length(fragPosition - lightPosition), nearPlane, farPlane, 0.0, 1.0);
}

