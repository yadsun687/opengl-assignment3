#ifndef PICKINGTEXTURE_H
#define PICKINGTEXTURE_H

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_projection.hpp>
#include <glm/glm.hpp>

class PickingTexture {
public:
  // refer to fragment position of drawn object
  struct PixelInfo {
    float vertX = 0;
    float vertY = 0;
    float vertZ = 0;
  };

  PickingTexture() {}
  ~PickingTexture() {};

  void init(unsigned int windowWidth, unsigned int windowHeight) {
    screenWidth = windowWidth;
    screenHeight = windowHeight;
    // Create the FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create the texture object for the primitive information buffer
    glGenTextures(1, &m_pickingTexture);
    glBindTexture(GL_TEXTURE_2D, m_pickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           m_pickingTexture, 0);

    // Create the texture object for the depth buffer
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth,
                 windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           m_depthTexture, 0);

    // Verify that the FBO is correct
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
      printf("FB error, status: 0x%x\n", Status);
      exit(1);
    }

    // Restore the default framebuffer
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  };

  void enableWriting() { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo); }

  void disableWriting() { // Bind back the default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  }

  PixelInfo ReadPixel(unsigned int x, unsigned int y) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);

    glReadBuffer(GL_COLOR_ATTACHMENT0);

    PixelInfo Pixel;
    glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, &Pixel);

    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    return Pixel;
  }

  glm::vec3 getClickedPosition(unsigned int x, unsigned int y, glm::mat4 view,
                               glm::mat4 projection) {

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    // Read depth
    float winZ;
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    // Unproject
    glm::vec3 windowPos = glm::vec3(x, y, winZ);
    glm::ivec4 viewport(0, 0, screenWidth, screenHeight);
    glm::vec3 worldPos = glm::unProject(windowPos, view, projection, viewport);

    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    return glm::vec3(worldPos);
  }

private:
  GLuint m_fbo = 0;
  GLuint m_pickingTexture = 0;
  GLuint m_depthTexture = 0;
  unsigned int screenWidth, screenHeight;
};

#endif
