#ifndef SHAPE_H
#define SHAPE_H

#include <vector>
#include <glm/glm.hpp>

// Base Shape class
class Shape {
public:
    virtual ~Shape() = default;

    virtual void updateParams(int param1, int param2 = 0) = 0;
    virtual void setVertexData() = 0;
    std::vector<float> m_vertexData;

protected:
    int m_param1, m_param2;

    void insertVec3(std::vector<float> &data, glm::vec3 v) {
        data.push_back(v.x);
        data.push_back(v.y);
        data.push_back(v.z);
    }
};

// Sphere subclass
class Sphere : public Shape {
public:
    void updateParams(int param1, int param2) override {
        m_vertexData = std::vector<float>();
        m_param1 = param1;
        m_param2 = param2;
        setVertexData();
    }

    void setVertexData() override {
        makeSphere();
    }

private:
    void makeTile(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight) {
        // Implementation for creating a tile on the sphere
        glm::vec3 vertices[] = { topLeft, topRight, bottomRight, bottomLeft };

        // radius = 0.5
        for (int i = 0; i < 4; i++) {
        vertices[i] = glm::normalize(vertices[i]) * 0.5f;
        }

        // calculate normals for each vertex
        glm::vec3 normals[4];
        for (int i = 0; i < 4; i++) {
        // normalize the vertex position to get the normal
        normals[i] = glm::normalize(vertices[i]);
        }

        // first triangle
        // top left
        insertVec3(m_vertexData, vertices[0]);
        insertVec3(m_vertexData, normals[0]);
        // bottom left
        insertVec3(m_vertexData, vertices[3]);
        insertVec3(m_vertexData, normals[3]);
        // top right
        insertVec3(m_vertexData, vertices[1]);
        insertVec3(m_vertexData, normals[1]);
        // second triangle
        // bottom left
        insertVec3(m_vertexData, vertices[3]);
        insertVec3(m_vertexData, normals[3]);
        // bottom right
        insertVec3(m_vertexData, vertices[2]);
        insertVec3(m_vertexData, normals[2]);
        // top right
        insertVec3(m_vertexData, vertices[1]);
        insertVec3(m_vertexData, normals[1]);
    }

    void makeWedge(float currentTheta, float nextTheta) {
        float phiIncrement = M_PI / m_param1;
        //    std::cout << phiIncrement << std::endl;

        for (int i = 0; i < m_param1; i++) {
        // Calculate phi values for the current and next latitude divisions
            float currentPhi = i * phiIncrement;
            float nextPhi = (i + 1) * phiIncrement;

            // Calculate vertex coordinates using spherical to cartesian conversion for the four corners of the tile
            glm::vec3 topLeft = glm::vec3(
                0.5 * glm::sin(currentPhi) * glm::sin(currentTheta),
                0.5 * glm::cos(currentPhi),
                0.5 * glm::sin(currentPhi) * glm::cos(currentTheta)
                );

            glm::vec3 topRight = glm::vec3(
                0.5 * glm::sin(currentPhi) * glm::sin(nextTheta),
                0.5 * glm::cos(currentPhi),
                0.5 * glm::sin(currentPhi) * glm::cos(nextTheta)
                );

            glm::vec3 bottomLeft = glm::vec3(
                0.5 * glm::sin(nextPhi) * glm::sin(currentTheta),
                0.5 * glm::cos(nextPhi),
                0.5 * glm::sin(nextPhi) * glm::cos(currentTheta)
                );

            glm::vec3 bottomRight = glm::vec3(
                0.5 * glm::sin(nextPhi) * glm::sin(nextTheta),
                0.5 * glm::cos(nextPhi),
                0.5 * glm::sin(nextPhi) * glm::cos(nextTheta)
                );

            // Use the makeTile() function to create a tile using these vertices
            makeTile(topLeft, topRight, bottomLeft, bottomRight);
        }

    }

    void makeSphere() {
        // Implementation for creating the entire sphere
        float thetaIncrement = 2 * M_PI / m_param2; // Incremental value of θ to create wedges

        for (int j = 0; j < m_param2; j++) {
            float currentTheta = j * thetaIncrement;
            float nextTheta = (j + 1) * thetaIncrement;

            // Call makeWedge() for the current θ segment
            makeWedge(currentTheta, nextTheta);
        }
    }
};

