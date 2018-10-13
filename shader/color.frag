#version 110

uniform vec3 lightPosition;

varying vec3 fragNormal;
varying vec3 fragLight;
varying vec3 fragColor;

void main()
{
	float light = clamp(dot(fragNormal, -fragLight), 0.0, 1.0);
	
	float z = gl_FragCoord.z/gl_FragCoord.w;
	float fog = min((exp2(0.2*z) - 1.0)*0.02, 1.0);
	
	gl_FragColor = vec4(fragColor * (1.0 + light * fog)*0.5, 1.0);
}

