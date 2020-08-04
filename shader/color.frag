#version 330
precision highp float;

uniform vec3 lightPosition;

in vec3 fragNormal;
in vec3 fragLight;
in vec3 fragAmbient;
in vec3 fragDiffuse;

out vec4 fragColor;

void main()
{
	float light = clamp(dot(fragNormal, -fragLight), 0.0, 1.0);

	float z = gl_FragCoord.z/gl_FragCoord.w;
	float fog = min((exp2(0.2*z) - 1.0)*0.02, 1.0);

	fragColor = vec4((fragAmbient + fragDiffuse * light) * (1.0 - fog), 1.0);
}

