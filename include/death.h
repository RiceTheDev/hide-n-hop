
#pragma once
#include <raylib.h>
#include <stdbool.h>

#define MAX_PLAYER_EXPLOSIONS 32

typedef struct
{
	bool is_dying;
	float timer;
	float duration;
	Vector3 death_pos;
	bool exploded;
	bool control_camera;
	float fade_timer;
	bool fade_started;
} DeathSequence;

extern DeathSequence death_seq;
extern DeathSequence player_explosions[MAX_PLAYER_EXPLOSIONS];

void TriggerPlayerDeath(Vector3 player_pos);
void TriggerPlayerExplosion(Vector3 player_pos);
void UpdateDeathSequence(Camera3D *cam, float delta);
void UpdatePlayerExplosions(float delta);
