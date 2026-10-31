#include "../include/game.h"
#include "../include/network.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/sky.h"
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

Camera cam;
Shader explosion_shader;
int progressLoc;

Shader sprite_shader;
int uvScaleLoc = -1;
int uvOffsetLoc = -1;

void Game_Init()
{
	if (!Net_Init("100.72.78.104", PORT))
		return;

	if (!Map_Load("assets/maps/map.glb", 0, "assets/maps/map_col.glb"))
	{
		TraceLog(LOG_WARNING,
				 "warning: couldnt load the custom map, (check files).");
	}

	explosion_shader = LoadShader("assets/shaders/explosion.vs",
								  "assets/shaders/explosion.fs");
	progressLoc = GetShaderLocation(explosion_shader, "progress");

	sprite_shader = LoadShader("assets/shaders/sprite_mesh.vs",
							   "assets/shaders/sprite_mesh.fs");

	uvScaleLoc = GetShaderLocation(sprite_shader, "uvScale");
	uvOffsetLoc = GetShaderLocation(sprite_shader, "uvOffset");

	int player_sprite_id =
		SpriteLoad("assets/player/player_sprite.png", 2, 0.1f);

	player_sprite = sprite_list[player_sprite_id].texture;
	player_mesh = GenMeshCube(1.5f, 2.0f, 0.001f);
	player_material = LoadMaterialDefault();
	player_material.shader = sprite_shader;
	SetMaterialTexture(&player_material, MATERIAL_MAP_DIFFUSE, player_sprite);

	// player_material.shader = Map_GetShader();

	hand_tex = LoadTexture("assets/player/hand.png");
	SetTextureFilter(hand_tex, TEXTURE_FILTER_POINT);
	SetTextureWrap(hand_tex, TEXTURE_WRAP_CLAMP);

	bomb_hand_tex = LoadTexture("assets/player/bomb_hand.png");
	SetTextureFilter(bomb_hand_tex, TEXTURE_FILTER_POINT);
	SetTextureWrap(bomb_hand_tex, TEXTURE_WRAP_CLAMP);

	cam.position = (Vector3){ 0, 0, 0 };
	cam.fovy = 90.0f;
	cam.projection = CAMERA_PERSPECTIVE;
	cam_mode = CAMERA_ORBITAL;
}
