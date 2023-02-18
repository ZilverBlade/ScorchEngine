#pragma once

#include <scorch/log.h>

#include <scorch/window.h>
#include <scorch/vkapi/device.h>
//#include <scorch/vkapi/renderer.h>

namespace ScorchEngine::Apps {
	class App {
	public:
		App(const char* name);
		virtual ~App();

		App(const App&) = delete;
		App& operator=(const App&) = delete;
		App(App&&) = delete;
		App& operator=(App&&) = delete;

		virtual void run();
	protected:
		SEDevice seDevice{};
		SEWindow seWindow;

	};
}