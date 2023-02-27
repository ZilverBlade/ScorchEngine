#version 450
#extension GL_GOOGLE_include_directive : enable
#include "pbr_math.glsl"
#include "constants.glsl"

layout (set = 0, binding = 0) uniform samplerCube environmentMap;

layout (location = 0) in vec3 fragUV;
layout (location = 0) out vec4 outDiffuse;

 float3 SH_Evaluation( Llm,                        AlmYlm,                        thetaPhi : VPOS ) : COLOR  {   

 } 

void main() {
	const vec3 normal = normalize(fragUV);
	
	const vec3 up = vec3(0.0, 1.0, 0.0);
	const vec3 right = normalize(cross(up, normal));
	
	vec3 irradiance = vec3(0.0); 
	
	float sampleDelta = 0.025;
	float nrSamples = 0.0; 
	float thetaPhi = 0.0;
	float sum = 0;
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{  
		for_all {T0:Llm}      
		T1 = map_to_tile_and_offset(T0, thetaPhi)     
		weighted_BRDF = AlmYlm[T1]      
		light_environment = inputEnvironmentMap[T0]      
		sum += light_environment * weighted_BRDF    
		return sum  
	}
	outDiffuse = vec4(PI * irradiance * (1.0 / float(nrSamples)), 1.0);
}