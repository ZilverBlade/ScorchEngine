#version 450

layout (location = 0) out vec2 variance; // R16G16

void main(){
	float d = gl_FragCoord.z;
	float moment_y = d * d;
	
	variance = vec2(d, moment_y);
}