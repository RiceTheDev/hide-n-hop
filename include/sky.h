#pragma once

#include "raylib.h"

typedef struct
{
	Vector3 lightDir;
	Vector4 lightColor;
	Vector4 skyColor;
	Vector4 horizonColor;
	Vector4 groundColor;
	Vector4 fogColor;
	float fogDensity;
} SkyPreset;

extern const SkyPreset PRESET_MORNING;
extern const SkyPreset PRESET_DUSK;
extern const SkyPreset PRESET_NIGHT;

bool Sky_Init(void);
void Sky_ApplyPreset(const SkyPreset *preset);
void Sky_Draw(Camera3D camera);
void Sky_Unload(void);
