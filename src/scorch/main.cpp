#include <scorch/apps/tests/window_test.h>
#include <scorch/apps/tests/vulkan_test.h>
#include <scorch/apps/tests/model_test.h>
#include <scorch/apps/tests/lighting_test.h>

int main() {
	ScorchEngine::Apps::App* app = new ScorchEngine::Apps::LightingTest("ScorchEngine v1.0");

	try {
		app->run();
	} catch(std::exception ex) {
		SELOG_ERR("%s", ex.what());
		return EXIT_FAILURE;
	}

	delete app;
	return EXIT_SUCCESS;
}