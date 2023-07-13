#version 450

// Variance Field Ambient Occlusion (for sky lights) 
// - By ZilverBlade 12-07-2023

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec4 outColor; // (Depth, DepthSQ, SlopeDelta, Occlusion)

layout (set = 0, binding = 0) uniform sampler2D varianceImage;

layout (push_constant) uniform myParams {
    mat4 projection;
	float slopeOcclusionRadius;
} push;

float fetchDepthTexel(vec2 offset) {
    return texture(varianceImage, fragUV + offset).r;
}
vec2 fetchVTexel(vec2 offset) {
    return  texture(varianceImage, fragUV + offset).rg;
}
void main(){
    vec2 texelSize = vec2(1.0) / vec2(textureSize(varianceImage, 0).xy);
    vec2 vd = fetchVTexel(vec2(0.0));
	
	// depth must be linear due to orthographic projection
	
	mat4 invProj = inverse(push.projection);
	vec4 clipNow = vec4(fragUV * 2.0 - 1.0, vd.x, 1.0);
	vec4 view = invProj * clipNow;
	
	float ddxy = fwidth(vd.x);
	float mean = exp(-1.2 * abs(ddxy / vd.x));
	
	vec2 vFilter = vd;
	
	const float varianceRadius = 2.1; // controls what mean size is required to have a noticeable AO effect
    vFilter += fetchVTexel(vec2(varianceRadius * texelSize.x, varianceRadius * texelSize.y));
    vFilter += fetchVTexel(vec2(-varianceRadius * texelSize.x, -varianceRadius * texelSize.y));
    vFilter += fetchVTexel(vec2(varianceRadius * texelSize.x, -varianceRadius * texelSize.y));
    vFilter += fetchVTexel(vec2(-varianceRadius * texelSize.x, varianceRadius * texelSize.y));
    vFilter += fetchVTexel(2.0 * vec2(varianceRadius *texelSize.x, varianceRadius * texelSize.y));
    vFilter += fetchVTexel(2.0 * vec2(-varianceRadius *texelSize.x, -varianceRadius * texelSize.y));
    vFilter += fetchVTexel(2.0 * vec2(varianceRadius* texelSize.x, -varianceRadius *texelSize.y));
    vFilter += fetchVTexel(2.0 * vec2(-varianceRadius * texelSize.x, varianceRadius *texelSize.y));
    vFilter /= 9.0;

    float radius = 7.0; // make push
    float occlusion = 0.0;
	
	vec2 cc = radius * vec2(texelSize.x, texelSize.y);
	
	// todo: random rotation per fragment because when i first wrote this i didnt know shit about montecarlo
    float sd0 =     fetchDepthTexel(cc.xy * vec2(1.0, 0.0));
    float sd30 =    fetchDepthTexel(cc.xy * vec2(sqrt(3.0) / 2.0, 0.5));
    float sd60 =    fetchDepthTexel(cc.xy * vec2(0.5, sqrt(3.0) / 2.0));
    float sd90 =    fetchDepthTexel(cc.xy * vec2(0.0, 1.0));
    float sd120 =   fetchDepthTexel(cc.xy * vec2(-0.5, sqrt(3.0) / 2.0));
    float sd150 =   fetchDepthTexel(cc.xy * vec2(-sqrt(3.0) / 2.0, 0.5));
    float sd180 =   fetchDepthTexel(cc.xy * vec2(-1.0, 0.0));
    float sd210 =   fetchDepthTexel(cc.xy * vec2(-sqrt(3.0) / 2.0, -0.5));
    float sd240 =   fetchDepthTexel(cc.xy * vec2(-0.5, -sqrt(3.0) / 2.0));
    float sd270 =   fetchDepthTexel(cc.xy * vec2(0.0, -1.0));
    float sd300 =   fetchDepthTexel(cc.xy * vec2(0.5, -sqrt(3.0) / 2.0));
    float sd330 =   fetchDepthTexel(cc.xy * vec2(sqrt(3.0) / 2.0, -0.5));

	float zRadius = radius / 600.0; // 600 is the distance in depth that the ortho projection we use is
	occlusion += float(int(bool(sd0 <= vd.x))) 		*  	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd0));
	occlusion += float(int(bool(sd30 <= vd.x))) 	* 	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd30));
	occlusion += float(int(bool(sd60 <= vd.x))) 	* 	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd60));
	occlusion += float(int(bool(sd90 <= vd.x))) 	* 	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd90));
	occlusion += float(int(bool(sd120 <= vd.x))) 	*	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd120));
	occlusion += float(int(bool(sd150 <= vd.x))) 	*	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd150));
	occlusion += float(int(bool(sd180 <= vd.x))) 	*	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd180));
	occlusion += float(int(bool(sd210 <= vd.x))) 	*	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd210));
	occlusion += float(int(bool(sd240 <= vd.x))) 	*	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd240));
	occlusion += float(int(bool(sd270 <= vd.x))) 	*	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd270));
	occlusion += float(int(bool(sd300 <= vd.x))) 	*	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd300));
	occlusion += float(int(bool(sd330 <= vd.x))) 	*	smoothstep(0.0, 1.0, zRadius / abs(vd.x - sd330));

    occlusion /= 12.0;

    outColor = vec4(vFilter.x, vFilter.y, mean, occlusion);
	
	// variance check with vFilter.xy, compare depth with slope for light ray probability, and add the occlusion
}