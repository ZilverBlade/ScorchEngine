#include "performance_viewer.h"
#include <glm/glm.hpp>
#include <algorithm>

namespace ScorchEngine {
	UIPerformanceViewer::UIPerformanceViewer(SEDevice& device, uint32_t dtRegressionFrames) 
		: device(device), dtRegressionMaxFrames(dtRegressionFrames){
	}
	void UIPerformanceViewer::update(float dt) {
		addDtToRegression(dt);
		sortRegression();
	}

	void UIPerformanceViewer::renderFps(SHUD::DrawList* drawList, SHUD::fvec2 position) {
		using namespace SHUD;
		float dt = dtRegression.back();
		float currFps = 1.0 / dt;
		float median = 1.0 / dtFromPercentile(0.5f);
		float bottom10Percentile = 1.0 / dtFromPercentile(1.0f - 0.1f);
		float bottom1Percentile = 1.0 / dtFromPercentile(1.0f - 0.01f);

		Transform rectTransform{};
		rectTransform.mPosition = position;
		rectTransform.mScale = {300.f, 150.f}; // extra padding
		rectTransform.mTransformOffset = SHUD_ANCHOR_OFFSET_TOP_RIGHT;
		drawList->DrawRect(rectTransform, 0x30303044, 0, SHUD_ANCHOR_OFFSET_TOP_RIGHT);
		TextFormatting text;
		text.mBold = true;
		text.mSizePx = 30;
		text.mHAlignment = TextFormattingHAlignment::Right;
		Transform textTransform{};
		textTransform.mPosition = position;
		textTransform.mScale = { 1.f,1.f };
		textTransform.mTransformOffset = SHUD_ANCHOR_OFFSET_TOP_RIGHT;
		drawList->DrawText(textTransform, 0xFFFFFFFF, 1, text, { "FPS: %i", (int)currFps }, SHUD_ANCHOR_OFFSET_TOP_RIGHT);
		textTransform.mPosition.y += 50.0f;
		drawList->DrawText(textTransform, 0xFFFFFFFF, 1, text, { "MEDIAN: %i", (int)median }, SHUD_ANCHOR_OFFSET_TOP_RIGHT);
		textTransform.mPosition.y += 50.0f;
		drawList->DrawText(textTransform, 0xFFFFFFFF, 1, text, { "1%: %i", (int)bottom1Percentile }, SHUD_ANCHOR_OFFSET_TOP_RIGHT);
	}

	void UIPerformanceViewer::renderDtGraph(SHUD::DrawList* drawList, SHUD::fvec2 xyOrigin, SHUD::fvec2 graphSize) {
		using namespace SHUD;
		float lowerDt = dtFromPercentile(0.0f);
		float upperDt = dtFromPercentile(1.0f);
		float dtSize = std::max(upperDt - lowerDt, 0.0001f);
		
		size_t samples = std::min(dtRegressionMaxFrames, dtRegression.size());
		float step = graphSize.x / dtRegressionMaxFrames;

		fvec2 lowerLeft = { xyOrigin.x, xyOrigin.y + graphSize.y };
		fvec2 upperRight = { xyOrigin.x + graphSize.x, xyOrigin.y };
		
		Transform rectTransform{};
		rectTransform.mPosition = xyOrigin;
		rectTransform.mScale = graphSize + 20.0f; // extra padding
		rectTransform.mTransformOffset = SHUD_ANCHOR_OFFSET_TOP_LEFT;
		drawList->DrawRect(rectTransform, 0x30303044, 0, SHUD_ANCHOR_OFFSET_TOP_LEFT);

		fvec2 lastPoint = -1.0f;
		for (int i = 0; i < samples; ++i) {
			fvec2 currentPoint = fvec2{ lowerLeft.x + i * step, glm::mix(lowerLeft.y, upperRight.y, dtRegression[i] / dtSize) };
			if (lastPoint.x == -1.0f) {
				lastPoint = currentPoint;
			}
			drawList->DrawLine(lastPoint, currentPoint, 0xFF0000FF, 1, SHUD_ANCHOR_OFFSET_TOP_LEFT);
			lastPoint = currentPoint;
		}
	}

	void UIPerformanceViewer::renderGpuUsage(SHUD::DrawList* drawList, SHUD::fvec2 position) {
		
	}

	void UIPerformanceViewer::addDtToRegression(float deltaTime) {
		if (dtRegression.size() >= dtRegressionMaxFrames) {
			dtRegression.pop_front();
		}
		dtRegression.push_back(deltaTime);
	}
	float UIPerformanceViewer::dtFromPercentile(float percentile) {
		size_t index = std::max(std::ceil(percentile * sortedRegression.size()) - 1, 0.f);
		bool odd = sortedRegression.size() % 2;

		if (odd) {
			return sortedRegression[index];
		} else {
			return (sortedRegression[index] + sortedRegression[std::min(index + 1, sortedRegression.size() - 1)]) / 2.0f;
		}
	}
	void UIPerformanceViewer::sortRegression() {
		sortedRegression = dtRegression;
		std::sort(sortedRegression.begin(), sortedRegression.end());
	}
}