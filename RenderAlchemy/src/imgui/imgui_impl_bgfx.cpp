#include "imgui_impl_bgfx.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/math.h"
#include <vector>
#include "vs_imgui_bin.h"
#include "fs_imgui_bin.h"


// Global variables
static bgfx::ViewId g_ViewId;
static bgfx::VertexLayout g_VertexLayout;
static bgfx::ProgramHandle g_ShaderProgram;
static bgfx::UniformHandle g_UniformTexture;
static bgfx::TextureHandle g_FontTexture;

struct ImGuiVertex {
    float pos[2];
    float uv[2];
    uint32_t col;
};

bool ImGui_ImplBgfx_Init(bgfx::ViewId viewId) {
    g_ViewId = viewId;

    // Create vertex layout
    g_VertexLayout.begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();

    // Load shaders
    const bgfx::Memory* vs = bgfx::copy(vs_imgui_bin_h, sizeof(vs_imgui_bin_h));
    const bgfx::Memory* fs = bgfx::copy(fs_imgui_bin_h, sizeof(fs_imgui_bin_h));
    g_ShaderProgram = bgfx::createProgram(bgfx::createShader(vs), bgfx::createShader(fs), true);

    // Create uniform
    g_UniformTexture = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

    // Create font texture
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    g_FontTexture = bgfx::createTexture2D(
        (uint16_t)width, (uint16_t)height, false, 1, bgfx::TextureFormat::BGRA8,
        0, bgfx::copy(pixels, width * height * 4)
    );
    io.Fonts->TexID = (ImTextureID)(uintptr_t)g_FontTexture.idx;

    // Assert valid DisplaySize
	io.DisplaySize = ImVec2(1280, 720);
    IM_ASSERT(io.DisplaySize.x >= 0.0f && io.DisplaySize.y >= 0.0f && "Invalid DisplaySize value!");

    // Initialize projection matrix
    float L = 0.0f;
    float R = io.DisplaySize.x;
    float T = 0.0f;
    float B = io.DisplaySize.y;
    float ortho[16] = {
        2.0f / (R - L), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (T - B), 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        (R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f,
    };
    bgfx::setViewTransform(g_ViewId, nullptr, ortho);

    return true;
}

void ImGui_ImplBgfx_Shutdown() {
    bgfx::destroy(g_FontTexture);
    bgfx::destroy(g_UniformTexture);
    bgfx::destroy(g_ShaderProgram);
}

void ImGui_ImplBgfx_RenderDrawData(ImDrawData* draw_data) {
    // Avoid rendering when minimized
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    const bgfx::Caps* caps = bgfx::getCaps();
    const float width = draw_data->DisplaySize.x;
    const float height = draw_data->DisplaySize.y;

    // Setup orthographic projection matrix
    float ortho[16];
    bx::mtxOrtho(ortho, 0.0f, width, height, 0.0f, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);

    bgfx::setViewTransform(g_ViewId, NULL, ortho);
    bgfx::setViewRect(g_ViewId, 0, 0, uint16_t(width), uint16_t(height));

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        // Create vertex and index buffers
        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;
        uint32_t numVertices = (uint32_t)cmd_list->VtxBuffer.Size;
        uint32_t numIndices = (uint32_t)cmd_list->IdxBuffer.Size;

        if (!bgfx::getAvailTransientVertexBuffer(numVertices, g_VertexLayout) || !bgfx::getAvailTransientIndexBuffer(numIndices))
            return;

        bgfx::allocTransientVertexBuffer(&tvb, numVertices, g_VertexLayout);
        bgfx::allocTransientIndexBuffer(&tib, numIndices);

        ImGuiVertex* verts = (ImGuiVertex*)tvb.data;
        memcpy(verts, cmd_list->VtxBuffer.Data, numVertices * sizeof(ImGuiVertex));
        memcpy(tib.data, cmd_list->IdxBuffer.Data, numIndices * sizeof(ImDrawIdx));

        // Render command lists
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                const uint16_t xx = uint16_t(bx::max(pcmd->ClipRect.x, 0.0f));
                const uint16_t yy = uint16_t(bx::max(pcmd->ClipRect.y, 0.0f));
                const uint16_t ww = uint16_t(bx::min(pcmd->ClipRect.z, 65535.0f) - xx);
                const uint16_t hh = uint16_t(bx::min(pcmd->ClipRect.w, 65535.0f) - yy);

                bgfx::setScissor(xx, yy, ww, hh);
                bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA);
                bgfx::setTexture(0, g_UniformTexture, g_FontTexture);
                bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
                bgfx::setIndexBuffer(&tib, (uint32_t)idx_buffer_offset, pcmd->ElemCount);
                bgfx::submit(g_ViewId, g_ShaderProgram);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }
}

void ImGui_ImplBgfx_NewFrame() {
    // No specific actions needed for bgfx in new frame
}
