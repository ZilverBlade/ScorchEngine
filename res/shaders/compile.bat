@echo off

glslc.exe -O -fshader-stage=vertex triangle.vsh -o spirv/triangle.vsh.spv  
glslc.exe -O -fshader-stage=fragment triangle.fsh -o spirv/triangle.fsh.spv

pause