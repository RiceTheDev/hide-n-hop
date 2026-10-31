#include "../include/player_physics.h"
#include "../include/map.h"
#include "raymath.h"
#include <math.h>

static Vector3 ClipVelocity(Vector3 in, Vector3 normal, float overbounce)
{
	float backoff = Vector3DotProduct(in, normal) * overbounce;
	if (backoff >= 0.0f)
		return in;

	Vector3 change = Vector3Scale(normal, backoff);
	return Vector3Subtract(in, change);
}

static void ApplyFriction(PlayerPhysicsState *body, float delta)
{
	float speed = sqrtf(body->vel.x * body->vel.x + body->vel.z * body->vel.z);
	if (speed < 0.001f)
	{
		body->vel.x = 0.0f;
		body->vel.z = 0.0f;
		return;
	}

	float control = (speed < STOP_SPEED) ? STOP_SPEED : speed;
	float drop = control * FRICTION * delta;

	float newSpeed = speed - drop;
	if (newSpeed < 0.0f)
		newSpeed = 0.0f;
	newSpeed /= speed;

	body->vel.x *= newSpeed;
	body->vel.z *= newSpeed;
}

static void Accelerate(PlayerPhysicsState *body,
					   Vector3 wishDir,
					   float wishSpeed,
					   float accel,
					   float delta,
					   bool inAir)
{
	float wishSpd = inAir ? fminf(wishSpeed, MAX_AIR_SPEED) : wishSpeed;
	float currentSpeed = body->vel.x * wishDir.x + body->vel.z * wishDir.z;
	float addSpeed = wishSpd - currentSpeed;

	if (addSpeed <= 0.0f)
		return;

	float accelSpeed = accel * wishSpeed * delta;
	if (accelSpeed > addSpeed)
		accelSpeed = addSpeed;

	body->vel.x += accelSpeed * wishDir.x;
	body->vel.z += accelSpeed * wishDir.z;
}

void StepPlayerPhysics(PlayerPhysicsState *body,
					   const PlayerInputFrame *input,
					   float dt)
{
	bool was_grounded = body->grounded;

	if (body->grounded && input->jump)
	{
		body->vel.y = JUMP_FORCE;
		body->grounded = false;
		body->is_jumping = true;
	}

	if (body->vel.y <= 0.0f)
	{
		body->is_jumping = false;
	}

	if (body->grounded)
	{
		ApplyFriction(body, dt);
	}
	else
	{
		body->vel.y -= GRAVITY * 0.5f * dt;
	}

	Vector3 front = { sinf(input->yaw), 0.0f, cosf(input->yaw) };
	Vector3 right = { cosf(-input->yaw), 0.0f, sinf(-input->yaw) };
	Vector3 wishDir = { input->move.x * right.x + input->move.y * front.x,
						0.0f,
						input->move.x * right.z + input->move.y * front.z };

	if (Vector3LengthSqr(wishDir) > 0.0f)
		wishDir = Vector3Normalize(wishDir);

	float targetSpeed = input->crouch ? CROUCH_SPEED : MAX_SPEED;
	if (body->grounded)
		Accelerate(body, wishDir, targetSpeed, GROUND_ACCEL, dt, false);
	else
		Accelerate(body, wishDir, targetSpeed, AIR_ACCEL, dt, true);

	body->pos = Vector3Add(body->pos, Vector3Scale(body->vel, dt));

	float playerHeight = input->crouch ? 1.2f : 1.8f;

	for (int bump = 0; bump < 3; bump++)
	{
		Vector3 wallNormal = { 0 };
		float pushDist = 0.0f;
		if (Map_CheckWallNormal(
				body->pos, 0.45f, playerHeight, &wallNormal, &pushDist))
		{
			body->pos = Vector3Add(body->pos,
								   Vector3Scale(wallNormal, pushDist + 0.001f));
			body->vel = ClipVelocity(body->vel, wallNormal, 1.0f);
		}
		else
		{
			break;
		}
	}

	bool hit_ground = false;
	Vector3 groundNormal = { 0.0f, 1.0f, 0.0f };
	float ground_y = Map_GetGroundInfo(body->pos, &hit_ground, &groundNormal);

	if (hit_ground)
	{
		if (body->is_jumping)
		{
			if (body->pos.y < ground_y && body->pos.y >= ground_y - 0.30f)
			{
				body->pos.y = ground_y;
				if (Vector3DotProduct(body->vel, groundNormal) < 0.0f)
					body->vel = ClipVelocity(body->vel, groundNormal, 1.0f);
			}
			body->grounded = false;
		}
		else
		{
			float snapDown =
				(was_grounded || body->vel.y <= 0.0f) ? 0.15f : 0.02f;

			if (body->pos.y <= ground_y + snapDown &&
				body->pos.y >= ground_y - 0.58f)
			{
				body->pos.y = ground_y;

				if (groundNormal.y < SURF_SLOPE_NORMAL)
				{
					body->grounded = false;
					body->vel = ClipVelocity(body->vel, groundNormal, 1.0f);
				}
				else
				{
					body->grounded = true;
					body->vel.y = 0.0f;
				}
			}
			else
			{
				body->grounded = false;
			}
		}
	}
	else if (!body->is_jumping && body->pos.y <= 0.0f && body->vel.y <= 0.0f)
	{
		body->pos.y = 0.0f;
		body->vel.y = 0.0f;
		body->grounded = true;
	}
	else
	{
		body->grounded = false;
	}

	float ceilingY = 0.0f;
	Vector3 ceilNormal = { 0 };
	if (Map_CheckCeiling(body->pos, playerHeight, &ceilingY, &ceilNormal))
	{
		float newY = ceilingY - playerHeight - 0.01f;
		if (hit_ground && newY < ground_y)
			newY = ground_y;

		body->pos.y = newY;
		if (body->vel.y > 0.0f)
		{
			body->vel = ClipVelocity(body->vel, ceilNormal, 1.0f);
			body->is_jumping = false;
		}
	}

	if (!body->grounded)
	{
		body->vel.y -= GRAVITY * 0.5f * dt;
	}
}
