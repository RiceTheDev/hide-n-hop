#include "../include/player.h"
#include "../include/map.h"
#include "../include/audiomanager.h"
#include "../include/death.h"
#include "raymath.h"
#include <math.h>

Vector2 sens = { 0.002f, 0.002f };
PlayerPhysicsState local_player = { 0 };
Vector2 look = { 0 };
float headLerp = STAND_HEIGHT;
float headTimer = 0.0f;
float walkLerp = 0.0f;
bool crouch = false;
Vector2 move = { 0 };
CameraMode cam_mode = CAMERA_FIRST_PERSON;
Texture2D hand_tex;
Texture2D bomb_hand_tex;

static float physics_accumulator = 0.0f;

void InitPlayer(Vector3 start_pos)
{
	local_player.pos = start_pos;
	local_player.vel = (Vector3){ 0 };
	local_player.grounded = true;
	local_player.is_jumping = false;
}

void UpdatePlayer(float rot,
				  Vector2 move_input,
				  bool jump_input,
				  bool crouch_input)
{
	float frame_time = GetFrameTime();
	if (frame_time > 0.1f)
		frame_time = 0.1f;

	physics_accumulator += frame_time;

	PlayerInputFrame current_frame = { .move = move_input,
									   .yaw = rot,
									   .jump = jump_input,
									   .crouch = crouch_input };

	bool was_grounded = local_player.grounded;

	while (physics_accumulator >= FIXED_DT)
	{
		if (local_player.grounded && jump_input)
		{
			jump_player(true);
		}

		StepPlayerPhysics(&local_player, &current_frame, FIXED_DT);

		if (!was_grounded && local_player.grounded)
		{
			landing_player();
		}
		was_grounded = local_player.grounded;

		physics_accumulator -= FIXED_DT;
	}
}

void UpdatePlayerCamera(float delta)
{
	if (death_seq.is_dying)
		return;

	if (local_player.grounded && Vector2LengthSqr(move) > 0.01f)
	{
		headTimer += delta * 3.5f;
		walkLerp = Lerp(walkLerp, 1.0f, 10.0f * delta);
	}
	else
	{
		walkLerp = Lerp(walkLerp, 0.0f, 10.0f * delta);
	}

	headLerp =
		Lerp(headLerp, crouch ? CROUCH_HEIGHT : STAND_HEIGHT, 15.0f * delta);

	cam.position = (Vector3){ local_player.pos.x,
							  local_player.pos.y + 0.5f + headLerp,
							  local_player.pos.z };

	Vector3 globalUp = { 0.0f, 1.0f, 0.0f };

	Vector3 yaw = Vector3RotateByAxisAngle(
		(Vector3){ 0.0f, 0.0f, -1.0f }, globalUp, look.x);

	Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, globalUp));
	Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, -look.y);

	float headSin = sinf(headTimer * PI);
	float headCos = cosf(headTimer * PI);

	Vector3 bobbing = Vector3Scale(right, headSin * 0.08f);
	bobbing.y = fabsf(headCos * 0.12f);

	cam.position = Vector3Add(cam.position, Vector3Scale(bobbing, walkLerp));
	cam.up =
		Vector3RotateByAxisAngle(globalUp, pitch, headSin * 0.015f * walkLerp);
	cam.target = Vector3Add(cam.position, pitch);
}
