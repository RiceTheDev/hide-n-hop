#include <raylib.h>
#include <raymath.h>
#include <math.h>

#include "mousestroke.h"
#include "theme.h"

ImVec2 mouse_delta_smooth = ImVec2(0, 0);
ImVec2 mouse_dot_offset = ImVec2(0, 0);

void ui_mousestroke()
{
    setup_theme_steam();

    ImGuiIO& io = ImGui::GetIO();

    const float size = 80.0f;
    const float radius = 4.0f;
    const float max_offset = size * 0.5f - radius - 2.0f;
    const float sensitivity = 1.5f;
    const float smoothing = 20.0f;
    const float return_speed = 5.0f;

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(
        ImVec2(
            viewport->WorkPos.x + viewport->WorkSize.x - size - 15.0f,
            viewport->WorkPos.y + 15.0f
        ),
        ImGuiCond_Always
    );

    ImGui::SetNextWindowSize(ImVec2(size, size), ImGuiCond_Always);

    ImGui::Begin(
        "##MouseStroke",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoInputs
    );

    const float dt = io.DeltaTime;
    const float alpha = 1.0f - expf(-smoothing * dt);
    const float return_alpha = 1.0f - expf(-return_speed * dt);

    Vector2 delta = GetMouseDelta();

    if (delta.x != 0.0f || delta.y != 0.0f)
    {
        mouse_dot_offset.x += delta.x * sensitivity;
        mouse_dot_offset.y += delta.y * sensitivity;
    }
    else
    {
        mouse_dot_offset.x += -mouse_dot_offset.x * return_alpha;
        mouse_dot_offset.y += -mouse_dot_offset.y * return_alpha;
    }

    mouse_dot_offset.x = Clamp(mouse_dot_offset.x, -max_offset, max_offset);
    mouse_dot_offset.y = Clamp(mouse_dot_offset.y, -max_offset, max_offset);

    mouse_delta_smooth.x +=
        (mouse_dot_offset.x - mouse_delta_smooth.x) * alpha;

    mouse_delta_smooth.y +=
        (mouse_dot_offset.y - mouse_delta_smooth.y) * alpha;

    ImVec2 center = ImGui::GetWindowPos();
    center.x += size * 0.5f;
    center.y += size * 0.5f;

    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(
            center.x + mouse_delta_smooth.x,
            center.y + mouse_delta_smooth.y
        ),
        radius,
        IM_COL32(255, 255, 255, 255),
        16
    );

    ImGui::End();
}
