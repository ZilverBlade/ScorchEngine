#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
namespace ScorchEngine {

class SEWindow {
 public:
  SEWindow(VkInstance instance, int w, int h, const char* name);
  ~SEWindow();

  SEWindow(const SEWindow &) = delete;
  SEWindow &operator=(const SEWindow &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(window); }
  VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
  bool wasWindowResized() { return framebufferResized; }
  void resetWindowResizedFlag() { framebufferResized = false; }
  GLFWwindow *getGLFWwindow() const { return window; }
  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

  VkSurfaceKHR getSurface() {
	  return surface;
  }
 private:
  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
  void initWindow();

  int width;
  int height;
  bool framebufferResized = false;

  const char* windowName;
  GLFWwindow *window;
  VkSurfaceKHR surface{};
  VkInstance instance;
};
}  // namespace ScorchEngine
