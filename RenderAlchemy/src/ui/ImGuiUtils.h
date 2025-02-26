#pragma once

#include <imgui.h>

namespace ImGuiUtils
{
	// Font indices for easy reference
	enum FontIndex
	{
		FONT_REGULAR = 0,
		FONT_MEDIUM = 1,
		FONT_LARGE = 2,
		FONT_ICONS = 3
	};

	// Initialize ImGui with custom fonts and styling
	void InitializeImGui(ImGuiIO& io);

	// Icon helper function - switches to icon font, draws icon, then restores font
	void Icon(const char* icon_literal);

	// Set up Gruvbox dark theme colors for ImGui
	void SetGruvboxTheme();
} // namespace ImGuiUtils
