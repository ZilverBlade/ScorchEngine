#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
namespace ScorchEngine {

class SEWindow {
 public:
  SEWindow(int w, int h, std::string name);
  ~SEWindow();

  SEWindow(const SEWindow &) = delete;
  SEWindow &operator=(const SEWindow &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(window); }
  VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
  bool wasWindowResized() { return framebufferResized; }
  void resetWindowResizedFlag() { framebufferResized = false; }
  GLFWwindow *getGLFWwindow() const { return window; }

  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

 private:
  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
  void initWindow();

  int width;
  int height;
  bool framebufferResized = false;

  std::string windowName;
  GLFWwindow *window;
};
}  // namespace ScorchEngine
