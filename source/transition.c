#include "../include/transition.h"
#include "../include/game.h"
#include "../include/death.h"
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stddef.h>

typedef enum
{
	FADE_NONE,
	FADE_OUT,
	FADE_IN
} FadeDir;

static FadeDir fade_dir = FADE_NONE;
static float fade_time = 0.0f;
static float fade_duration = 1.0f;
static float screen_alpha = 0.0f;

static void (*fade_midpoint_callback)(void) = NULL;

void start_fade_out(float duration)
{
	fade_dir = FADE_OUT;
	fade_time = 0.0f;
	fade_duration = (duration > 0.001f) ? duration : 0.001f;
}

void start_fade_in(float duration)
{
	fade_dir = FADE_IN;
	fade_time = 0.0f;
	fade_duration = (duration > 0.001f) ? duration : 0.001f;
}

void set_fade_callback(void (*cb)(void))
{
	fade_midpoint_callback = cb;
}

void update_fade(float dt)
{
	if (fade_dir == FADE_NONE)
		return;

	fade_time += dt;
	float t = fade_time / fade_duration;
	if (t > 1.0f)
		t = 1.0f;

	float eased = (fade_dir == FADE_OUT) ? powf(t, 0.7f) : powf(t, 1.4f);

	if (fade_dir == FADE_OUT)
		screen_alpha = eased;
	else
		screen_alpha = 1.0f - eased;

	if (t >= 1.0f)
	{
		if (fade_dir == FADE_OUT)
			screen_alpha = 1.0f;
		else
			screen_alpha = 0.0f;

		fade_dir = FADE_NONE;

		if (fade_midpoint_callback)
		{
			void (*cb)(void) = fade_midpoint_callback;
			fade_midpoint_callback = NULL;

			cb();
		}
	}
}

void draw_screen_fade(void)
{
	if (screen_alpha > 0.001f)
	{
		unsigned char a = (unsigned char)(screen_alpha * 255.0f);
		DrawRectangle(
			0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 0, 0, 0, a });
	}
}

static void render_explosion_sequence(DeathSequence *explosion)
{
	if (!explosion->is_dying || !explosion->exploded)
		return;

	float t =
		Clamp((explosion->timer - explosion->duration) / 2.0f, 0.0f, 1.0f);

	float progress = 1.0f - powf(1.0f - t, 5.0f);

	Vector3 pos = { explosion->death_pos.x,
					explosion->death_pos.y + 1.0f,
					explosion->death_pos.z };

	float radius = 0.5f + progress * 4.5f;

	SetShaderValue(
		explosion_shader, progressLoc, &progress, SHADER_UNIFORM_FLOAT);

	BeginBlendMode(BLEND_ALPHA);
	rlDrawRenderBatchActive();
	BeginShaderMode(explosion_shader);
	DrawSphereEx(pos, radius, 32, 32, WHITE);
	EndShaderMode();
	EndBlendMode();
}

void render_explosion(void)
{
	render_explosion_sequence(&death_seq);

	for (int i = 0; i < MAX_PLAYER_EXPLOSIONS; i++)
		render_explosion_sequence(&player_explosions[i]);
}
