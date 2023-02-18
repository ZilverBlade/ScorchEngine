#include <scorch/apps/window_test.h>

namespace ScorchEngine::Apps {
	WindowTest::WindowTest(const char* name) : App(name) {
	
	}
	WindowTest::~WindowTest() {}
	void WindowTest::run() {
		SELOG_INF("%s", "Hello World!");
		SELOG_WRN("%s", "Warning!");
		SELOG_ERR("%s", "Error!");
		while (!seWindow.shouldClose()) {
			glfwPollEvents();
		}
	}
}