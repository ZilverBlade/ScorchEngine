#version 450
// 11 weights
const float gaussianWeight11[11] = float[](
    0.0093,
    0.028002,
    0.065984,
    0.121703,
    0.175713,
    0.198596,
    0.175713,
    0.121703,
    0.065984,
    0.028002,
    0.0093
);
const float gaussianWeight7[7] = float[](
	0.0027,
	0.0451,	
	0.2415,	
    0.4215,	
	0.2415,	
	0.0451,	
	0.0027
);

#define	GAUSSIAN_KERNEL_SIZE_11
// 	GAUSSIAN_KERNEL_SIZE_7

#define GAUSSIAN_PASS_HORIZONTAL
// 	GAUSSIAN_PASS_VERTICAL

//	TEX_FORMAT_R
//	TEX_FORMAT_RG
//	TEX_FORMAT_RGB
#define	TEX_FORMAT_RGBA

#ifdef TEX_FORMAT_R
#define txFormat float
#define outFormat float
#endif
#ifdef TEX_FORMAT_RG
#define txFormat vec2
#define outFormat vec2
#endif
#ifdef TEX_FORMAT_RGB
#define txFormat vec3
#define outFormat vec3
#endif
#ifdef TEX_FORMAT_RGBA
#define txFormat vec4
#define outFormat vec4
#endif

#ifdef GAUSSIAN_KERNEL_SIZE_7
#define kernelSize 7
#define halfKernelSize 3
#define gaussianArray gaussianWeight7
#endif
#ifdef GAUSSIAN_KERNEL_SIZE_11
#define kernelSize 11
#define halfKernelSize 5
#define gaussianArray gaussianWeight11
#endif

layout (location = 0) in vec2 fragUV;
layout (location = 0) out outFormat outColor;

layout (set = 0, binding = 0) uniform sampler2D sceneInputImage;

txFormat fetchTexel(vec2 coord) {
#ifdef TEX_FORMAT_R
    return texture(sceneInputImage, coord).r;
#endif
#ifdef TEX_FORMAT_RG
    return texture(sceneInputImage, coord).rg;
#endif
#ifdef TEX_FORMAT_RGB
    return texture(sceneInputImage, coord).rgb;
#endif
#ifdef TEX_FORMAT_RGBA
    return texture(sceneInputImage, coord).rgba;
#endif
}

void main()	{  
    txFormat blurColor = txFormat(0.0);
#ifdef GAUSSIAN_PASS_HORIZONTAL
 float texelSize = 1.0 / textureSize(sceneInputImage, 0).x;
#endif
#ifdef GAUSSIAN_PASS_VERTICAL   
 float texelSize = 1.0 / textureSize(sceneInputImage, 0).y;
#endif

    for (int i = 0; i < kernelSize; i++){
        int kernelOffset = i - halfKernelSize;
        blurColor += fetchTexel(vec2(
#ifdef GAUSSIAN_PASS_HORIZONTAL
		fragUV.x  + texelSize * kernelOffset, 
		fragUV.y
#endif
#ifdef GAUSSIAN_PASS_VERTICAL   
		fragUV.x,
		fragUV.y + texelSize * kernelOffset
#endif
		)) * gaussianArray[i];
    }

#ifdef TEX_FORMAT_RGB
    outColor = outFormat(blurColor, 1.0);
#else
    outColor = blurColor;
#endif
}