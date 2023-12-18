#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "./shape.cpp"
#include "./utils/shaderloader.h"

bool firstRun = true;

void printMat4(const glm::mat4& matrix) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << matrix[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}

void printVec4(const glm::vec4& vector) {
    std::cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")" << std::endl;
}

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    // Delete all vao & vbo
    glDeleteBuffers(1, &m_fullscreen_vbo);
    glDeleteVertexArrays(1, &m_fullscreen_vao);

    glDeleteBuffers(1, &vbo_cube);
    glDeleteVertexArrays(1, &vao_cube);

    glDeleteBuffers(1, &vbo_sphere);
    glDeleteVertexArrays(1, &vao_sphere);

    glDeleteBuffers(1, &vbo_cyl);
    glDeleteVertexArrays(1, &vao_cyl);

    glDeleteBuffers(1, &vbo_cone);
    glDeleteVertexArrays(1, &vao_cone);

    // Delete FBO, RBO and associated textures
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    glClearColor(0.8, 0.8, 0.8, 0.0);

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    m_screen_width = reinterpret_cast<int>(size().width() * m_devicePixelRatio);
    m_screen_height = reinterpret_cast<int>(size().height() * m_devicePixelRatio);
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;


    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_shader = ShaderLoader::createShaderProgram("/Users/leoxu/Brown/CS1230/projects-realtime-lebretou/resources/shaders/default.vert",
                                                 "/Users/leoxu/Brown/CS1230/projects-realtime-lebretou/resources/shaders/default.frag");

    m_texture_shader = ShaderLoader::createShaderProgram("/Users/leoxu/Brown/CS1230/projects-realtime-lebretou/resources/shaders/texture.vert",
                                                 "/Users/leoxu/Brown/CS1230/projects-realtime-lebretou/resources/shaders/texture.frag");

    firstRun = false;

     glUseProgram(m_texture_shader);
     glUniform1i(glGetUniformLocation(m_texture_shader, "my_texture"), 0);
     glUseProgram(0);

    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS    //
            -1.0f,  1.0f, 0.0f,
            0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f, 0.0f,
            1.0f,  1.0f, 0.0f,
            1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, -1.0f, 0.0f,
            1.0f, 0.0f
        };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // modify the code below to add a second attribute to the vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    // add UV coordinates to the VAO
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    makeFBO();


}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    // Bind our FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Call glViewport
    glViewport(0, 0, m_fbo_width, m_fbo_height);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawShapes();

    // bind the default buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width, m_screen_height);

    // Task 26: Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    paintTexture(m_fbo_texture, settings.perPixelFilter, settings.kernelBasedFilter);

}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
}

void Realtime::sceneChanged() {
    // parse in the scene file
    SceneParser::parse(settings.sceneFilePath, curRenderData);

    // update and calculat the view matrix
    curRenderData.cameraData.updateView();

    // assign the current view matrix
    curView = curRenderData.cameraData.view;

    // update the camera data and proj matrix using the new settings
    updateCamera(settings.nearPlane, settings.farPlane);

    updateVAOVBO();

    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    // update the camera data and proj matrix using the new settings
    if ((abs(settings.nearPlane - oldNear) > 0.0001 ) || (abs(settings.farPlane - oldFar) > 0.0001)) {
        updateCamera(settings.nearPlane, settings.farPlane);
        oldNear = settings.nearPlane;
        oldFar = settings.farPlane;
    }
    else {
        // we won't update the vao or vbo on the first run since no data
        if (!firstRun) {
            updateVAOVBO();
        }
    }

    update(); // asks for a PaintGL() call to occur
}

