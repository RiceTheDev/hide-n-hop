#include "../include/audiomanager.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>

Sound step;
Sound jump;
Sound land;

void load_sounds()
{
	step = LoadSound("assets/sfx/step.wav");
	jump = LoadSound("assets/sfx/jump.wav");
	land = LoadSound("assets/sfx/land.wav");
}

static bool check(float delta, Vector3 velocity, bool grounded)
{
	static float stepDistanceAccumulator = 0.0f;

	float speed = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);

	if (!grounded || speed < 0.1f)
	{
		stepDistanceAccumulator = 0.0f;
		return false;
	}

	stepDistanceAccumulator += speed * delta;

	float stepThreshold = 2.5f;

	if (stepDistanceAccumulator >= stepThreshold)
	{
		stepDistanceAccumulator -= stepThreshold;
		return true;
	}

	return false;
}

void footstep_player(float delta, Vector3 velocity, bool grounded)
{
	if (!IsSoundValid(step))
		return;

	bool play = check(delta, velocity, grounded);

	if (play)
	{
		float randomPitch = 0.90f + ((float)rand() / (float)RAND_MAX) * 0.20f;

		SetSoundPitch(step, randomPitch);
		SetSoundVolume(step, 0.15f);
		PlaySound(step);
	}
}

void jump_player(bool jumping)
{
	if (!IsSoundValid(jump))
		return;

	if (!IsSoundPlaying(jump))
	{
		float randomPitch = 0.6f + ((float)rand() / (float)RAND_MAX) * 0.8f;

		SetSoundPitch(jump, randomPitch);
		SetSoundVolume(jump, 0.15f);
		PlaySound(jump);
	}
}

void landing_player()
{
	if (!IsSoundValid(land))
		return;

	if (!IsSoundPlaying(land))
	{
		float randomPitch = 0.90f + ((float)rand() / (float)RAND_MAX) * 0.20f;

		SetSoundPitch(land, randomPitch);
		SetSoundVolume(land, 0.15f);
		PlaySound(land);
	}
}
