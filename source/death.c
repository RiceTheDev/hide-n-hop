
#include "raylib.h"
#include "raymath.h"
#include "../include/death.h"
#include "../include/player.h"
#include "../include/transition.h"
#include <stdbool.h>
#include <math.h>

DeathSequence death_seq = { 0 };
DeathSequence player_explosions[MAX_PLAYER_EXPLOSIONS] = { 0 };

void TriggerPlayerDeath(Vector3 player_pos)
{
	death_seq.is_dying = true;
	death_seq.timer = 0.0f;
	death_seq.duration = 0.50f;
	death_seq.death_pos = player_pos;
	death_seq.exploded = false;
	death_seq.control_camera = true;
	death_seq.fade_timer = 0.0f;
	death_seq.fade_started = false;
}

void TriggerPlayerExplosion(Vector3 player_pos)
{
	for (int i = 0; i < MAX_PLAYER_EXPLOSIONS; i++)
	{
		if (!player_explosions[i].is_dying)
		{
			player_explosions[i].is_dying = true;
			player_explosions[i].timer = 0.0f;
			player_explosions[i].duration = 0.5f;
			player_explosions[i].death_pos = player_pos;
			player_explosions[i].exploded = false;
			player_explosions[i].control_camera = false;
			death_seq.fade_timer = 0.0f;
			death_seq.fade_started = false;
			return;
		}
	}
}

void UpdatePlayerExplosions(float delta)
{
	for (int i = 0; i < MAX_PLAYER_EXPLOSIONS; i++)
	{
		DeathSequence *explosion = &player_explosions[i];

		if (!explosion->is_dying)
			continue;

		explosion->timer += delta;

		if (!explosion->exploded && explosion->timer >= explosion->duration)
		{
			explosion->exploded = true;
		}

		if (explosion->exploded &&
			explosion->timer >= explosion->duration + 2.0f)
		{
			explosion->is_dying = false;
		}
	}
}

static void respawn_player(void)
{
	death_seq.is_dying = false;
	death_seq.exploded = false;
	death_seq.control_camera = false;

	local_player.pos = (Vector3){ 0, 5.0f, 0 };
	local_player.vel = (Vector3){ 0, 0, 0 };

	start_fade_in(0.75f);
}

void UpdateDeathSequence(Camera3D *cam, float delta)
{
	UpdatePlayerExplosions(delta);

	if (!death_seq.is_dying)
		return;

	death_seq.timer += delta;

	if (!death_seq.exploded)
	{
		Vector3 targetPos = { death_seq.death_pos.x,
							  death_seq.death_pos.y + 0.8f,
							  death_seq.death_pos.z };

		float t = fminf(death_seq.timer / death_seq.duration, 1.0f);
		float ease = (expf(4.0f * t) - 1.0f) / (expf(4.0f) - 1.0f);

		float distance = 1.0f + ease * 4.5f;
		float height = 0.5f + ease * 2.5f;
		float angle = ease * 1.5f;

		cam->position.x = targetPos.x + sinf(angle) * distance;
		cam->position.y = targetPos.y + height;
		cam->position.z = targetPos.z + cosf(angle) * distance;

		float preShake = (t > 0.5f) ? (t - 0.5f) * 0.15f : 0.0f;

		cam->target.x =
			targetPos.x + ((float)GetRandomValue(-10, 10) / 100.0f) * preShake;
		cam->target.y =
			targetPos.y + ((float)GetRandomValue(-10, 10) / 100.0f) * preShake;
		cam->target.z =
			targetPos.z + ((float)GetRandomValue(-10, 10) / 100.0f) * preShake;

		cam->up = (Vector3){ 0.0f, 1.0f, 0.0f };

		if (death_seq.timer >= death_seq.duration)
			death_seq.exploded = true;
	}
	else
	{
		float postShakeTime = death_seq.timer - death_seq.duration;
		float postShake = fmaxf(0.0f, 1.0f - (postShakeTime / 0.5f)) * 0.5f;

		cam->target.x = death_seq.death_pos.x +
						((float)GetRandomValue(-10, 10) / 100.0f) * postShake;
		cam->target.y = death_seq.death_pos.y + 0.8f +
						((float)GetRandomValue(-10, 10) / 100.0f) * postShake;
		cam->target.z = death_seq.death_pos.z +
						((float)GetRandomValue(-10, 10) / 100.0f) * postShake;
	}

	if (death_seq.exploded && !death_seq.fade_started)
	{
		death_seq.fade_timer += delta;

		if (death_seq.fade_timer >= 0.5f)
		{
			death_seq.fade_started = true;
			start_fade_out(0.5f);
			set_fade_callback(respawn_player);
		}
	}
}
