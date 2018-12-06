
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imguiRenderFrameEx.h"

namespace ImGui {
	void RenderFrameEx(ImVec2 p_min, ImVec2 p_max, bool border, float rounding, float thickness)
	{
		ImGuiWindow* window = GetCurrentWindow();

		if (border)
		{
			window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, 15, thickness);
			window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, 15, thickness);
		}
	}

} // IMGUI_API
