//  Copyright © 2020 Subph. All rights reserved.
//

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <unordered_map>

#include "../libraries/tiny_obj_loader/tiny_obj_loader.h"

#include "mesh.h"

#include "../system.h"

Mesh::Mesh() {}
Mesh::~Mesh() {}

void Mesh::cleanup() {
    m_indexBuffer->cleanup();
    m_vertexBuffer->cleanup();
}

void Mesh::createPlane() {
    m_positions = {{-1., 0., 1.}, {1., 0., 1.}, {1., 0., -1.}, {-1., 0., -1.}};
    m_normals   = {{ 0., 1., 0.}, {0., 1., 0.}, {0., 1.,  0.}, { 0., 1.,  0.}};
    m_texCoords = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
    m_indices   = { 0, 1, 2, 2, 3, 0 };
}

void Mesh::createQuad() {
    m_positions = {{.5, .5, 0.}, {-.5, .5, 0.}, {-.5,-.5, 0.}, {.5,-.5, 0.}};
    m_normals   = {{ 0., 0., 1.}, {0., 0., 1.}, {0., 0., 1.}, { 0., 0., 1.}};
    m_texCoords = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
    m_indices   = { 0, 1, 2, 2, 3, 0 };
}

void Mesh::createCube() {
    glm::vec3 cubeVertices[8] = {
        {-.5,  .5,  .5}, {-.5, -.5,  .5}, { .5,  .5,  .5}, { .5, -.5,  .5},
        { .5,  .5, -.5}, { .5, -.5, -.5}, {-.5,  .5, -.5}, {-.5, -.5, -.5}
    };
    unsigned int cubeIndices[] = {
        6, 7, 0, 0, 7, 1,   2, 3, 4, 4, 3, 5,
        1, 7, 3, 3, 7, 5,   6, 0, 4, 4, 0, 2,
        4, 5, 6, 6, 5, 7,   0, 1, 2, 2, 1, 3
    };

    for (int i = 0; i < 36; i++) {
        glm::vec3 vertex = cubeVertices[cubeIndices[i]];

        glm::vec3 normal = { .0, .0, .0 };
        int side = i / 6;
        int axis = side / 2;
        normal[axis] = side % 2 * 2 - 1;

        glm::vec2 texture = { .0, .0 };
        if (axis == 0) texture.x = vertex.y > 0;
        else           texture.x = vertex.x > 0;
        if (axis == 2) texture.y = vertex.y > 0;
        else           texture.y = vertex.z < 0;

        m_positions.emplace_back(vertex);
        m_normals  .emplace_back(normal);
        m_texCoords.emplace_back(texture);
    }
    
    m_indices = {
        0 ,1 ,2 ,3 ,4 ,5 ,   6 ,7 ,8 ,9 ,10,11,
        12,13,14,15,16,17,   18,19,20,21,22,23,
        24,25,26,27,28,29,   30,31,32,33,34,35
    };
}

void Mesh::createSphere(int wedge, int segment) {
    float x, y, z, xz;
    float s, t;

    float segmentStep = -2 * PI / segment;  // counter-clockwise
    float wedgeStep = PI / wedge;
    float segmentAngle, wedgeAngle;

    for(int i = 0; i <= wedge; i++) {
        wedgeAngle = i * wedgeStep;             // starting from 0 to pi
        y  = cosf(wedgeAngle);                  // r * cos(u)
        xz = sinf(wedgeAngle);                  // r * sin(u)

        for(int j = 0; j <= segment; j++) {
            segmentAngle = j * segmentStep;     // starting from 0 to 2pi
            x = xz * cosf(segmentAngle);        // r * sin(u) * cos(v)
            z = xz * sinf(segmentAngle);        // r * sin(u) * sin(v)
            
            m_positions.emplace_back(glm::vec3(x, y, z));
            m_normals  .emplace_back(glm::vec3(x, y, z));

            s = (float)j / segment;             // vertex tex coord (s, t)
            t = (float)i / wedge;               // range between [0, 1]
            m_texCoords.emplace_back(glm::vec2(s, t));
        }
    }
    
    int w1, w2;
    int segmentVertices = segment + 1;
    for(int i = 0; i < wedge; i++) {
        w1 = i  * segmentVertices;
        w2 = w1 + segmentVertices;

        for(uint j = 0; j < segment; j++) {
            uint d = j + 1;
            m_indices.insert(m_indices.end(), { w1+j, w2+j, w1+d });
            m_indices.insert(m_indices.end(), { w1+d, w2+j, w2+d });
        }
    }
}

