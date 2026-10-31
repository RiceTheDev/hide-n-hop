#ifndef PLAYER_PHYSICS_H
#define PLAYER_PHYSICS_H

#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>

#define PHYSICS_TICK_RATE 60.0f
#define FIXED_DT (1.0f / PHYSICS_TICK_RATE)

#define GRAVITY 22.0f
#define JUMP_FORCE 8.2f
#define MAX_SPEED 15.0f
#define CROUCH_SPEED 4.5f
#define MAX_AIR_SPEED 1.15f
#define GROUND_ACCEL 10.0f
#define AIR_ACCEL 120.0f
#define FRICTION 6.0f
#define STOP_SPEED 3.0f
#define SURF_SLOPE_NORMAL 0.7f

typedef struct
{
	Vector3 pos;
	Vector3 vel;
	bool grounded;
	bool is_jumping;
} PlayerPhysicsState;

typedef struct
{
	Vector2 move;
	float yaw;
	bool jump;
	bool crouch;
} PlayerInputFrame;

void StepPlayerPhysics(PlayerPhysicsState *body,
					   const PlayerInputFrame *input,
					   float dt);

#endif
