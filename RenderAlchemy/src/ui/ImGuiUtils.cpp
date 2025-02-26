#include "ImGuiUtils.h"

#include <imgui_internal.h>

// Include embedded font data
#include "../fonts/IconsLucide.h"
#include "../fonts/RobotoBold.h"
#include "../fonts/RobotoRegular.h"

namespace ImGuiUtils
{

	// Initialize ImGui with custom fonts and styling
	void InitializeImGui(ImGuiIO& io)
	{
		// Load and configure fonts
		ImFontConfig fontConfig;
		fontConfig.PixelSnapH = true;
		fontConfig.OversampleH = 3;
		fontConfig.OversampleV = 3;

		// Set up font sizes
		float baseFontSize = 16.0f;
		float mediumFontSize = 18.0f;
		float largeFontSize = 24.0f;
		float iconFontSize = 18.0f; // Size for icons

		// Clear default font
		io.Fonts->Clear();

		// Default font (Roboto Regular)
		io.Fonts->AddFontFromMemoryCompressedTTF(roboto_regular_compressed_data, roboto_regular_compressed_size, baseFontSize, &fontConfig);

		// Medium size font
		io.Fonts->AddFontFromMemoryCompressedTTF(roboto_regular_compressed_data, roboto_regular_compressed_size, mediumFontSize, &fontConfig);

		// Large font (bold)
		io.Fonts->AddFontFromMemoryCompressedTTF(roboto_bold_compressed_data, roboto_bold_compressed_size, largeFontSize, &fontConfig);

		// Load icons font (will be at index FONT_ICONS = 3)
		static const ImWchar icons_ranges[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = false; // Changed to false so it's a standalone font
		icons_config.PixelSnapH = true;
		icons_config.OversampleH = 2;
		icons_config.OversampleV = 2;

		// Ill fix this path up to be relative later its too late and I have work in the morning
		io.Fonts->AddFontFromFileTTF("C:\\Users\\alexm\\Source\\repos\\RenderAlchemy\\RenderAlchemy\\src\\fonts\\lucide.ttf", iconFontSize, &icons_config, icons_ranges);

		// Build font atlas
		io.Fonts->Build();
	}

	// Icon helper function - safely switches to icon font, draws icon, then restores font
	void Icon(const char* icon_literal)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImFont* iconFont = nullptr;

		// Safety check to make sure we have enough fonts and FONT_ICONS is valid
		if (io.Fonts && FONT_ICONS < io.Fonts->Fonts.Size)
		{
			iconFont = io.Fonts->Fonts[FONT_ICONS];
		}

		if (iconFont)
		{
			ImGui::PushFont(iconFont);
			ImGui::Text("%s", icon_literal);
			ImGui::PopFont();
			ImGui::SameLine();
		}
		else
		{
			// Fallback if icon font is not available
			ImGui::Text("?");
			ImGui::SameLine();
		}
	}

	// Set up Gruvbox dark theme colors for ImGui
	void SetGruvboxTheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		// Set up some style variables
		style.WindowRounding = 6.0f;
		style.FrameRounding = 4.0f;
		style.PopupRounding = 4.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabRounding = 3.0f;
		style.TabRounding = 4.0f;

		// Set up item spacing and padding
		style.FramePadding = ImVec2(5, 3);
		style.ItemSpacing = ImVec2(6, 5);
		style.ItemInnerSpacing = ImVec2(6, 4);

		// Alpha values
		style.Alpha = 1.0f;
		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.PopupBorderSize = 1.0f;

		// Gruvbox Dark color scheme
		ImVec4* colors = style.Colors;

		// Gruvbox Dark Palette
		const ImVec4 bg0_h = ImVec4(0.18f, 0.16f, 0.15f, 1.00f); // #1d2021
		const ImVec4 bg0 = ImVec4(0.21f, 0.19f, 0.18f, 1.00f);   // #282828
		const ImVec4 bg1 = ImVec4(0.25f, 0.22f, 0.21f, 1.00f);   // #3c3836
		const ImVec4 bg2 = ImVec4(0.30f, 0.27f, 0.23f, 1.00f);   // #504945
		const ImVec4 bg3 = ImVec4(0.37f, 0.33f, 0.29f, 1.00f);   // #665c54
		const ImVec4 bg4 = ImVec4(0.45f, 0.39f, 0.33f, 1.00f);   // #7c6f64

		const ImVec4 fg0 = ImVec4(0.95f, 0.84f, 0.64f, 1.00f); // #fbf1c7
		const ImVec4 fg1 = ImVec4(0.84f, 0.76f, 0.58f, 1.00f); // #ebdbb2
		const ImVec4 fg2 = ImVec4(0.73f, 0.66f, 0.51f, 1.00f); // #d5c4a1
		const ImVec4 fg3 = ImVec4(0.62f, 0.56f, 0.44f, 1.00f); // #bdae93
		const ImVec4 fg4 = ImVec4(0.52f, 0.45f, 0.36f, 1.00f); // #a89984

