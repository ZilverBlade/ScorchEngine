#include <scorch/apps/window_test.h>
#include <scorch/apps/vulkan_test.h>

int main() {
	ScorchEngine::App* app = new ScorchEngine::VulkanTest("ScorchEngine v1.0");

	try {
		app->run();
	} catch(std::exception ex) {
		SELOG_ERR("%s", ex.what());
		return EXIT_FAILURE;
	}

	delete app;
	return EXIT_SUCCESS;
}