#include "window.h"

// std
#include <stdexcept>

namespace ScorchEngine {

    SEWindow::SEWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
        initWindow();
    }
    
    SEWindow::~SEWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    void SEWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }
    
    void SEWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to craete window surface");
        }
    }
    
    void SEWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
      auto wnd = reinterpret_cast<SEWindow *>(glfwGetWindowUserPointer(window));
      wnd->framebufferResized = true;
      wnd->width = width;
      wnd->height = height;
    }
}
