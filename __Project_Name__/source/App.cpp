#include "App.hpp"

#include "imgui.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

constexpr glm::vec3 colors[] = {
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.5f, 0.0f, 1.0f }
};

void App::init()
{
  // init opengl test scene
  m_shader.addFile(Shader::Type::vertex, "shaders/cloth.vert");
  m_shader.addFile(Shader::Type::fragment, "shaders/cloth.frag");
  m_shader.compile();

  glGenVertexArrays(1, &m_vertex_array);
  glBindVertexArray(m_vertex_array);

  glGenBuffers(1, &m_position_buffer);
  glGenBuffers(1, &m_color_buffer);

  glBindBuffer(GL_ARRAY_BUFFER, m_position_buffer);
  glEnableVertexArrayAttrib(m_vertex_array, m_shader.getAttribute("vPos"));
  glVertexAttribPointer(m_shader.getAttribute("vPos"), 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 3, positions, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, m_color_buffer);
  glEnableVertexArrayAttrib(m_vertex_array, m_shader.getAttribute("vCol"));
  glVertexAttribPointer(m_shader.getAttribute("vCol"), 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 3, colors, GL_STATIC_DRAW);

  camera.position = { 0.2, 0.0, 1.5 };
  camera.rotation = { 0.0, 0.0 };
  camera.setProjection(Camera::ProjType::perspective);

  glDisable(GL_CULL_FACE);
}

void App::stop()
{
  m_shader.clear();

  if (glIsBuffer(m_position_buffer))
    glDeleteBuffers(1, &m_position_buffer);
  if (glIsBuffer(m_color_buffer))
    glDeleteBuffers(1, &m_color_buffer);
  if (glIsVertexArray(m_vertex_array))
    glDeleteVertexArrays(1, &m_vertex_array);
}

#include <iostream>

void App::update(float timestep)
{
  constexpr float radius = 0.12f;
  constexpr float loop_period = 5.0f;

  constexpr auto sigmoid = [](float x) { return 1.0f / (1.0f + exp(-5.0f * x)); };

  constexpr auto altered_sigmoid = [](float x) {
    const float s0 = sigmoid(0 - 0.5f);
    const float s1 = sigmoid(1 - 0.5f);

    const float scale = s1 - s0;

    return (sigmoid(x - 0.5f)) / scale - s0;
  };

  const float time = (float)glfwGetTime();

  // time loops back after loop_period seconds
  const float clamped = glm::mod(time, loop_period) / loop_period;

  const float s = altered_sigmoid(clamped);
  const float angle1 = glm::radians(s * 360.0f);
  const float angle2 = glm::radians(s * 360.0f + 120.0f);
  const float angle3 = glm::radians(s * 360.0f + 240.0f);

  positions[0] = glm::vec3( 0.0f + cos(angle1) * radius,    0.5f + sin(angle1) * radius,   0.0f);
  positions[1] = glm::vec3(-0.5f + cos(angle2) * radius,   -0.5f + sin(angle2) * radius,   0.0f);
  positions[2] = glm::vec3( 0.5f + cos(angle3) * radius,   -0.5f + sin(angle3) * radius,   0.0f);
}

void App::render()
{
  glBindVertexArray(m_vertex_array);

  glBindBuffer(GL_ARRAY_BUFFER, m_position_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 3, positions, GL_DYNAMIC_DRAW);

  glUseProgram(m_shader.id());

  glm::mat4 mvp = camera.getProj() * camera.getView();
  glUniformMatrix4fv(m_shader.getUniform("MVP"), 1, GL_FALSE, glm::value_ptr(mvp));
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void App::update_ui()
{
  ImGui::ShowDemoWindow();
}