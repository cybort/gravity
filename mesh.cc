#include "mesh.hh"
#include "resource-cache.hh"

#include <iostream>

Mesh::Mesh(const GLfloat *vertexData, int n, GLuint texture, bool hud) :
  vertexCount(n),
  texture(texture)
{
  glGenBuffers(1, &this->vbo);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, n * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  if (hud)
    this->program = ResourceCache::hudTexturedPolygonProgram;
  else
    this->program = ResourceCache::texturedPolygonProgram;
}

Mesh::~Mesh() {
  glDeleteBuffers(1, &this->vbo);
}

void Mesh::Draw(const b2Vec2 &pos, float32 angle, float32 scale_factor) const {
  glUseProgram(this->program);

  GLuint textureUniform = glGetUniformLocation(this->program, "texture0");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, this->texture);
  glUniform1i(textureUniform, 0); // set it to 0  because the texture is bound to GL_TEXTURE0

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  GLint coordAttr = glGetAttribLocation(this->program, "coord");
  GLint texCoordAttr = glGetAttribLocation(this->program, "tex_coord");
  GLint positionAttr = glGetAttribLocation(this->program, "position");
  GLint angleAttr = glGetAttribLocation(this->program, "angle");
  GLint scaleAttr = glGetAttribLocation(this->program, "scale_factor");

  glEnableVertexAttribArray(coordAttr);
  glEnableVertexAttribArray(texCoordAttr);

  glVertexAttribPointer(coordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) 0);
  glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) (2 * sizeof(GLfloat)));
  glVertexAttrib2f(positionAttr, pos.x, pos.y);
  glVertexAttrib1f(angleAttr, angle);
  glVertexAttrib1f(scaleAttr, scale_factor);

  glDrawArrays(GL_TRIANGLES, 0, this->vertexCount);
  if (glGetError() != GL_NO_ERROR)
    cout << "OpenGL draw error." << endl;

  glDisableVertexAttribArray(coordAttr);
  glDisableVertexAttribArray(texCoordAttr);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}
