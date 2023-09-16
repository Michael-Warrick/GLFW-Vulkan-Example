# if unable to run, use `chmod +x compile.sh`
$VULKAN_SDK/bin/glslc -fshader-stage=vertex vert.glsl -o compiled/vert.spv
$VULKAN_SDK/bin/glslc -fshader-stage=fragment frag.glsl -o compiled/frag.spv

# TODO: Make a compile.bat equivalent