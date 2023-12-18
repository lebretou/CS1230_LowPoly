#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>
#include "./utils/sceneparser.h"

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);

    RenderData curRenderData;
    glm::mat4 curView;
    glm::mat4 curProj;

    GLuint m_shader;
    GLuint m_texture_shader;

    std::vector<float> vertex_data;


    std::vector<float> d_cube;
    std::vector<float> d_sphere;
    std::vector<float> d_cone;
    std::vector<float> d_cyl;

    GLuint vbo, vao;
    GLuint vbo_cube, vbo_sphere, vbo_cyl, vbo_cone;
    GLuint vao_cube, vao_sphere, vao_cyl, vao_cone;
    GLuint m_fbo_texture;
    GLuint m_fullscreen_vao, m_fullscreen_vbo;
    GLuint m_fbo_renderbuffer, m_fbo;

    // -.-.-.-.-.-.-.-.-.-.-.-.-.-
    // TA look here!
    GLuint m_defaultFBO = 2;
    // -.-.-.-.-.-.-.-.-.-.-.-.-.-

    int m_screen_width = reinterpret_cast<int>(size().width());
    int m_screen_height = reinterpret_cast<int>(size().height());
    int m_fbo_width = m_screen_width;
    int m_fbo_height = m_screen_height;

    float oldNear, oldFar;

    void updateProjection(float near, float far, float heightAngle, float widthAngle) {
        float c = - near / far;

        glm::mat4 scaleTrans = glm::mat4(
            glm::vec4(1, 0, 0, 0), // First column
            glm::vec4(0, 1, 0, 0), // Second column
            glm::vec4(0, 0, -2, 0), // Third column
            glm::vec4(0, 0, -1, 1)  // Fourth column
        );

        glm::mat4 unhinge = glm::mat4(
            glm::vec4(1, 0, 0, 0), // First column
            glm::vec4(0, 1, 0, 0), // Second column
            glm::vec4(0, 0, (1.0f / (1.0f + c)), -1), // Third column
            glm::vec4(0, 0, (-c / (1.0f + c)), 0)  // Fourth column
        );

        glm::mat4 pers = glm::mat4(
            glm::vec4((1.0f/(far * tan(widthAngle/2.0))), 0, 0, 0), // First column
            glm::vec4(0, (1.0f/(far * tan(heightAngle/2.0))), 0, 0), // Second column
            glm::vec4(0, 0, (1.0/far), 0), // Third column
            glm::vec4(0, 0, 0, 1)  // Fourth column
        );

        curProj = scaleTrans * unhinge * pers;
    }

    void updateVAOVBO();

    std::vector<float> combineVectors(const std::vector<float>& vec1,
                                      const std::vector<float>& vec2,
                                      const std::vector<float>& vec3,
                                      const std::vector<float>& vec4) {
        // Create a new vector to hold the combined elements
        std::vector<float> combined;

        // Reserve enough space to avoid multiple reallocations
        combined.reserve(vec1.size() + vec2.size() + vec3.size() + vec4.size());

        // Append each vector in order
        combined.insert(combined.end(), vec1.begin(), vec1.end());
        combined.insert(combined.end(), vec2.begin(), vec2.end());
        combined.insert(combined.end(), vec3.begin(), vec3.end());
        combined.insert(combined.end(), vec4.begin(), vec4.end());

        return combined;
    }

    void setVertexAttributes() {
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    }

    void setupShapeVAOVBO(GLuint& vao, GLuint& vbo, const std::vector<float>& vertexData) {
        if (!vertexData.empty()) {
            // Generate and set up the VAO
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            // Generate and bind the VBO
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

            // Set vertex attributes
            setVertexAttributes();

            // Unbind the VBO and VAO
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
    }

    void updateCamera(float near, float far) {
        float heightAngle = curRenderData.cameraData.heightAngle;
        float aRatio = static_cast<float>(size().width()) / size().height();
        float widthAngle = heightAngle * aRatio;


        updateProjection(near, far, heightAngle, widthAngle);
    }

    glm::mat4 createRotationMatrix(glm::vec3 u, float theta) {
        // Normalize the rotation axis vector
        u = glm::normalize(u);

        // Pre-compute the sine and cosine (angles must be in radians)
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);

        // Compute the terms of the rotation matrix
        glm::mat4 rotationMatrix;
        rotationMatrix[0][0] = cosTheta + u.x * u.x * (1 - cosTheta);
        rotationMatrix[0][1] = u.x * u.y * (1 - cosTheta) - u.z * sinTheta;
        rotationMatrix[0][2] = u.x * u.z * (1 - cosTheta) + u.y * sinTheta;
        rotationMatrix[0][3] = 0.0f;

        rotationMatrix[1][0] = u.y * u.x * (1 - cosTheta) + u.z * sinTheta;
        rotationMatrix[1][1] = cosTheta + u.y * u.y * (1 - cosTheta);
        rotationMatrix[1][2] = u.y * u.z * (1 - cosTheta) - u.x * sinTheta;
        rotationMatrix[1][3] = 0.0f;

        rotationMatrix[2][0] = u.z * u.x * (1 - cosTheta) - u.y * sinTheta;
        rotationMatrix[2][1] = u.z * u.y * (1 - cosTheta) + u.x * sinTheta;
        rotationMatrix[2][2] = cosTheta + u.z * u.z * (1 - cosTheta);
        rotationMatrix[2][3] = 0.0f;

        rotationMatrix[3][0] = 0.0f;
        rotationMatrix[3][1] = 0.0f;
        rotationMatrix[3][2] = 0.0f;
        rotationMatrix[3][3] = 1.0f;

        return rotationMatrix;
    }

    void makeFBO();

    void paintTexture(GLuint texture, bool invert, bool blur);

    void drawShapes();
public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio = this->devicePixelRatio();


};