		const ImVec4 red = ImVec4(0.80f, 0.14f, 0.11f, 1.00f);    // #cc241d
		const ImVec4 green = ImVec4(0.60f, 0.59f, 0.10f, 1.00f);  // #98971a
		const ImVec4 yellow = ImVec4(0.84f, 0.60f, 0.13f, 1.00f); // #d79921
		const ImVec4 blue = ImVec4(0.27f, 0.52f, 0.53f, 1.00f);   // #458588
		const ImVec4 purple = ImVec4(0.69f, 0.38f, 0.66f, 1.00f); // #b16286
		const ImVec4 aqua = ImVec4(0.42f, 0.65f, 0.40f, 1.00f);   // #689d6a
		const ImVec4 orange = ImVec4(0.92f, 0.44f, 0.20f, 1.00f); // #d65d0e

		// Bright versions
		const ImVec4 brightRed = ImVec4(0.98f, 0.21f, 0.16f, 1.00f);    // #fb4934
		const ImVec4 brightGreen = ImVec4(0.72f, 0.73f, 0.15f, 1.00f);  // #b8bb26
		const ImVec4 brightYellow = ImVec4(0.98f, 0.75f, 0.18f, 1.00f); // #fabd2f
		const ImVec4 brightBlue = ImVec4(0.31f, 0.65f, 0.66f, 1.00f);   // #83a598
		const ImVec4 brightPurple = ImVec4(0.83f, 0.46f, 0.78f, 1.00f); // #d3869b
		const ImVec4 brightAqua = ImVec4(0.54f, 0.76f, 0.49f, 1.00f);   // #8ec07c
		const ImVec4 brightOrange = ImVec4(0.98f, 0.59f, 0.20f, 1.00f); // #fe8019

		// Main
		colors[ImGuiCol_WindowBg] = bg0;
		colors[ImGuiCol_ChildBg] = bg0_h;
		colors[ImGuiCol_PopupBg] = bg0;
		colors[ImGuiCol_Border] = bg3;
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = bg1;
		colors[ImGuiCol_FrameBgHovered] = bg2;
		colors[ImGuiCol_FrameBgActive] = bg3;

		// Text
		colors[ImGuiCol_Text] = fg1;
		colors[ImGuiCol_TextDisabled] = fg4;

		// Tabs, titles, headers
		colors[ImGuiCol_MenuBarBg] = bg0_h;
		colors[ImGuiCol_TitleBg] = bg0_h;
		colors[ImGuiCol_TitleBgActive] = bg1;
		colors[ImGuiCol_TitleBgCollapsed] = bg0_h;
		colors[ImGuiCol_Tab] = bg1;
		colors[ImGuiCol_TabHovered] = yellow;
		colors[ImGuiCol_TabActive] = orange;
		colors[ImGuiCol_TabUnfocused] = bg1;
		colors[ImGuiCol_TabUnfocusedActive] = bg2;

		// Headers
		colors[ImGuiCol_Header] = bg2;
		colors[ImGuiCol_HeaderHovered] = bg3;
		colors[ImGuiCol_HeaderActive] = bg4;

		// Buttons
		colors[ImGuiCol_Button] = bg1;
		colors[ImGuiCol_ButtonHovered] = brightOrange;
		colors[ImGuiCol_ButtonActive] = orange;

		// Checkmarks, radio buttons
		colors[ImGuiCol_CheckMark] = brightYellow;

		// Sliders, drag elements
		colors[ImGuiCol_SliderGrab] = bg4;
		colors[ImGuiCol_SliderGrabActive] = brightOrange;
		colors[ImGuiCol_DragDropTarget] = yellow;

		// Scrollbars
		colors[ImGuiCol_ScrollbarBg] = bg0_h;
		colors[ImGuiCol_ScrollbarGrab] = bg3;
		colors[ImGuiCol_ScrollbarGrabHovered] = bg4;
		colors[ImGuiCol_ScrollbarGrabActive] = fg4;

		// Separators
		colors[ImGuiCol_Separator] = bg2;
		colors[ImGuiCol_SeparatorHovered] = bg3;
		colors[ImGuiCol_SeparatorActive] = bg4;

		// Selection
		colors[ImGuiCol_ResizeGrip] = bg3;
		colors[ImGuiCol_ResizeGripHovered] = bg4;
		colors[ImGuiCol_ResizeGripActive] = brightOrange;

		// Plots
		colors[ImGuiCol_PlotLines] = fg3;
		colors[ImGuiCol_PlotLinesHovered] = brightOrange;
		colors[ImGuiCol_PlotHistogram] = blue;
		colors[ImGuiCol_PlotHistogramHovered] = brightBlue;

		// Tables
		colors[ImGuiCol_TableHeaderBg] = bg1;
		colors[ImGuiCol_TableBorderStrong] = bg3;
		colors[ImGuiCol_TableBorderLight] = bg2;
		colors[ImGuiCol_TableRowBg] = bg0_h;
		colors[ImGuiCol_TableRowBgAlt] = bg1;

		// Navigation, selection
		colors[ImGuiCol_NavHighlight] = brightYellow;
		colors[ImGuiCol_NavWindowingHighlight] = yellow;
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.65f);

		// Combo boxes
		colors[ImGuiCol_TextSelectedBg] = bg3;
	}
} // namespace ImGuiUtils
