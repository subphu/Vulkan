cd /Users/subrotoph/Developer/Vulkan/Vulkan/shaders
/usr/local/bin/glslc shader.vert -o SPV/vert.spv
/usr/local/bin/glslc shader.frag -o SPV/frag.spv
/usr/local/bin/glslc shader.comp -o SPV/comp.spv

/usr/local/bin/glslc compute/interference.comp -o SPV/interference.comp.spv

/usr/local/bin/glslc PBR/main.vert -o SPV/main.vert.spv
/usr/local/bin/glslc PBR/main.frag -o SPV/main.frag.spv
