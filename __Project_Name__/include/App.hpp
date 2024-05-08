#pragma once

#include "Application.hpp"

class App : public Application
{
public:
  virtual void init() override;
  virtual void stop() override;

  virtual void update_ui() override;

  virtual void update(float timestep) override;
  virtual void render() override;

private:

  glm::vec3 positions[3] = {
    {  0.0f,  0.5f, 0.0f },
    { -0.5f, -0.5f, 0.0f },
    {  0.5f, -0.5f, 0.0f }
  };



  Shader m_shader;

  GLuint m_vertex_array = 0;
  GLuint m_position_buffer = 0;
  GLuint m_color_buffer = 0;
};
