#version 130

uniform vec3 lightPosition;

in vec3 fragNormal;
in vec3 fragLight;
in vec3 fragColor;

out vec3 color;

void main()
{
	float light = clamp(dot(fragNormal, -fragLight), 0, 1);
	float z = gl_FragCoord.z/gl_FragCoord.w;
	float fog = exp2(-0.002*z*z);
	color = fragColor * light * fog;
}

