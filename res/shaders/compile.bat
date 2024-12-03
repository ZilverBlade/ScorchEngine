@echo off

glslc.exe -O -fshader-stage=vertex triangle.vsh -o spirv/triangle.vsh.spv  
glslc.exe -O -fshader-stage=fragment triangle.fsh -o spirv/triangle.fsh.spv

glslc.exe -O -fshader-stage=vertex full_screen.vsh -o spirv/full_screen.vsh.spv
glslc.exe -O -fshader-stage=vertex full_screen_cube.vsh -o spirv/full_screen_cube.vsh.spv

glslc.exe -O -fshader-stage=fragment screen_correct.fsh -o spirv/screen_correct.fsh.spv

glslc.exe -O -fshader-stage=vertex model.vsh -o spirv/model.vsh.spv
glslc.exe -O -g -fshader-stage=fragment forward_shading.fsh -o spirv/forward_shading.fsh.spv

glslc.exe -O -fshader-stage=vertex depth.vsh -o spirv/depth.vsh.spv
glslc.exe -O -fshader-stage=fragment depth.fsh -o spirv/depth.fsh.spv

glslc.exe -O -fshader-stage=fragment skybox.fsh -o spirv/skybox.fsh.spv
glslc.exe -O -fshader-stage=vertex skybox.vsh -o spirv/skybox.vsh.spv

glslc.exe -O -fshader-stage=fragment gaussian_h.fsh -o spirv/gaussian_h.fsh.spv
glslc.exe -O -fshader-stage=fragment gaussian_v.fsh -o spirv/gaussian_v.fsh.spv

glslc.exe -O -fshader-stage=fragment shadow.fsh -o spirv/shadow.fsh.spv
glslc.exe -O -fshader-stage=vertex shadow.vsh -o spirv/shadow.vsh.spv

glslc.exe -O -fshader-stage=fragment shadow_rsm.fsh -o spirv/shadow_rsm.fsh.spv
glslc.exe -O -fshader-stage=vertex shadow_rsm.vsh -o spirv/shadow_rsm.vsh.spv

glslc.exe -O -fshader-stage=fragment vfao.fsh -o spirv/vfao.fsh.spv
glslc.exe -O -fshader-stage=vertex vfao.vsh -o spirv/vfao.vsh.spv
glslc.exe -O -g -fshader-stage=fragment vfao_fields.fsh -o spirv/vfao_fields.fsh.spv

glslc.exe -O -g -fshader-stage=fragment caustics.fsh -o spirv/caustics.fsh.spv
glslc.exe -O -g -fshader-stage=fragment caustics_injection.fsh -o spirv/caustics_injection.fsh.spv
glslc.exe -O -fshader-stage=compute caustics_clear.csh -o spirv/caustics_clear.csh.spv
glslc.exe -O -fshader-stage=fragment caustics_scale.fsh -o spirv/caustics_scale.fsh.spv

glslc.exe -O -fshader-stage=fragment envirrgen.fsh -o spirv/envirrgen.fsh.spv
glslc.exe -O -fshader-stage=fragment envprefiltergen.fsh -o spirv/envprefiltergen.fsh.spv
glslc.exe -O -fshader-stage=fragment envbrdfgen.fsh -o spirv/envbrdfgen.fsh.spv

glslc.exe -O -fshader-stage=compute lpv_temporal_blend.csh -o spirv/lpv_temporal_blend.csh.spv
glslc.exe -O -fshader-stage=compute lpv_clear.csh -o spirv/lpv_clear.csh.spv
glslc.exe -O -g -fshader-stage=fragment lpv_injection.fsh -o spirv/lpv_injection.fsh.spv
glslc.exe -O -g -fshader-stage=fragment lpv_propagation.fsh -o spirv/lpv_propagation.fsh.spv

glslc.exe -O -g -fshader-stage=vertex vsdf_preview.vsh -o spirv/vsdf_preview.vsh.spv
glslc.exe -O -g -fshader-stage=fragment vsdf_preview.fsh -o spirv/vsdf_preview.fsh.spv

pause