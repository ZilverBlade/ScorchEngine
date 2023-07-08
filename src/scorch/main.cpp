#include <scorch/apps/tests/window_test.h>
#include <scorch/apps/tests/vulkan_test.h>
#include <scorch/apps/tests/lighting_test.h>
#include <scorch/apps/renderers/render_app.h>

#ifdef _WIN32	
#include <Windows.h>
#include <commdlg.h>
#include <shlobj_core.h>
#endif
int main() {
	ScorchEngine::Apps::App* app = new ScorchEngine::Apps::RenderApp("ScorchEngine v1.0");

	try {
		app->run();
	} catch(std::exception ex) {
		SELOG_ERR("%s", ex.what());
#ifdef _WIN32	
		MessageBoxA(nullptr, ex.what(), "FATAL ENGINE ERROR", MB_OK | MB_ICONERROR);
#endif
		return EXIT_FAILURE;
	}

	delete app;
	return EXIT_SUCCESS;
}