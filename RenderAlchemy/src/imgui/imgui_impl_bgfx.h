
#pragma once

#include "imgui.h"
#include "bgfx/bgfx.h"

bool ImGui_ImplBgfx_Init(bgfx::ViewId viewId);
void ImGui_ImplBgfx_Shutdown();
void ImGui_ImplBgfx_RenderDrawData(ImDrawData* draw_data);
void ImGui_ImplBgfx_NewFrame();
