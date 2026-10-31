#pragma once

#include "raylib.h"
#include "player_physics.h"
#include "spritemanager.h"

#define STAND_HEIGHT 1.25f
#define CROUCH_HEIGHT 0.65f

extern Texture2D player_sprite;
extern Texture2D hand_tex;
extern Texture2D bomb_hand_tex;
extern Material player_material;
extern Mesh player_mesh;
extern Camera cam;
extern CameraMode cam_mode;
extern Vector2 sens;
extern PlayerPhysicsState local_player;
extern Vector2 look;
extern float headLerp;
extern float headTimer;
extern float walkLerp;
extern bool crouch;
extern Vector2 move;

void InitPlayer(Vector3 start_pos);
void UpdatePlayer(float rot, Vector2 move, bool jump, bool crouch);
void UpdatePlayerCamera(float delta);

#pragma endregion
