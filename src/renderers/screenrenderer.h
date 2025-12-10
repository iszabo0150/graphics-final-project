#pragma once

#include <GL/glew.h>
#include <vector>

class ScreenRenderer {
public:
    ScreenRenderer();
    ~ScreenRenderer();
    
    void initialize();
    void cleanup();
    
    // Render a texture to the screen
    void renderToScreen(GLuint texture, int width, int height);
    
private:
    void initializeFullscreenQuad();
    
    GLuint m_shader;
    GLuint m_quadVAO;
    GLuint m_quadVBO;
};