void Mesh::loadModel(const char* filename) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::unordered_map<size_t, uint32_t> uniqueVertices;
    std::string warn, err;
    
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename)) {
        throw std::runtime_error(warn + err);
    }
    
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            glm::vec3 position = glm::vec3(
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            );
            
            glm::vec3 normal = glm::vec3(
                 attrib.normals[3 * index.normal_index + 0],
                 attrib.normals[3 * index.normal_index + 1],
                 attrib.normals[3 * index.normal_index + 2]
            );
            
            glm::vec2 texCoord = glm::vec2(
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            );

            size_t hash = std::hash<glm::vec3>()(position) ^
                         (std::hash<glm::vec2>()(texCoord) << 1);
            if (uniqueVertices.count(hash) == 0) {
                uniqueVertices[hash] = UINT32(m_positions.size());
                m_positions.emplace_back(glm::vec3(position.x, position.y, position.z));
                m_normals  .emplace_back(glm::vec3(normal.x, normal.y, normal.z));
                m_texCoords.emplace_back(glm::vec2(texCoord.x, texCoord.y));
            }

            m_indices.push_back(uniqueVertices[hash]);
        }
    }
}

void Mesh::cmdCreateVertexBuffer() {
    VkDeviceSize bufferSize = sizeofPositions() + sizeofNormals() + sizeofTexCoords();
    
    Buffer* tempBuffer = new Buffer();
    tempBuffer->setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    tempBuffer->create();
    
    uint32_t shift = 0;
    for (int i = 0; i < m_positions.size(); i++) {
        tempBuffer->fillBuffer(&m_positions[i], sizeofPosition, shift);
        shift += sizeofPosition;
        tempBuffer->fillBuffer(&m_normals  [i], sizeofNormal  , shift);
        shift += sizeofNormal;
        tempBuffer->fillBuffer(&m_texCoords[i], sizeofTexCoord, shift);
        shift += sizeofTexCoord;
    }
    
    Buffer* vertexBuffer = new Buffer();
    vertexBuffer->setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vertexBuffer->create();
    vertexBuffer->cmdCopyFromBuffer(tempBuffer->m_buffer, bufferSize);
    
    
    tempBuffer->cleanup();
    
    { m_vertexBuffer = vertexBuffer; }
}

void Mesh::cmdCreateIndexBuffer() {
    VkDeviceSize bufferSize = sizeofIndices();
    
    Buffer* tempBuffer = new Buffer();
    tempBuffer->setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    tempBuffer->create();
    tempBuffer->fillBufferFull(m_indices.data());
    
    Buffer* indexBuffer = new Buffer();
    indexBuffer->setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    indexBuffer->create();
    indexBuffer->cmdCopyFromBuffer(tempBuffer->m_buffer, bufferSize);
    
    tempBuffer->cleanup();
    
    { m_indexBuffer = indexBuffer; }
}

VkPipelineVertexInputStateCreateInfo* Mesh::createVertexInputInfo() {
    uint32_t stride = sizeofPosition + sizeofNormal + sizeofTexCoord;
    
    bindingDescription.binding = 0;
    bindingDescription.stride = stride;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    attributeDescriptions.resize(3);
    
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = 0;
    
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = sizeofPosition;
    
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = sizeofPosition + sizeofNormal;
    
    stateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    stateCreateInfo.vertexBindingDescriptionCount = 1;
    stateCreateInfo.vertexAttributeDescriptionCount = UINT32(attributeDescriptions.size());
    stateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    stateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    
    return &stateCreateInfo;
}

void Mesh::scale(glm::vec3 size)               { m_model = glm::scale(m_model, size); }
void Mesh::rotate(float angle, glm::vec3 axis) { m_model = glm::rotate(m_model, glm::radians(angle), axis); }
void Mesh::translate(glm::vec3 translation)    { m_model = glm::translate(m_model, translation); }
glm::mat4 Mesh::getMatrix() { return m_model; }

uint32_t Mesh::sizeofPositions() { return sizeofPosition * (uint32_t) m_positions.size(); }
uint32_t Mesh::sizeofNormals  () { return sizeofNormal   * (uint32_t) m_normals.size(); }
uint32_t Mesh::sizeofTexCoords() { return sizeofTexCoord * (uint32_t) m_texCoords.size(); }
uint32_t Mesh::sizeofIndices  () { return sizeofIndex    * (uint32_t) m_indices.size(); }
