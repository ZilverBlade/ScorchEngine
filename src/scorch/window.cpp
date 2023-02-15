#include "window.h"

// std
#include <stdexcept>

namespace ScorchEngine {

    SEWindow::SEWindow(VkInstance instance, int w, int h, const char* name) : instance(instance), width{w}, height{h}, windowName{name} {
        initWindow();
        createWindowSurface(instance, &surface);
    }
    
    SEWindow::~SEWindow() {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    void SEWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }
    
    void SEWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface");
        }
    }
    
    void SEWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
      auto wnd = reinterpret_cast<SEWindow *>(glfwGetWindowUserPointer(window));
      wnd->framebufferResized = true;
      wnd->width = width;
      wnd->height = height;
    }
}
