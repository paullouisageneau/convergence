#version 110

uniform vec3 lightPosition;

varying vec3 fragNormal;
varying vec3 fragLight;
varying vec3 fragAmbient;
varying vec3 fragDiffuse;
varying vec3 fragSmoothness;

void main()
{
	float light = clamp(dot(fragNormal, -fragLight), 0.0, 1.0);

	float z = gl_FragCoord.z/gl_FragCoord.w;
	float fog = min((exp2(0.2*z) - 1.0)*0.02, 1.0);

	gl_FragColor = vec4((fragAmbient + fragDiffuse * light) * (1.0 - fog), 1.0);
}

