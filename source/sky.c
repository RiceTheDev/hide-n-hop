#include "../include/sky.h"
#include "rlgl.h"
#include <stddef.h>

static Mesh sky_mesh = { 0 };
static Model sky_model = { 0 };
static Shader sky_shader = { 0 };

static int locLightDir = -1;
static int locLightColor = -1;
static int locSkyColor = -1;
static int locHorizonColor = -1;

const SkyPreset PRESET_MORNING = {
	.lightDir = { -0.50f, -0.60f, -0.40f },
	.lightColor = { 1.0f, 0.96f, 0.85f, 1.0f },
	.skyColor = { 0.42f, 0.65f, 0.95f, 1.0f },
	.horizonColor = { 0.82f, 0.88f, 0.96f, 1.0f },
	.groundColor = { 0.22f, 0.28f, 0.20f, 1.0f },
	.fogColor = { 0.80f, 0.87f, 0.95f, 1.0f },
	.fogDensity = 0.004f
};

const SkyPreset PRESET_DUSK = { .lightDir = { -0.75f, -0.25f, -0.30f },
								.lightColor = { 1.0f, 0.65f, 0.35f, 1.0f },
								.skyColor = { 0.25f, 0.28f, 0.58f, 1.0f },
								.horizonColor = { 0.88f, 0.52f, 0.35f, 1.0f },
								.groundColor = { 0.30f, 0.18f, 0.12f, 1.0f },
								.fogColor = { 0.85f, 0.55f, 0.45f, 1.0f },
								.fogDensity = 0.0035f };

bool Sky_Init(void)
{
	sky_shader = LoadShader("assets/shaders/sky.vs", "assets/shaders/sky.fs");
	if (sky_shader.id == 0)
		return false;

	locLightDir = GetShaderLocation(sky_shader, "lightDir");
	locLightColor = GetShaderLocation(sky_shader, "lightColor");
	locSkyColor = GetShaderLocation(sky_shader, "skyColor");
	locHorizonColor = GetShaderLocation(sky_shader, "horizonColor");

	sky_mesh = GenMeshCube(1.0f, 1.0f, 1.0f);
	sky_model = LoadModelFromMesh(sky_mesh);
	sky_model.materials[0].shader = sky_shader;

	return true;
}

void Sky_ApplyPreset(const SkyPreset *preset)
{
	if (sky_shader.id == 0 || preset == NULL)
		return;

	SetShaderValue(
		sky_shader, locLightDir, &preset->lightDir, SHADER_UNIFORM_VEC3);
	SetShaderValue(
		sky_shader, locLightColor, &preset->lightColor, SHADER_UNIFORM_VEC4);
	SetShaderValue(
		sky_shader, locSkyColor, &preset->skyColor, SHADER_UNIFORM_VEC4);
	SetShaderValue(sky_shader,
				   locHorizonColor,
				   &preset->horizonColor,
				   SHADER_UNIFORM_VEC4);
}

void Sky_Draw(Camera3D camera)
{
	rlDisableDepthMask();
	rlDisableBackfaceCulling();

	DrawModel(sky_model, camera.position, 50.0f, WHITE);

	rlEnableBackfaceCulling();
	rlEnableDepthMask();
}

void Sky_Unload(void)
{
	UnloadModel(sky_model);
	UnloadShader(sky_shader);
}
