//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "../renderer/renderer.h"

class Mesh {
    
public:
    Mesh();
    ~Mesh();
    
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_texCoords;
    std::vector<int>   m_indices;
    
    VkBuffer vertexBuffer = {};
    VkBuffer indexBuffer = {};
    VkDeviceMemory vertexBufferMemory = {};
    VkDeviceMemory indexBufferMemory = {};
    
    void cleanup();
    void createPlane();
    void createQuad();
    void createCube();
    void createSphere(int wedge = 10, int segment = 20);
    void loadModel(const char* filename);
    
    void setRenderer(Renderer* renderer);
    void createVertexBuffer();
    void createIndexBuffer();
    
    void scale(glm::vec3 size);
    void rotate(float angle, glm::vec3 axis);
    void translate(glm::vec3 translation);
    
    glm::mat4 getMatrix();
    
    uint32_t sizeofPositions();
    uint32_t sizeofNormals();
    uint32_t sizeofTexCoords();
    uint32_t sizeofIndices();
    
    VkPipelineVertexInputStateCreateInfo* createVertexInputInfo();
    
private:
    Renderer* m_renderer;
    glm::mat4 m_model = glm::mat4(1.0f);
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    VkPipelineVertexInputStateCreateInfo stateCreateInfo{};
    VkVertexInputBindingDescription bindingDescription{};
    
    const uint32_t sizeofPosition = sizeof(glm::vec3);
    const uint32_t sizeofNormal   = sizeof(glm::vec3);
    const uint32_t sizeofTexCoord = sizeof(glm::vec2);
    const uint32_t sizeofIndex    = sizeof(int);
    
};
