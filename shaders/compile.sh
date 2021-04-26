cd /Users/subrotoph/Developer/Vulkan/Vulkan/shaders
/usr/local/bin/glslc shader.vert -o SPV/vert.spv
/usr/local/bin/glslc shader.frag -o SPV/frag.spv
/usr/local/bin/glslc shader.comp -o SPV/comp.spv

/usr/local/bin/glslc compute/interference1d.comp -o SPV/interference1d.comp.spv
/usr/local/bin/glslc compute/interference2d.comp -o SPV/interference2d.comp.spv

/usr/local/bin/glslc PBR/main1d.vert -o SPV/main1d.vert.spv
/usr/local/bin/glslc PBR/main1d.frag -o SPV/main1d.frag.spv
/usr/local/bin/glslc PBR/main2d.vert -o SPV/main2d.vert.spv
/usr/local/bin/glslc PBR/main2d.frag -o SPV/main2d.frag.spv
