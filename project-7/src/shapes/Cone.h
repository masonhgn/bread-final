#pragma once

#include <vector>
#include <glm/glm.hpp>

class Cone
{
public:
    void updateParams(int param1, int param2);
    std::vector<float> generateShape() { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void insertVec2(std::vector<float> &data, glm::vec2 v);


    void setVertexData();



    void makeCapTile(glm::vec3 center, glm::vec3 p1, glm::vec3 p2);
    void makeSlopeTile(glm::vec3 p1, glm::vec3 p2,glm::vec3 p3, glm::vec3 p4);
    void makeCapSlice(float theta1, float theta2);
    void makeSlopeSlice(float theta1, float theta2);
    void makeWedge(float theta1, float theta2);

    std::vector<float> m_vertexData;
    int m_param1;
    int m_param2;
    float m_radius = 0.5;
};
