#include "__Project_Name__.hpp"

#include "Application.hpp"
#include <glad/gl.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;
namespace chrono = std::chrono;
using loop_clock = chrono::steady_clock;
using clock_unit = chrono::milliseconds;

Application::Application() : window(*this) {}

void Application::run()
{
  running = true;

  _init();
  loop();
  _stop();
}

// callbacks
static void error_callback(int error, const char *description)
{
  fprintf(stderr, "Error: %s\n", description);
}


void Application::timed_loop(std::function<bool(void)> conditionChecker, std::function<void(float)> function)
{
  // cap the loop to a minimum interval to ensure a good deltatime acuracy
  constexpr auto LOOP_TRESHOLD = 1ms;

  loop_clock::time_point now;
  loop_clock::time_point last = loop_clock::now();

  while (conditionChecker())
  {
    now = loop_clock::now();
    chrono::duration elapsed = now - last;

    if (elapsed < LOOP_TRESHOLD)
    {
      std::this_thread::sleep_for(LOOP_TRESHOLD - elapsed);
      elapsed = LOOP_TRESHOLD;
    }

    const float deltatime = chrono::duration_cast<clock_unit>(elapsed).count() / 1000.0f;
    function(deltatime);
    last = now;
  }
}


void Application::loop()
{
  m_updating = false;

  m_updateThread = std::thread(&Application::updateLoop, this);

  while (!m_updating);

  timed_loop(
    [this]() { return (running && window.isOpen()); },
    [this](float deltatime) {
      window.update();

      const bool isMouseGrabbed = (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_NORMAL);
      if (isMouseGrabbed && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
        camera.update(deltatime, window);

      _update_ui();
      _render();
    }
  );
  running = false;

  m_updateThread.join();
}

void Application::updateLoop()
{
  timed_loop(
    [this]() { return !!running; },
    [this](float deltatime) {
      _update(deltatime);
      m_updating = true;
    }
  );
}

/// This function isn't used anymore due to imgui requirement of having opengl context and glfw events on the same thread
void Application::renderLoop()
{
  while (!m_updating);

  window.grabRenderingContext();

  timed_loop(
    [this]() { return (running && window.isOpen()); },
    [this](float) { _render(); }
  );

  window.releaseRenderingContext();
  running = false;
}


void Application::_init()
{
  glfwInit();
  glfwSetErrorCallback(error_callback);

  window.create(PROJECT_NAME, 1280, 720);
  window.setVSync(true);

  {
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
  
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigDockingWithShift = true;
  
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
  }

  camera.setProjection(Camera::ProjType::perspective);

  glEnable(GL_CULL_FACE);
  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  init();
}

void Application::_stop()
{
  running = false;

  stop();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  window.clear();
  glfwTerminate();
}

void Application::_update(float timestep)
{
  update(timestep);
}

void Application::_render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  window.render();
}


void Application::_update_ui()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();

  ImGui::NewFrame();

  update_ui();

  ImGui::EndFrame();

  ImGui::Render();

  const ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();

    // Platform functions may change the current OpenGL context, so we restore it to make sure the render pipelien still works
    window.grabRenderingContext();
  }
}

void Application::onResize(Window &window, int width, int height)
{
  glViewport(0, 0, width, height);
  camera.setViewport(width, height);
}

void Application::onClick(Window &window, int button, int action, int mods)
{
  if (ImGui::GetIO().WantCaptureMouse)
    return;

  if (button == GLFW_MOUSE_BUTTON_2)
  {
    if (action == GLFW_PRESS)
    {
      glfwGetCursorPos(window, &m_cursorSave.x, &m_cursorSave.y);
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      glfwSetCursorPos(window, 0, 0);
    }
    else if (action == GLFW_RELEASE)
    {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      glfwSetCursorPos(window, m_cursorSave.x, m_cursorSave.y);
    }
  }
}

void Application::onKeyboard(Window &window, int key, int scancode, int action, int mods)
{
  if (ImGui::GetIO().WantCaptureKeyboard)
    return;

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    window.close();

  if (action != GLFW_PRESS)
    return;

  if (key == GLFW_KEY_P)
  {
    if (camera.getProjectionType() == Camera::ProjType::perspective)
      camera.setProjection(Camera::ProjType::orthographic);
    else
      camera.setProjection(Camera::ProjType::perspective);
  }

  if (key == GLFW_KEY_O)
  {
    std::cout << "Camera:\n"
      << "  x: " << camera.position.x << "\n"
      << "  y: " << camera.position.y << "\n"
      << "  z: " << camera.position.z << "\n"
      << "  yaw: " << camera.yaw << "\n"
      << "  pitch: " << camera.pitch << std::endl;
  }
}