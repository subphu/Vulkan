//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "../renderer/descriptor.h"
#include "../renderer/swapchain.h"
#include "../renderer/pipeline_graphic.h"
#include "../resources/shader.h"
#include "../resources/buffer.h"
#include "../mesh/mesh.h"


class GraphicEquirectangular {
    
public:
    const std::string VERT_SHADER_PATH = "shaders/equirectangular.vert.spv";
    const std::string FRAG_SHADER_PATH = "shaders/equirectangular.frag.spv";
    const std::string EQUIREC_PATH[1] = {
        "textures/Arches_E_PineTree/Arches_E_PineTree.hdr"
    };
    
    GraphicEquirectangular();
    ~GraphicEquirectangular();
    
    void cleanup();
    
    void createAssets();
    
    void setup();
    
    void draw();
    
    void recordCommand();
    
private:
    
    Cleaner m_cleaner;

    std::string m_equirecPath;
    
    Descriptor*      m_pDescriptor = nullptr;
    PipelineGraphic* m_pPipeline   = nullptr;
    
    Mesh*  m_pCube;
    Image* m_pHDR;
    Image* m_pRenderTarget;
    
    uint m_size;
    
    std::vector<Shader*> m_pShaders;
    
    VkCommandBuffer m_cmdBuffer;
    VkRenderPass m_renderPass;
    VkFramebuffer m_framebuffer;
    
    void createHDR();
    void createCube();
    
    void createCmdBuffer();
    
    void createRenderPass();
    void createFramebuffer();
    void generateRenderPassBeginInfo();
    
    void createDescriptor();
    void createPipeline();
    
};
