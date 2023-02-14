#pragma once

#include <scorch/apps/app.h>

namespace ScorchEngine {
	class WindowTest : public App {
	public:
		WindowTest(const char* name);
		virtual ~WindowTest();

		virtual void run();
	protected:

	};
}