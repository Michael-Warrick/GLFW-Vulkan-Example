# if unable to run, use `chmod +x compile.sh`
glslc -fshader-stage=vertex vert.glsl -o compiled/vert.spv
glslc -fshader-stage=fragment frag.glsl -o compiled/frag.spv

# TODO: Make a compile.bat equivalent