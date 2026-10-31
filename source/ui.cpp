#include "../include/ui.h"
#include "../include/game.h"
#include "../include/player.h"
#include "../include/network.h"
#include "ui/fps_graph.h"
#include "ui/keystrokes.h"
#include "rlImGui/rlImGui.h"
#include "ui/mousestroke.h"
#include "ui/theme.h"
#include <cstdio>
#include <imgui.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

Texture2D menu_background;
float menu_bg_time = 0.0f;
bool settings = false;

ImFont *font_body;
ImFont *font_title;

static float handAirOffset = 0.0f;

void ui_init()
{
	rlImGuiSetup(true);
	ImGui::StyleColorsDark();
	ImGuiIO &io = ImGui::GetIO();

	font_body =
		io.Fonts->AddFontFromFileTTF("assets/fonts/DMsans.ttf", 22.0f);

	font_title =
		io.Fonts->AddFontFromFileTTF("assets/fonts/Outfit-Bold.ttf", 64.0f);

	menu_background = LoadTexture("assets/ui/bg_render.png");
}

void ui_update()
{
	BeginDrawing();
	const int w = GetScreenWidth();
	const int h = GetScreenHeight();

	ClearBackground(BLACK);

	if (menu_background.id != 0)
	{
		menu_bg_time += GetFrameTime();

		const float t = menu_bg_time;

		const float zoom = 1.06f + 0.06f * sinf(t * 0.35f);

		const float pan_x = sinf(t * 0.18f) * 0.035f;
		const float pan_y = cosf(t * 0.13f) * 0.018f;

		const float image_w = (float)menu_background.width;
		const float image_h = (float)menu_background.height;

		const float cover = fmaxf((float)w / image_w, (float)h / image_h);

		const float draw_w = image_w * cover * zoom;
		const float draw_h = image_h * cover * zoom;

		const float extra_x = draw_w - w;
		const float extra_y = draw_h - h;

		const float x = -extra_x * (0.5f + pan_x);
		const float y = -extra_y * (0.5f + pan_y);

		DrawTexturePro(menu_background,
					   (Rectangle){ 0, 0, image_w, image_h },
					   (Rectangle){ x, y, draw_w, draw_h },
					   (Vector2){ 0, 0 },
					   0.0f,
					   WHITE);
	}

	DrawRectangle(0, 0, w, h, (Color){ 0, 0, 0, 110 });
	rlImGuiBegin();

	float scale = fminf(w / 1280.0f, h / 720.0f);
	scale = fmaxf(0.65f, fminf(scale, 1.6f));

	const float left = w * 0.1f;
	const float title_y = h * 0.35f;

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2((float)w, (float)h));

	ImGui::Begin("##menu",
				 nullptr,
				 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
					 ImGuiWindowFlags_NoBackground |
					 ImGuiWindowFlags_NoSavedSettings |
					 ImGuiWindowFlags_NoBringToFrontOnFocus);

	if (font_title)
		ImGui::PushFont(font_title);

	ImGui::SetCursorPos(ImVec2(left, title_y));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
	ImGui::TextUnformatted("hide & hop");
	ImGui::PopStyleColor();

	ImGui::SameLine(0, 10.0f * scale);
	// ImGui::SetCursorPosY(ImGuiSetCursorPosY + 4.0f * scale);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.45f, 0.40f, 0.8f));
	ImGui::TextDisabled("v0.1");
	ImGui::PopStyleColor();

	if (font_title)
		ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(left, title_y + 90.0f * scale));

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.77f, 0.85f, 0.48f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
						  ImVec4(0.84f, 0.90f, 0.60f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive,
						  ImVec4(0.68f, 0.77f, 0.40f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.14f, 0.18f, 0.14f, 1.0f));

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
						ImVec2(24.0f * scale, 12.0f * scale));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f * scale);

	if (font_body)
		ImGui::PushFont(font_body);

	if (ImGui::Button("play"))
	{
		DisableCursor();

		Game_Init();
		game_state = GAME_INGAME;

	}

	ImGui::SameLine(0, 10.0f * scale);

	if (ImGui::Button("settings"))
	{
		settings = !settings;
	}

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(4);

	if (font_body)
		ImGui::PopFont();

	ImGui::End();

	if (settings)
        {
            struct ResPreset {
                int width;
                int height;
                const char* label;
            };

            static const ResPreset resolutions[] = {
                { 640, 480,   "640 x 480" },
                { 800, 600,   "800 x 600" },
                { 1280, 720,  "1280 x 720" },
                { 1600, 900,  "1600 x 900" },
                { 1920, 1080, "1920 x 1080" },
                { 2560, 1440, "2560 x 1440" }
            };
            static int selected_res = 2;

            struct FpsPreset {
                int fps;
                const char* label;
            };

            static const FpsPreset fps_limits[] = {
                { 30,  "30 FPS" },
                { 60,  "60 FPS" },
                { 120, "120 FPS" },
                { 144, "144 FPS" },
                { 240, "240 FPS" },
                { 0,   "Unlimited" }
            };
            static int selected_fps = 1;

            static bool vsync_enabled = false;
            static float master_volume = 1.0f;

            const float panel_w = 440.0f * scale;
            const float panel_h = 440.0f * scale;

            ImGui::SetNextWindowPos(ImVec2((w - panel_w) * 0.5f, (h - panel_h) * 0.5f), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(panel_w, panel_h));

            const ImVec4 accent_base    = ImVec4(0.77f, 0.85f, 0.48f, 1.0f);
            const ImVec4 accent_hover   = ImVec4(0.84f, 0.90f, 0.60f, 1.0f);
            const ImVec4 accent_active  = ImVec4(0.68f, 0.77f, 0.40f, 1.0f);
            const ImVec4 dark_text      = ImVec4(0.14f, 0.18f, 0.14f, 1.0f);
            const ImVec4 muted_text     = ImVec4(0.40f, 0.45f, 0.40f, 0.8f);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f * scale);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f * scale);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f * scale, 24.0f * scale));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f * scale, 8.0f * scale));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f * scale, 12.0f * scale));

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.10f, 0.08f, 0.94f));
            ImGui::PushStyleColor(ImGuiCol_Border, accent_base);
            ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.08f, 0.10f, 0.08f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.08f, 0.10f, 0.08f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.16f, 0.20f, 0.16f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.22f, 0.28f, 0.22f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_CheckMark, accent_base);
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, accent_base);
            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, accent_active);

            ImGui::PushStyleColor(ImGuiCol_Button, accent_base);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, accent_hover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, accent_active);
            ImGui::PushStyleColor(ImGuiCol_Header, accent_base);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, accent_hover);

            if (font_body)
                ImGui::PushFont(font_body);

            ImGui::Begin("settings",
                         &settings,
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoSavedSettings);

            bool is_fullscreen = IsWindowFullscreen();
            if (ImGui::Checkbox(" fullscreen", &is_fullscreen))
            {
                ToggleFullscreen();
            }

            if (is_fullscreen)
            {
                ImGui::BeginDisabled();
            }

            if (ImGui::BeginCombo(" resolution", resolutions[selected_res].label))
            {
                for (int i = 0; i < 6; i++)
                {
                    const bool is_selected = (selected_res == i);

                    if (is_selected)
                        ImGui::PushStyleColor(ImGuiCol_Text, dark_text);

                    if (ImGui::Selectable(resolutions[i].label, is_selected))
                    {
                        selected_res = i;
                        SetWindowSize(resolutions[i].width, resolutions[i].height);
                    }

                    if (is_selected)
                    {
                        ImGui::PopStyleColor();
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (is_fullscreen)
            {
                ImGui::EndDisabled();
                ImGui::PushStyleColor(ImGuiCol_Text, muted_text);
                ImGui::TextDisabled("(disable fullscreen to change window size)");
                ImGui::PopStyleColor();
            }

            ImGui::Separator();

            if (ImGui::Checkbox(" vsync", &vsync_enabled))
            {
                if (vsync_enabled)
                {
                    SetWindowState(FLAG_VSYNC_HINT);
                }
                else
                {
                    ClearWindowState(FLAG_VSYNC_HINT);
                }
            }

            if (vsync_enabled)
            {
                ImGui::BeginDisabled();
            }

            if (ImGui::BeginCombo(" max fps", fps_limits[selected_fps].label))
            {
                for (int i = 0; i < 6; i++)
                {
                    const bool is_selected = (selected_fps == i);

                    if (is_selected)
                        ImGui::PushStyleColor(ImGuiCol_Text, dark_text);

                    if (ImGui::Selectable(fps_limits[i].label, is_selected))
                    {
                        selected_fps = i;
                        SetTargetFPS(fps_limits[i].fps);
                    }

                    if (is_selected)
                    {
                        ImGui::PopStyleColor();
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (vsync_enabled)
            {
                ImGui::EndDisabled();
            }

            ImGui::Separator();

            static int volume_percent = 100;

            if (ImGui::SliderInt(" volume", &volume_percent, 0, 100, "%d%%"))
            {
                SetMasterVolume((float)volume_percent / 100.0f);
            }
            ImGui::Separator();

            ImGui::PushStyleColor(ImGuiCol_Text, dark_text);
            if (ImGui::Button("back", ImVec2(100.0f * scale, 34.0f * scale)))
            {
                settings = false;
            }
            ImGui::PopStyleColor();

            if (font_body)
                ImGui::PopFont();

            ImGui::End();

            ImGui::PopStyleColor(15);
            ImGui::PopStyleVar(5);
        }

	rlImGuiEnd();

	EndDrawing();
}

void ui_render_hand() {
	float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    float handSize = sh * 0.45f;

    float delta = GetFrameTime();

    float targetAir = Clamp(local_player.vel.y * 2.0f, -15.0f, 15.0f);
    handAirOffset = Lerp(handAirOffset, targetAir, 12.0f * delta);

    float bobX = sinf(headTimer * PI) * (sh * 0.015f) * walkLerp;
    float bobY = fabsf(cosf(headTimer * PI)) * (sh * 0.02f) * walkLerp;

    float jumpY = handAirOffset * (sh * 0.0018f);
    float jumpRot = handAirOffset * 0.25f;
    float minBaseY = sh - (handSize * 0.5f) + (sh * 0.03f);
    float baseY = fmaxf(sh - (handSize * 0.5f) + bobY + jumpY + (sh * 0.08f), minBaseY);

	Vector2 origin = { handSize * 0.5f, handSize * 0.5f };

    bool has_bomb = (my_local_id != -1 && my_local_id == current_state.it_player_id);

    if (has_bomb)
    {
	    float bombHeight = handSize;
        float bombWidth  = handSize * 2.0f;

        Vector2 originBomb = { bombWidth * 0.5f, bombHeight * 0.5f };

        Rectangle srcBomb = { 0.0f, 0.0f, (float)bomb_hand_tex.width, (float)bomb_hand_tex.height };
        Rectangle dstBomb = {
            (sw * 0.5f) + bobX,
            baseY,
            bombWidth,
            bombHeight
        };

        if (current_state.potato_timer < 5.0f && current_state.player_count >= 2)
        {
            dstBomb.x += (float)GetRandomValue(-3, 3);
            dstBomb.y += (float)GetRandomValue(-3, 3);
        }

        DrawTexturePro(bomb_hand_tex, srcBomb, dstBomb, originBomb, jumpRot * 0.5f, WHITE);
    }
    else
    {
        Rectangle srcRight = { 0.0f, 0.0f, (float)hand_tex.width, (float)hand_tex.height };
        Rectangle srcLeft  = { 0.0f, 0.0f, -(float)hand_tex.width, (float)hand_tex.height };

        Rectangle dstLeft = {
            (sw * 0.08f) + (handSize * 0.5f) - bobX,
            baseY,
            handSize,
            handSize
        };

        Rectangle dstRight = {
            sw - (sw * 0.08f) - (handSize * 0.5f) + bobX,
            baseY,
            handSize,
            handSize
        };

        DrawTexturePro(hand_tex, srcLeft, dstLeft, origin, -jumpRot, WHITE);
        DrawTexturePro(hand_tex, srcRight, dstRight, origin, jumpRot, WHITE);
    }
}

void ui_debug_menu() {
	setup_theme_steam();

	rlImGuiBegin();
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 0), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoInputs;

    ImGui::Begin("debug view", NULL, flags);

    ImVec4 timerColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    if (current_state.potato_timer < 5.0f)
        timerColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    else if (current_state.potato_timer < 12.0f)
        timerColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);

    if (connected)
    {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "status: connected, id: %d", my_local_id);
    }
    else
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "status: connecting...");
    }

    float currentSpeed = Vector2Length((Vector2){ local_player.vel.x, local_player.vel.z });
    ImGui::Text("speed: %.1f u/s", currentSpeed);

    if (current_state.player_count >= 2)
    {
        ImGui::TextColored(timerColor, "time left: %.1fs", current_state.potato_timer);

        float progress = Clamp(current_state.potato_timer / 25.0f, 0.0f, 1.0f);
        ImGui::ProgressBar(progress, ImVec2(-1, 15), "");

        if (my_local_id != -1 && my_local_id == current_state.it_player_id)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "the potato is all yours, baby!");
        }
        else if (current_state.it_player_id != -1)
        {
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "potato is with player %d", current_state.it_player_id);
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "waiting for more players...");
    }


    ui_keystroke();
    ui_mousestroke();

    ImGui::Text("\n");

    ImGui::Text("frame time graph");
    ui_frametime(1.0f);
    ImGui::End();
    rlImGuiEnd();
}

void ui_shutdown()
{
	UnloadTexture(menu_background);
	rlImGuiShutdown();
}