// Cube subclass
class Cube : public Shape {
public:
    void updateParams(int param1, int param2 = 0) override {
        // Cube-specific implementation
        m_vertexData = std::vector<float>();
        m_param1 = param1;
        setVertexData();
    }

    void setVertexData() override {
        // Cube-specific implementation
        // Front face
        makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
                 glm::vec3( 0.5f,  0.5f, 0.5f),
                 glm::vec3(-0.5f, -0.5f, 0.5f),
                 glm::vec3( 0.5f, -0.5f, 0.5f));

        // Back face
        makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
                 glm::vec3(-0.5f,  0.5f, -0.5f),
                 glm::vec3( 0.5f, -0.5f, -0.5f),
                 glm::vec3(-0.5f, -0.5f, -0.5f));

        // Top face
        makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
                 glm::vec3( 0.5f,  0.5f, -0.5f),
                 glm::vec3(-0.5f,  0.5f,  0.5f),
                 glm::vec3( 0.5f,  0.5f,  0.5f));

        // Bottom face
        makeFace(glm::vec3(-0.5f, -0.5f,  0.5f),
                 glm::vec3( 0.5f, -0.5f,  0.5f),
                 glm::vec3(-0.5f, -0.5f, -0.5f),
                 glm::vec3( 0.5f, -0.5f, -0.5f));

        // Right face
        makeFace(glm::vec3( 0.5f,  0.5f,  0.5f),
                 glm::vec3( 0.5f,  0.5f, -0.5f),
                 glm::vec3( 0.5f, -0.5f,  0.5f),
                 glm::vec3( 0.5f, -0.5f, -0.5f));

        // Left face
        makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
                 glm::vec3(-0.5f,  0.5f,  0.5f),
                 glm::vec3(-0.5f, -0.5f, -0.5f),
                 glm::vec3(-0.5f, -0.5f,  0.5f));
    }

private:
    void makeTile(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight) {
        glm::vec3 normal = -glm::normalize(glm::cross(topRight - topLeft, bottomLeft - topLeft));

        // Triangle 1
        insertVec3(m_vertexData, topLeft);
        insertVec3(m_vertexData, normal);

        insertVec3(m_vertexData, bottomLeft);
        insertVec3(m_vertexData, normal);

        insertVec3(m_vertexData, topRight);
        insertVec3(m_vertexData, normal);

        // Triangle 2
        insertVec3(m_vertexData, bottomLeft);
        insertVec3(m_vertexData, normal);

        insertVec3(m_vertexData, bottomRight);
        insertVec3(m_vertexData, normal);

        insertVec3(m_vertexData, topRight);
        insertVec3(m_vertexData, normal);
    }

    void makeFace(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight) {
        glm::vec3 horizontalStep = (topRight - topLeft) / static_cast<float>(m_param1);

        // Calculate the vector representing one vertical step
        glm::vec3 verticalStep = (bottomLeft - topLeft) / static_cast<float>(m_param1);

        // Iterate over each row and column to create the tiles
        for (int i = 0; i < m_param1; i++) {
            for (int j = 0; j < m_param1; j++) {
                // Calculate the vertices for the current tile
                glm::vec3 currentTopLeft = topLeft + horizontalStep * static_cast<float>(j) + verticalStep * static_cast<float>(i);
                glm::vec3 currentTopRight = currentTopLeft + horizontalStep;
                glm::vec3 currentBottomLeft = currentTopLeft + verticalStep;
                glm::vec3 currentBottomRight = currentBottomLeft + horizontalStep;

                // Make the tile with the calculated vertices
                makeTile(currentTopLeft, currentTopRight, currentBottomLeft, currentBottomRight);
            }
        }
    }
};

// Cone subclass
class Cone : public Shape {
public:
    void adjustVertexPositions() {
        for (size_t i = 1; i < m_vertexData.size(); i += 6) {
            m_vertexData[i] -= 0.5f; // Adjust the y-coordinate
        }
    }

    void updateParams(int param1, int param2) override {
        m_vertexData = std::vector<float>();
        m_param1 = param1;
        m_param2 = param2;
//        float temp = m_param1;
//        m_param1 = m_param2;
//        m_param2 = temp;
        setVertexData();
        adjustVertexPositions();
    }

