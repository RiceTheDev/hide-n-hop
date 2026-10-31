#ifndef MAP_H
#define MAP_H

#include "raylib.h"

bool Map_Load(const char *visualPath,
			  const char *texturePath,
			  const char *collisionPath);

void Map_UpdateShader(Vector3 camPos);

Shader Map_GetShader(void);

void Map_RenderShadowPass(Vector3 playerPos,
						  Vector3 lightDir,
						  void (*drawEntitiesCallback)(void));

void Map_Draw(void);

void Map_Unload(void);

float Map_GetGroundHeight(Vector3 pos, bool *success);
float Map_GetGroundInfo(Vector3 pos, bool *hit, Vector3 *outNormal);
bool Map_CheckCeiling(Vector3 pos,
					  float playerHeight,
					  float *outCeilingY,
					  Vector3 *outNormal);
bool Map_CheckWallNormal(Vector3 pos,
						 float radius,
						 float playerHeight,
						 Vector3 *outNormal,
						 float *outPush);
bool Map_CheckWall(Vector3 pos);

#endif
