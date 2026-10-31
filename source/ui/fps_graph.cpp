#include "fps_graph.h"
#include <raylib.h>

void ui_frametime(float scale)
{
	constexpr int HISTORY_COUNT = 60;
	static float ms_history[HISTORY_COUNT] = { 0 };
	static int history_offset = 0;

	float current_ms = GetFrameTime() * 1000.0f;
	ms_history[history_offset] = current_ms;
	history_offset = (history_offset + 1) % HISTORY_COUNT;

	char overlay_text[32];
	snprintf(overlay_text, sizeof(overlay_text), "%.1f ms", current_ms);

	ImGui::PushStyleColor(ImGuiCol_PlotLines,
						  ImVec4(0.77f, 0.85f, 0.48f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.15f, 0.12f, 0.8f));

	ImGui::PlotLines("##frametime_plot",
					 ms_history,
					 HISTORY_COUNT,
					 history_offset,
					 overlay_text,
					 0.0f,
					 33.3f,
					 ImVec2(200.0f * scale, 50.0f * scale));

	ImGui::PopStyleColor(2);
}
