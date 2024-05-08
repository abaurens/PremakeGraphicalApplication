#pragma once

#include "Window.hpp"
#include "Camera.hpp"

#include "Shader.hpp"

#include <vector>
#include <thread>

class Application
{
public:
  Application();
  void run();

protected:
  virtual void init() {};
  virtual void stop() {};

  virtual void render() {};
  virtual void update(float timestep) {};

  virtual void update_ui() {};

private:
  void loop();
  void updateLoop();
  void renderLoop();

  void timed_loop(std::function<bool(void)> conditionChecker, std::function<void(float)> function);

  void _init();
  void _stop();

  void _update_ui();

  void _render();
  void _update(float timestep);

public:
  virtual void onResize(Window &window, int width, int height);
  virtual void onClick(Window &window, int button, int action, int mods);
  virtual void onKeyboard(Window &window, int key, int scancode, int action, int mods);



protected:
  Camera camera;
  Window window;

private:
  std::atomic<bool> running = false;
  glm::dvec2 m_cursorSave = { 0, 0 };

private:
  std::atomic<bool> m_updating;
  std::thread m_updateThread;
};