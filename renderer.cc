#include "renderer.hh"
#include "resource-cache.hh"

#include <iostream>
#include <sstream>

using namespace std;

Renderer::Renderer(SDL_Window *window) :
  window(window)
{
  this->context = SDL_GL_CreateContext(window);

  // initialize glew
  GLenum status = glewInit();
  if (status != GLEW_OK) {
    cout << "Could not initialize GLEW." << endl;
    exit(1);
  }

  if (!GLEW_VERSION_3_3) {
    cout << "GLEW version 3.3 not found." << endl;
    exit(1);
  }

  // Enable blending.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Get background texture.
  this->backgroundTexture = ResourceCache::GetTexture("background");

  //glBindTexture(GL_TEXTURE_2D, this->backgroundTexture);
  //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &this->backgroundTextureWidth);
  //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &this->backgroundTextureHeight);
  //glBindTexture(GL_TEXTURE_2D, 0);

  // The commented lines above do not work at the moment because SOIL
  // resizes images when requested to make MIPMAPS and makes them
  // square sized. These lines are just a temporary workaround. Image
  // size should not be hard coded here.
  this->backgroundTextureWidth = 1920;
  this->backgroundTextureHeight = 1080;

  // Create background vertex buffer object.
  this->RebuildBackground();
  SDL_GetWindowSize(this->window, &this->lastWindowWidth, &this->lastWindowHeight);
}

Renderer::~Renderer() {
  glDeleteBuffers(1, &this->backgroundVbo);
}

void Renderer::RebuildBackground() {
  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);

  float windowRatio = (float) winw / winh;

  GLfloat tex_x1, tex_y1, tex_x2, tex_y2;
  float backgroundTextureRatio = (float) this->backgroundTextureWidth / this->backgroundTextureHeight;
  if (backgroundTextureRatio < windowRatio) {
    tex_x1 = 0.0f;
    tex_x2 = 1.0f;
    tex_y1 = 0.5f * ((float) winw / this->backgroundTextureWidth - (float) winh / this->backgroundTextureHeight);
    tex_y2 = 1.0 - 0.5f * ((float) winw / this->backgroundTextureWidth - (float) winh / this->backgroundTextureHeight);
  }
  else {
    tex_y1 = 0.0f;
    tex_y2 = 1.0f;
    tex_x1 = 0.5f * ((float) winh / this->backgroundTextureHeight - (float) winw / this->backgroundTextureWidth);
    tex_x2 = 1.0 - 0.5f * ((float) winh / this->backgroundTextureHeight - (float) winw / this->backgroundTextureWidth);
  }

  const GLfloat vertexData[] = {
    // triangle 1
    /* coord */ -1.0f, -1.0f, /* tex_coord */ tex_x1, tex_y1,
    /* coord */ -1.0f,  1.0f, /* tex_coord */ tex_x1, tex_y2,
    /* coord */  1.0f, -1.0f, /* tex_coord */ tex_x2, tex_y1,

    // triangle 2
    /* coord */ -1.0f,  1.0f, /* tex_coord */ tex_x1, tex_y2,
    /* coord */  1.0f,  1.0f, /* tex_coord */ tex_x2, tex_y2,
    /* coord */  1.0f, -1.0f, /* tex_coord */ tex_x2, tex_y1,
  };

  glGenBuffers(1, &this->backgroundVbo);

  glBindBuffer(GL_ARRAY_BUFFER, this->backgroundVbo);
  glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::PresentScreen() const {
  SDL_GL_SwapWindow(this->window);
}

void Renderer::SetCamera(Camera &camera) {
  this->camera = camera;
}

void Renderer::ClearScreen() {
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::DrawBackground() {
  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);
  if (winw != this->lastWindowWidth || winh != this->lastWindowHeight) {
    this->lastWindowWidth = winw;
    this->lastWindowHeight = winh;
    this->RebuildBackground();
  }

  GLuint program = ResourceCache::backgroundProgram;

  glUseProgram(program);

  GLuint textureUniform = glGetUniformLocation(program, "texture0");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, this->backgroundTexture);
  glUniform1i(textureUniform, 0); // set it to 0  because the texture is bound to GL_TEXTURE0

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindBuffer(GL_ARRAY_BUFFER, this->backgroundVbo);

  GLint coordAttr = glGetAttribLocation(program, "coord");
  GLint texCoordAttr = glGetAttribLocation(program, "tex_coord");

  glEnableVertexAttribArray(coordAttr);
  glEnableVertexAttribArray(texCoordAttr);

  glVertexAttribPointer(coordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) 0);
  glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) (2 * sizeof(GLfloat)));

  glDrawArrays(GL_TRIANGLES, 0, 6);
  if (glGetError() != GL_NO_ERROR)
    cout << "OpenGL draw error." << endl;

  glDisableVertexAttribArray(coordAttr);
  glDisableVertexAttribArray(texCoordAttr);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}
