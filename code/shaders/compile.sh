cd /Users/subrotoph/Developer/Vulkan/Vulkan/code/shaders
/usr/local/bin/glslc shader.vert -o ../../shaders/vert.spv
/usr/local/bin/glslc shader.frag -o ../../shaders/frag.spv
/usr/local/bin/glslc shader.comp -o ../../shaders/comp.spv
/usr/local/bin/glslc skybox.vert -o ../../shaders/skybox.vert.spv
/usr/local/bin/glslc skybox.frag -o ../../shaders/skybox.frag.spv

/usr/local/bin/glslc compute/interference1d.comp -o ../../shaders/interference1d.comp.spv
/usr/local/bin/glslc compute/interference2d.comp -o ../../shaders/interference2d.comp.spv

/usr/local/bin/glslc PBR/main1d.vert -o ../../shaders/main1d.vert.spv
/usr/local/bin/glslc PBR/main1d.frag -o ../../shaders/main1d.frag.spv
/usr/local/bin/glslc PBR/main2d.vert -o ../../shaders/main2d.vert.spv
/usr/local/bin/glslc PBR/main2d.frag -o ../../shaders/main2d.frag.spv
/usr/local/bin/glslc PBR/manual.vert -o ../../shaders/manual.vert.spv
/usr/local/bin/glslc PBR/manual.frag -o ../../shaders/manual.frag.spv
