@echo off

glslc.exe -O -fshader-stage=vertex triangle.vsh -o spirv/triangle.vsh.spv  
glslc.exe -O -fshader-stage=fragment triangle.fsh -o spirv/triangle.fsh.spv

glslc.exe -O -fshader-stage=vertex full_screen.vsh -o spirv/full_screen.vsh.spv
glslc.exe -O -fshader-stage=vertex full_screen_cube.vsh -o spirv/full_screen_cube.vsh.spv

glslc.exe -O -fshader-stage=fragment screen_correct.fsh -o spirv/screen_correct.fsh.spv

glslc.exe -O -fshader-stage=vertex model.vsh -o spirv/model.vsh.spv
glslc.exe -O -fshader-stage=fragment forward_shading.fsh -o spirv/forward_shading.fsh.spv

glslc.exe -O -fshader-stage=vertex depth.vsh -o spirv/depth.vsh.spv
glslc.exe -O -fshader-stage=fragment depth.fsh -o spirv/depth.fsh.spv

glslc.exe -O -fshader-stage=fragment skybox.fsh -o spirv/skybox.fsh.spv
glslc.exe -O -fshader-stage=vertex skybox.vsh -o spirv/skybox.vsh.spv


glslc.exe -O -fshader-stage=fragment envirrgen.fsh -o spirv/envirrgen.fsh.spv
glslc.exe -O -fshader-stage=fragment envprefiltergen.fsh -o spirv/envprefiltergen.fsh.spv
glslc.exe -O -fshader-stage=fragment envbrdfgen.fsh -o spirv/envbrdfgen.fsh.spv

pause