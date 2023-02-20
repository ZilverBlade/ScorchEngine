#include <scorch/apps/tests/window_test.h>
#include <scorch/apps/tests/vulkan_test.h>
#include <scorch/apps/tests/model_test.h>
#include <scorch/apps/tests/lighting_test.h>

#ifdef _WIN32	
#include <Windows.h>
#include <commdlg.h>
#include <shlobj_core.h>
#endif
int main() {
	ScorchEngine::Apps::App* app = new ScorchEngine::Apps::LightingTest("ScorchEngine v1.0");

	try {
		app->run();
	} catch(std::exception ex) {
		SELOG_ERR("%s", ex.what());
		MessageBoxA(nullptr, ex.what(), "FATAL ENGINE ERROR", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}

	delete app;
	return EXIT_SUCCESS;
}