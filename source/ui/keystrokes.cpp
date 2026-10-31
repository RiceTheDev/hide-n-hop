#include <imgui.h>
#include <stdio.h>
#include "keystrokes.h"
#include "theme.h"

static void draw_keycap(
    const char* id,
    ImVec2 pos,
    ImVec2 size,
    const char* label,
    bool pressed)
{
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);

    char window_id[64];
    snprintf(window_id, sizeof(window_id), "##Key_%s", id);

    ImGui::Begin(
        window_id,
        nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoInputs
    );

    ImVec2 text_size = ImGui::CalcTextSize(label);

    ImGui::SetCursorPos(ImVec2(
        (size.x - text_size.x) * 0.5f,
        (size.y - text_size.y) * 0.5f
    ));

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        pressed
            ? IM_COL32(28, 30, 36, 255)
            : IM_COL32(240, 242, 245, 255)
    );

    ImGui::TextUnformatted(label);

    ImGui::PopStyleColor();
    ImGui::End();
}

void ui_keystroke()
{
	setup_theme_steam();

    // if (font_body)
    //     ImGui::PushFont(font_body);

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    const ImVec2 origin(
        viewport->WorkPos.x + viewport->WorkSize.x - 15.0f - 164.0f,
        viewport->WorkPos.y + viewport->WorkSize.y - 15.0f - 148.0f
    );

    bool w = ImGui::IsKeyDown(ImGuiKey_W);
    bool a = ImGui::IsKeyDown(ImGuiKey_A);
    bool s = ImGui::IsKeyDown(ImGuiKey_S);
    bool d = ImGui::IsKeyDown(ImGuiKey_D);

    bool ctrl =
        ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
        ImGui::IsKeyDown(ImGuiKey_RightCtrl);

    bool space = ImGui::IsKeyDown(ImGuiKey_Space);

    draw_keycap("W", ImVec2(origin.x + 62, origin.y + 8),
        ImVec2(40, 40), "W", w);

    draw_keycap("A", ImVec2(origin.x + 14, origin.y + 56),
        ImVec2(42, 42), "A", a);

    draw_keycap("S", ImVec2(origin.x + 62, origin.y + 56),
        ImVec2(40, 42), "S", s);

    draw_keycap("D", ImVec2(origin.x + 108, origin.y + 56),
        ImVec2(42, 42), "D", d);

    draw_keycap("Ctrl", ImVec2(origin.x + 14, origin.y + 106),
        ImVec2(56, 30), "ctrl", ctrl);

    draw_keycap("Space", ImVec2(origin.x + 74, origin.y + 106),
        ImVec2(76, 30), "space", space);

    // if (font_body)
    //     ImGui::PopFont();
}
