#pragma once
#undef DrawText
#include <scorch/vkapi/device.h>
#include <shudcpp/shud.h>
#include <deque>

namespace ScorchEngine {
	class UIPerformanceViewer {
	public:
		UIPerformanceViewer(SEDevice& device, uint32_t dtRegressionFrames = 100);

		UIPerformanceViewer(const UIPerformanceViewer&) = delete;
		UIPerformanceViewer& operator=(const UIPerformanceViewer&) = delete;
		UIPerformanceViewer(UIPerformanceViewer&&) = delete;
		UIPerformanceViewer& operator=(UIPerformanceViewer&&) = delete;

		void update(float dt);
		void renderFps(SHUD::DrawList* drawList, SHUD::fvec2 position);
		void renderDtGraph(SHUD::DrawList* drawList, SHUD::fvec2 xyOrigin, SHUD::fvec2 graphSize);
		void renderGpuUsage(SHUD::DrawList* drawList, SHUD::fvec2 position);
	private:
		void addDtToRegression(float deltaTime);
		float dtFromPercentile(float percentile);
		void sortRegression();

		size_t dtRegressionMaxFrames;
		std::deque<float> dtRegression;
		std::deque<float> sortedRegression;
		SEDevice& device;
	};
}