    void setVertexData() override {
        float radius = 0.5f;
        float height = 1.0f;
        float thetaStep = 2 * M_PI / m_param1;
        float heightStep = height / m_param2;
        glm::vec3 coneTip = glm::vec3(0.0f, height, 0.0f);

        // Create the body of the cone
        for (int i = 0; i < m_param1; i++) {
            float currentTheta = i * thetaStep;
            float nextTheta = (i + 1) * thetaStep;

            for (int j = 0; j < m_param2; j++) {
                float currentHeight = j * heightStep;
                float nextHeight = (j + 1) * heightStep;

                float lowerRadius = radius * (1 - static_cast<float>(j) / m_param2);
                float upperRadius = radius * (1 - static_cast<float>(j + 1) / m_param2);

                // compute vertices for the current slice
                glm::vec3 bottomLeft = glm::vec3(lowerRadius * cos(currentTheta), currentHeight, lowerRadius * sin(currentTheta));
                glm::vec3 bottomRight = glm::vec3(lowerRadius * cos(nextTheta), currentHeight, lowerRadius * sin(nextTheta));
                glm::vec3 topLeft = glm::vec3(upperRadius * cos(currentTheta), nextHeight, upperRadius * sin(currentTheta));
                glm::vec3 topRight = glm::vec3(upperRadius * cos(nextTheta), nextHeight, upperRadius * sin(nextTheta));

//                if (j == m_param2 - 1) {
//                    //                // This is the last ring, which connects to the tip
//                    glm::vec3 normal = glm::normalize(glm::vec3(bottomLeft.x, 0.0f, bottomLeft.z));

//                    // Triangle that connects to the tip
//                    insertVec3(m_vertexData, bottomLeft);
//                    insertVec3(m_vertexData, normal);
//                    insertVec3(m_vertexData, coneTip);
//                    insertVec3(m_vertexData, normal);
//                    insertVec3(m_vertexData, bottomRight);
//                    insertVec3(m_vertexData, normal);
//                } else {
//                    // Non-tip tiles
//                    makeTile(bottomLeft, bottomRight, topLeft, topRight);
//                }
                makeTile(bottomLeft, bottomRight, topLeft, topRight);

            }
        }

        // Create the base of the cone
        glm::vec3 centerBottom = glm::vec3(0.0f, 0.0f, 0.0f); // Center of the cone's base
        for (int i = 0; i < m_param1; i++) {
            float currentTheta = i * thetaStep;
            float nextTheta = (i + 1) * thetaStep;

            glm::vec3 point1 = glm::vec3(radius * cos(currentTheta), 0.0f, radius * sin(currentTheta));
            glm::vec3 point2 = glm::vec3(radius * cos(nextTheta), 0.0f, radius * sin(nextTheta));

            // For the base, normals will be pointing downwards
            glm::vec3 normal = glm::vec3(0.0f, -1.0f, 0.0f);

            // Add the triangles for the base
            insertVec3(m_vertexData, centerBottom);
            insertVec3(m_vertexData, normal);

            insertVec3(m_vertexData, point1);
            insertVec3(m_vertexData, normal);

            insertVec3(m_vertexData, point2);
            insertVec3(m_vertexData, normal);
        }
    }

private:
    void makeTile(glm::vec3 topLeft,
                        glm::vec3 topRight,
                        glm::vec3 bottomLeft,
                        glm::vec3 bottomRight) {

        glm::vec3 normalBottomLeft = glm::normalize(bottomLeft - glm::vec3(0, bottomLeft.y, 0));
        glm::vec3 normalBottomRight = glm::normalize(bottomRight - glm::vec3(0, bottomRight.y, 0));
        glm::vec3 normalTopLeft = glm::normalize(topLeft - glm::vec3(0, topLeft.y, 0));
        glm::vec3 normalTopRight = glm::normalize(topRight - glm::vec3(0, topRight.y, 0));

        insertVec3(m_vertexData, topLeft);
        insertVec3(m_vertexData, normalTopLeft);
        insertVec3(m_vertexData, bottomLeft);
        insertVec3(m_vertexData, normalBottomLeft);
        insertVec3(m_vertexData, bottomRight);
        insertVec3(m_vertexData, normalBottomRight);

        // Second triangle
        insertVec3(m_vertexData, topLeft);
        insertVec3(m_vertexData, normalTopLeft);
        insertVec3(m_vertexData, bottomRight);
        insertVec3(m_vertexData, normalBottomRight);
        insertVec3(m_vertexData, topRight);
        insertVec3(m_vertexData, normalTopRight);
    }


};