void Realtime::updateVAOVBO() {
    vertex_data = std::vector<float>();
    d_cube = std::vector<float>();
    d_sphere = std::vector<float>();
    d_cyl = std::vector<float>();
    d_cone = std::vector<float>();
    for (auto& shape:curRenderData.shapes) {
        // generate shape vertives positions and normals according to the shape type
        switch (shape.primitive.type) {
            case PrimitiveType::PRIMITIVE_CUBE: {
                Cube cube;
                cube.updateParams(settings.shapeParameter1, settings.shapeParameter2);
                if (!d_cube.empty()) {
                    continue;
                }
                d_cube.insert(d_cube.end(), cube.m_vertexData.begin(), cube.m_vertexData.end());
                break;
            }
            case PrimitiveType::PRIMITIVE_CONE: {
                Cone cone;
                cone.updateParams(fmax(settings.shapeParameter1, 3), fmax(settings.shapeParameter2, 100));
                if (!d_cone.empty()) {
                    continue;
                }
                d_cone.insert(d_cone.end(), cone.m_vertexData.begin(),cone.m_vertexData.end());
                break;
            }
            case PrimitiveType::PRIMITIVE_SPHERE: {
                Sphere sphere;
                sphere.updateParams(fmax(2, settings.shapeParameter1), fmax(3, settings.shapeParameter2));
                if (!d_sphere.empty()) {
                    continue;
                }
                d_sphere.insert(d_sphere.end(), sphere.m_vertexData.begin(), sphere.m_vertexData.end());
                break;
            }
            case PrimitiveType::PRIMITIVE_CYLINDER: {
                Cylinder cyl;
                cyl.updateParams(fmax(3, settings.shapeParameter1), fmax(3, settings.shapeParameter2));
                if (!d_cyl.empty()) {
                    continue;
                }
                d_cyl.insert(d_cyl.end(), cyl.m_vertexData.begin(), cyl.m_vertexData.end());
                break;
            }

            default: {
                break;
            }
        }
    }


    setupShapeVAOVBO(vao_cube, vbo_cube, d_cube);
    setupShapeVAOVBO(vao_sphere, vbo_sphere, d_sphere);
    setupShapeVAOVBO(vao_cyl, vbo_cyl, d_cyl);
    setupShapeVAOVBO(vao_cone, vbo_cone, d_cone);

}


