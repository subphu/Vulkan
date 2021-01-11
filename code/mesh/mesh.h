//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <math.h>

#include "../renderer/renderer.h"
#include "../libraries/tiny_obj_loader/tiny_obj_loader.h"

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
    
    unsigned long sizeofPositions();
    unsigned long sizeofNormals();
    unsigned long sizeofTexCoords();
    unsigned long sizeofIndices();
    
    VkPipelineVertexInputStateCreateInfo* createVertexInputInfo();
    
private:
    Renderer* m_renderer;
    glm::mat4 m_model = glm::mat4(1.0f);
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    VkPipelineVertexInputStateCreateInfo stateCreateInfo{};
    VkVertexInputBindingDescription bindingDescription{};
    
    const unsigned long sizeofPosition = sizeof(glm::vec3);
    const unsigned long sizeofNormal   = sizeof(glm::vec3);
    const unsigned long sizeofTexCoord = sizeof(glm::vec2);
    const unsigned long sizeofIndex    = sizeof(int);
    
};