// Cylinder subclass
class Cylinder : public Shape {
public:
    void updateParams(int param1, int param2) override {
        m_vertexData = std::vector<float>();
        m_param1 = param1;
        m_param2 = param2;
        setVertexData();
    }

    void setVertexData() override {
        float heightIncrement = 1.0f / m_param2;
        float radius = 0.5f;
        float angleIncrement = 2 * M_PI / m_param1;

        // Create the cylinder sides
        for (int i = 0; i < m_param1; ++i) {
            float currentAngle = i * angleIncrement;
            float nextAngle = (i + 1) * angleIncrement;
            for (int j = 0; j < m_param2; ++j) {
                float currentHeight = -0.5f + j * heightIncrement;
                float nextHeight = currentHeight + heightIncrement;

                glm::vec3 bottomLeft(radius * cos(currentAngle), currentHeight, radius * sin(currentAngle));
                glm::vec3 bottomRight(radius * cos(nextAngle), currentHeight, radius * sin(nextAngle));
                glm::vec3 topLeft(radius * cos(currentAngle), nextHeight, radius * sin(currentAngle));
                glm::vec3 topRight(radius * cos(nextAngle), nextHeight, radius * sin(nextAngle));

                makeSideTile(bottomLeft, bottomRight, topLeft, topRight);
            }
        }

        // Create the top cap
        makeCap(true);

        // Create the bottom cap
        makeCap(false);
    }

private:
    void makeSideTile(glm::vec3 bottomLeft, glm::vec3 bottomRight, glm::vec3 topLeft, glm::vec3 topRight) {
        // Calculate normals for each triangle
        glm::vec3 normalBottomLeft = glm::normalize(bottomLeft - glm::vec3(0, bottomLeft.y, 0));
        glm::vec3 normalBottomRight = glm::normalize(bottomRight - glm::vec3(0, bottomRight.y, 0));
        glm::vec3 normalTopLeft = glm::normalize(topLeft - glm::vec3(0, topLeft.y, 0));
        glm::vec3 normalTopRight = glm::normalize(topRight - glm::vec3(0, topRight.y, 0));

        // First triangle
        insertVec3(m_vertexData, bottomLeft);
        insertVec3(m_vertexData, normalBottomLeft);
        insertVec3(m_vertexData, topLeft);
        insertVec3(m_vertexData, normalTopLeft);
        insertVec3(m_vertexData, bottomRight);
        insertVec3(m_vertexData, normalBottomRight);

        // Second triangle
        insertVec3(m_vertexData, topLeft);
        insertVec3(m_vertexData, normalTopLeft);
        insertVec3(m_vertexData, topRight);
        insertVec3(m_vertexData, normalTopRight);
        insertVec3(m_vertexData, bottomRight);
        insertVec3(m_vertexData, normalBottomRight);
    }

    void makeCap(bool top) {
        float radius = 0.5f;
        float y = top ? 0.5f : -0.5f;
        glm::vec3 center(0, y, 0);
        glm::vec3 normal(0, top ? 1.0f : -1.0f, 0);

        float angleIncrement = 2 * M_PI / m_param1;
        for (int i = 0; i < m_param1; ++i) {
            float theta = i * angleIncrement;
            float nextTheta = (i + 1) * angleIncrement;

            glm::vec3 point1(radius * cos(theta), y, radius * sin(theta));
            glm::vec3 point2(radius * cos(nextTheta), y, radius * sin(nextTheta));

            // top bottom should have different order
            if (!top) {
                insertVec3(m_vertexData, center);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, point1);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, point2);
                insertVec3(m_vertexData, normal);
            }
            else {
                insertVec3(m_vertexData, point2);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, point1);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, center);
                insertVec3(m_vertexData, normal);

            }
        }
    }
};


#endif // SHAPE_H
