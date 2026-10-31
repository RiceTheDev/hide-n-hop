#pragma once
#include <imgui.h>
typedef enum
{
	GAME_MENU,
	GAME_INGAME,
	GAME_PAUSE
} State;

extern State game_state;

// ui globals
extern ImFont *font_body;
extern ImFont *font_title;

void ui_init();
void ui_update();
void ui_debug_menu();
void ui_render_hand();
void ui_shutdown();
