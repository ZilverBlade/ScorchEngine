#include <scorch/apps/tests/window_test.h>
#include <scorch/apps/tests/vulkan_test.h>
#include <scorch/apps/renderers/sponza_app.h>
#include <scorch/apps/renderers/grass_app.h>
#include <scorch/apps/renderers/caustics.h>

#ifdef _WIN32	
#include <Windows.h>
#include <commdlg.h>
#include <shlobj_core.h>
#endif
int main() {
	ScorchEngine::Apps::App* app = new ScorchEngine::Apps::SponzaApp("ScorchEngine v1.0");

	//try {
		app->run();
	//} catch(std::exception ex) {
	//	SELOG_ERR("%s", ex.what());
#ifdef _WIN32	
	//	MessageBoxA(nullptr, ex.what(), "FATAL ENGINE ERROR", MB_ABORTRETRYIGNORE | MB_ICONERROR);
#endif
	//	return EXIT_FAILURE;
	//}

	delete app;
	return EXIT_SUCCESS;
}