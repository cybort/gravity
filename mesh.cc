#include "mesh.hh"
#include "resource-cache.hh"

#include <iostream>

Mesh::Mesh(const GLfloat *vertexData, int n, GLuint texture) :
  vertexCount(n),
  texture(texture)
{
  glGenBuffers(1, &this->vbo);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, n * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Mesh::~Mesh() {
  glDeleteBuffers(1, &this->vbo);
}

void Mesh::Draw(const b2Vec2 &pos, float32 angle) const {
  GLuint program = ResourceCache::texturedPolygonProgram;

  glUseProgram(program);

  GLuint textureUniform = glGetUniformLocation(program, "texture0");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, this->texture);
  glUniform1i(textureUniform, 0); // set it to 0  because the texture is bound to GL_TEXTURE0

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  GLint coordAttr = glGetAttribLocation(program, "coord");
  GLint texCoordAttr = glGetAttribLocation(program, "tex_coord");
  GLint positionAttr = glGetAttribLocation(program, "position");
  GLint angleAttr = glGetAttribLocation(program, "angle");
  GLint scaleAttr = glGetAttribLocation(program, "scale_factor");

  glEnableVertexAttribArray(coordAttr);
  glEnableVertexAttribArray(texCoordAttr);

  glVertexAttribPointer(coordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) 0);
  glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) (2 * sizeof(GLfloat)));
  glVertexAttrib2f(positionAttr, pos.x, pos.y);
  glVertexAttrib1f(angleAttr, angle);
  glVertexAttrib1f(scaleAttr, 1.0f);

  glDrawArrays(GL_TRIANGLES, 0, this->vertexCount);
  if (glGetError() != GL_NO_ERROR)
    cout << "OpenGL draw error." << endl;

  glDisableVertexAttribArray(coordAttr);
  glDisableVertexAttribArray(texCoordAttr);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}
