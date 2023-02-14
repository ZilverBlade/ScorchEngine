#pragma once

#include <scorch/log.h>

#include <scorch/window.h>
#include <scorch/vkapi/device.h>
//#include <scorch/vkapi/renderer.h>

namespace ScorchEngine {
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
		SEWindow seWindow;
		SEDevice seDevice{};
		//SERenderer seRenderer = { seWindow, seDevice };
	};
}