void Realtime::drawShapes() {
    glUseProgram(m_shader);

    int size = 0;
    int i = 0;

    for (auto& light : curRenderData.lights) {
        glm::vec3 direction = -glm::vec3(light.dir);
        glm::vec3 color = glm::vec3(light.color);
        glm::vec3 position = glm::vec3(light.pos);
        glm::vec3 attenuation = light.function;
        float light_angle = light.angle;
        float light_penu = light.penumbra;
        int light_type = static_cast<int>(light.type);


        // send light's direction
        GLint loc_dir = glGetUniformLocation(m_shader, ("light_directions[" + std::to_string(i) + "]").c_str());
        glUniform3f(loc_dir, direction.x, direction.y, direction.z);

        // sed light's color
        GLint loc_color = glGetUniformLocation(m_shader, ("light_colors[" + std::to_string(i) + "]").c_str());
        glUniform3f(loc_color, color.x, color.y, color.z);

        // send light's position
        GLint loc_pos = glGetUniformLocation(m_shader, ("light_positions[" + std::to_string(i) + "]").c_str());
        glUniform3f(loc_pos, position.x, position.y, position.z);

        // send light's attenuation
        GLint loc_att = glGetUniformLocation(m_shader, ("light_atts[" + std::to_string(i) + "]").c_str());
        glUniform3f(loc_att, attenuation.x, attenuation.y, attenuation.z);

        // send light's type
        GLint loc_type = glGetUniformLocation(m_shader, ("light_types[" + std::to_string(i) + "]").c_str());
        glUniform1i(loc_type, light_type);

        // send light's angle
        GLint loc_angle = glGetUniformLocation(m_shader, ("light_angles[" + std::to_string(i) + "]").c_str());
        glUniform1f(loc_angle, light_angle);

        // send light's type
        GLint loc_penu = glGetUniformLocation(m_shader, ("light_penus[" + std::to_string(i) + "]").c_str());
        glUniform1f(loc_penu, light_penu);

        i++;
    }

    // send the number of total lights to the shader
    glUniform1i(glGetUniformLocation(m_shader, "num_lights"), i);


    // send the position of camera to the shader
    glUniform3f(glGetUniformLocation(m_shader, "camera_pos"), curRenderData.cameraData.pos[0],
                curRenderData.cameraData.pos[1],
                curRenderData.cameraData.pos[2]);


    // for each shape, bind the corresponding vao
    for (auto& shape : curRenderData.shapes) {
        switch (shape.primitive.type) {
        case PrimitiveType::PRIMITIVE_CUBE:
                glBindVertexArray(vao_cube);
                size = d_cube.size();
                break;
        case PrimitiveType::PRIMITIVE_CONE:
                glBindVertexArray(vao_cone);
                size = d_cone.size();
                break;
        case PrimitiveType::PRIMITIVE_CYLINDER:
                glBindVertexArray(vao_cyl);
                size = d_cyl.size();
                break;
        case PrimitiveType::PRIMITIVE_SPHERE:
                glBindVertexArray(vao_sphere);
                size = d_sphere.size();
                break;
        default:
                break;
        }

        // send shapes' ctm as a uniform
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "model_matrix"), 1, GL_FALSE, &shape.ctm[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "model_matrix_inverse"), 1, GL_FALSE, &shape.inverse_ctm[0][0]);
        glUniformMatrix3fv(glGetUniformLocation(m_shader, "model_matrix_inv_trans"), 1, GL_FALSE, &shape.inverse_transpose_ctm3[0][0]);

        // send view and proj matrices
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "model_view"), 1, GL_FALSE, &curView[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "model_proj"), 1, GL_FALSE, &curProj[0][0]);

        // send scene light coefficients as uniforms
        glUniform1f(glGetUniformLocation(m_shader, "ka"), curRenderData.globalData.ka);
        glUniform1f(glGetUniformLocation(m_shader, "ks"), curRenderData.globalData.ks);
        glUniform1f(glGetUniformLocation(m_shader, "kd"), curRenderData.globalData.kd);

        // send the shape's material terms as uniforms
        glUniform4f(glGetUniformLocation(m_shader, "cAmbient"), shape.primitive.material.cAmbient[0],
                    shape.primitive.material.cAmbient[1],
                    shape.primitive.material.cAmbient[2],
                    shape.primitive.material.cAmbient[3]);


        glUniform4f(glGetUniformLocation(m_shader, "cDiffuse"), shape.primitive.material.cDiffuse[0],
                    shape.primitive.material.cDiffuse[1],
                    shape.primitive.material.cDiffuse[2],
                    shape.primitive.material.cDiffuse[3]);

        glUniform4f(glGetUniformLocation(m_shader, "cSpecular"), shape.primitive.material.cSpecular[0],
                    shape.primitive.material.cSpecular[1],
                    shape.primitive.material.cSpecular[2],
                    shape.primitive.material.cSpecular[3]);

        glUniform1f(glGetUniformLocation(m_shader, "shininess"), shape.primitive.material.shininess);
        // perform draw
        glDrawArrays(GL_TRIANGLES, 0, size / 6);

        // unbind vao
        glBindVertexArray(0);

    }

    glUseProgram(0);
}


// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        float sensitivity = 0.005f;
        float yawAngle = deltaX * sensitivity;
        float pitchAngle = deltaY * sensitivity;

        glm::vec3 look = glm::vec3(curRenderData.cameraData.look);
        glm::vec3 up = glm::vec3(curRenderData.cameraData.up);


        // Yaw rotation matrix around world up vector
        glm::mat4 yawRotation = createRotationMatrix(glm::vec3(0, 1, 0), yawAngle);

        // Pitch rotation matrix around right vector
        glm::vec3 right = glm::normalize(glm::cross(look, up));
        glm::mat4 pitchRotation = createRotationMatrix(right, pitchAngle);

        // Apply rotations to the look and up vectors
        glm::vec4 newLook = pitchRotation * yawRotation * glm::vec4(look, 0.0f);
        curRenderData.cameraData.look = newLook;

        glm::vec4 newUp = yawRotation * glm::vec4(up, 0.0f); // Up vector only rotates around yaw
        curRenderData.cameraData.up = newUp;


        // Update camera view matrix
        curRenderData.cameraData.updateView();
        curView = curRenderData.cameraData.view;

//        // Yaw rotation matrix around world up vector
//        glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), yawAngle, glm::vec3(0, 1, 0));

//        // Pitch rotation matrix around right vector
//        glm::vec3 right = glm::normalize(glm::cross(look, up));
//        glm::mat4 pitchRotation = glm::rotate(glm::mat4(1.0f), pitchAngle, right);

//        // Apply rotations to the look and up vectors
//        glm::vec4 newLook = pitchRotation * yawRotation * glm::vec4(look, 0.0f);
//        curRenderData.cameraData.look = newLook;

//        glm::vec4 newUp = yawRotation * glm::vec4(up, 0.0f); // Up vector only rotates around yaw
//        curRenderData.cameraData.up = newUp;

//        // Update camera view matrix
//        curRenderData.cameraData.updateView();
//        curView = curRenderData.cameraData.view;


        update(); // asks for a PaintGL() call to occur
    }
}


void Realtime::makeFBO() {
    glGenTextures(1, &m_fbo_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // set linear interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);

    // Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Realtime::paintTexture(GLuint texture, bool invert, bool blur) {
    glUseProgram(m_texture_shader);
    glUniform1f(glGetUniformLocation(m_texture_shader, "width"), 1.0f * m_fbo_width);
    glUniform1f(glGetUniformLocation(m_texture_shader, "height"), 1.0f * m_fbo_height);

    // Task 32: Set your bool uniform on whether or not to filter the texture drawn
    if (invert) {
        glUniform1i(glGetUniformLocation(m_texture_shader, "post_pro"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(m_texture_shader, "post_pro"), 0);
    }

    if (blur) {
        glUniform1i(glGetUniformLocation(m_texture_shader, "blur"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(m_texture_shader, "blur"), 0);
    }


    glBindVertexArray(m_fullscreen_vao);
    // Task 10: Bind "texture" to slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);


    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    float speed = 5.0f;


    // Use deltaTime and m_keyMap here to move around
    glm::vec3 forward = glm::normalize(glm::vec3(curRenderData.cameraData.look));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::normalize(glm::vec3(curRenderData.cameraData.up))));

    glm::vec3 translation = glm::vec3(0.0f);

    if (m_keyMap[Qt::Key_W]) {
        translation += forward;
    }
    if (m_keyMap[Qt::Key_S]) {
        translation -= forward;
    }
    if (m_keyMap[Qt::Key_A]) {
        translation -= right;
    }
    if (m_keyMap[Qt::Key_D]) {
        translation += right;
    }
    if (m_keyMap[Qt::Key_Space]) {
        translation += glm::vec3(0, 1, 0); // Upward in world space
    }
    if (m_keyMap[Qt::Key_Control]) {
        translation += glm::vec3(0, -1, 0); // Downward in world space
    }

    glm::mat4 transMat = glm::mat4(1.0f);

    translation *= speed * deltaTime;
    transMat[3][0] = translation.x;
    transMat[3][1] = translation.y;
    transMat[3][2] = translation.z;

    curRenderData.cameraData.pos = transMat * curRenderData.cameraData.pos;
//    curRenderData.cameraData.look = transMat * curRenderData.cameraData.look;

    // update view matrix
    curRenderData.cameraData.updateView();
    curView = curRenderData.cameraData.view;


    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
