#include "../include/map.h"
#include "../include/sky.h"
#include <stddef.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <raymath.h>
#include "rlgl.h"

static Model visual_model = { 0 };
static Texture2D visual_texture = { 0 };
static Model collision_model = { 0 };

#define SHADOWMAP_RESOLUTION 2048

static Shader light_shader = { 0 };
static RenderTexture2D shadow_map = { 0 };
static Camera3D light_cam = { 0 };

static int viewPosLoc = -1;
static int lightVPLoc = -1;
static int shadowMapLoc = -1;

typedef struct
{
	Vector3 a, b, c;
	Vector3 min, max;
} CollisionTri;

static CollisionTri *all_tris = NULL;
static int all_tri_count = 0;

static int *local_tri_indices = NULL;
static int local_tri_count = 0;

static Vector3 cache_center = { -99999.0f, -99999.0f, -99999.0f };
#define CACHE_EXTENT_XZ 4.0f
#define CACHE_EXTENT_Y 5.0f
#define CACHE_REBUILD_DIST_XZ 2.0f
#define CACHE_REBUILD_DIST_Y 2.5f

static void BuildCollisionTriangles(void)
{
	all_tri_count = 0;
	for (int m = 0; m < collision_model.meshCount; m++)
	{
		all_tri_count += collision_model.meshes[m].triangleCount;
	}

	if (all_tri_count == 0)
		return;

	all_tris = (CollisionTri *)MemAlloc(sizeof(CollisionTri) * all_tri_count);
	local_tri_indices = (int *)MemAlloc(sizeof(int) * all_tri_count);

	int idx = 0;
	for (int m = 0; m < collision_model.meshCount; m++)
	{
		Mesh mesh = collision_model.meshes[m];
		const float *verts = mesh.vertices;
		const unsigned short *indices = mesh.indices;

		for (int t = 0; t < mesh.triangleCount; t++)
		{
			Vector3 a, b, c;
			if (indices != NULL)
			{
				int i0 = indices[t * 3 + 0] * 3;
				int i1 = indices[t * 3 + 1] * 3;
				int i2 = indices[t * 3 + 2] * 3;
				a = (Vector3){ verts[i0], verts[i0 + 1], verts[i0 + 2] };
				b = (Vector3){ verts[i1], verts[i1 + 1], verts[i1 + 2] };
				c = (Vector3){ verts[i2], verts[i2 + 1], verts[i2 + 2] };
			}
			else
			{
				int base = t * 9;
				a = (Vector3){ verts[base + 0],
							   verts[base + 1],
							   verts[base + 2] };
				b = (Vector3){ verts[base + 3],
							   verts[base + 4],
							   verts[base + 5] };
				c = (Vector3){ verts[base + 6],
							   verts[base + 7],
							   verts[base + 8] };
			}

			CollisionTri tri;
			tri.a = a;
			tri.b = b;
			tri.c = c;

			tri.min.x = fminf(a.x, fminf(b.x, c.x));
			tri.min.y = fminf(a.y, fminf(b.y, c.y));
			tri.min.z = fminf(a.z, fminf(b.z, c.z));

			tri.max.x = fmaxf(a.x, fmaxf(b.x, c.x));
			tri.max.y = fmaxf(a.y, fmaxf(b.y, c.y));
			tri.max.z = fmaxf(a.z, fmaxf(b.z, c.z));

			all_tris[idx++] = tri;
		}
	}

	cache_center = (Vector3){ -99999.0f, -99999.0f, -99999.0f };
}

static void EnsureLocalCache(Vector3 pos)
{
	if (fabsf(pos.x - cache_center.x) <= CACHE_REBUILD_DIST_XZ &&
		fabsf(pos.y - cache_center.y) <= CACHE_REBUILD_DIST_Y &&
		fabsf(pos.z - cache_center.z) <= CACHE_REBUILD_DIST_XZ)
	{
		return;
	}

	cache_center = pos;
	local_tri_count = 0;

	float minX = pos.x - CACHE_EXTENT_XZ;
	float maxX = pos.x + CACHE_EXTENT_XZ;
	float minY = pos.y - CACHE_EXTENT_Y;
	float maxY = pos.y + CACHE_EXTENT_Y;
	float minZ = pos.z - CACHE_EXTENT_XZ;
	float maxZ = pos.z + CACHE_EXTENT_XZ;

	for (int i = 0; i < all_tri_count; i++)
	{
		const CollisionTri *t = &all_tris[i];
		if (t->max.x < minX || t->min.x > maxX || t->max.y < minY ||
			t->min.y > maxY || t->max.z < minZ || t->min.z > maxZ)
		{
			continue;
		}
		local_tri_indices[local_tri_count++] = i;
	}
}

static RayCollision RaycastLocal(Ray ray, float maxDist)
{
	RayCollision best = { 0 };
	best.distance = FLT_MAX;

	Vector3 endPos =
		Vector3Add(ray.position, Vector3Scale(ray.direction, maxDist));
	Vector3 rMin = { fminf(ray.position.x, endPos.x) - 0.01f,
					 fminf(ray.position.y, endPos.y) - 0.01f,
					 fminf(ray.position.z, endPos.z) - 0.01f };
	Vector3 rMax = { fmaxf(ray.position.x, endPos.x) + 0.01f,
					 fmaxf(ray.position.y, endPos.y) + 0.01f,
					 fmaxf(ray.position.z, endPos.z) + 0.01f };

	for (int i = 0; i < local_tri_count; i++)
	{
		const CollisionTri *t = &all_tris[local_tri_indices[i]];

		if (t->max.x < rMin.x || t->min.x > rMax.x || t->max.y < rMin.y ||
			t->min.y > rMax.y || t->max.z < rMin.z || t->min.z > rMax.z)
		{
			continue;
		}

		RayCollision col = GetRayCollisionTriangle(ray, t->a, t->b, t->c);
		if (col.hit && col.distance <= maxDist && col.distance < best.distance)
		{
			best = col;
		}
	}

	return best;
}

static RenderTexture2D LoadShadowmapRenderTexture(int width, int height)
{
	RenderTexture2D target = { 0 };
	target.id = rlLoadFramebuffer();
	target.texture.width = width;
	target.texture.height = height;

	if (target.id > 0)
	{
		rlEnableFramebuffer(target.id);
		target.depth.id = rlLoadTextureDepth(width, height, false);
		target.depth.width = width;
		target.depth.height = height;
		target.depth.format = 19; // DEPTH_COMPONENT_24BIT
		target.depth.mipmaps = 1;

		rlFramebufferAttach(target.id,
							target.depth.id,
							RL_ATTACHMENT_DEPTH,
							RL_ATTACHMENT_TEXTURE2D,
							0);
		rlDisableFramebuffer();
	}
	return target;
}

bool Map_Load(const char *visualPath,
			  const char *texturePath,
			  const char *collisionPath)
{
	(void)texturePath;
	visual_model = LoadModel(visualPath);
	if (visual_model.meshCount == 0)
		return false;

	light_shader =
		LoadShader("assets/shaders/lighting.vs", "assets/shaders/lighting.fs");
	if (light_shader.id == 0)
	{
		UnloadModel(visual_model);
		return false;
	}

	viewPosLoc = GetShaderLocation(light_shader, "viewPos");
	lightVPLoc = GetShaderLocation(light_shader, "lightVP");
	shadowMapLoc = GetShaderLocation(light_shader, "shadowMap");

	int shadowSlot = 1;
	SetShaderValue(light_shader, shadowMapLoc, &shadowSlot, SHADER_UNIFORM_INT);

	Sky_Init();

	Sky_ApplyPreset(&PRESET_DUSK);

	// por do sol muito foda:
	Vector3 lightDir = Vector3Normalize((Vector3){ -0.75f, -0.25f, -0.30f });
	Vector4 lightColor = { 1.0f, 0.65f, 0.35f, 1.0f };
	Vector4 skyColor = { 0.45f, 0.48f, 0.75f, 1.0f };
	Vector4 groundColor = { 0.30f, 0.18f, 0.12f, 1.0f };

	Vector4 fogColor = { 0.85f, 0.55f, 0.45f, 1.0f };
	float fogDensity = 0.0035f;
	//
	// Vector3 lightDir = Vector3Normalize((Vector3){ -0.50f, -0.60f, -0.40f });
	// Vector4 lightColor = { 1.0f, 0.96f, 0.85f, 1.0f };
	// Vector4 skyColor = { 0.55f, 0.75f, 0.98f, 1.0f };
	// Vector4 groundColor = { 0.22f, 0.28f, 0.20f, 1.0f };
	// Vector4 fogColor = { 0.78f, 0.86f, 0.95f, 1.0f };
	// float fogDensity = 0.004f;

	SetShaderValue(light_shader,
				   GetShaderLocation(light_shader, "lightDir"),
				   &lightDir,
				   SHADER_UNIFORM_VEC3);
	SetShaderValue(light_shader,
				   GetShaderLocation(light_shader, "lightColor"),
				   &lightColor,
				   SHADER_UNIFORM_VEC4);
	SetShaderValue(light_shader,
				   GetShaderLocation(light_shader, "skyColor"),
				   &skyColor,
				   SHADER_UNIFORM_VEC4);
	SetShaderValue(light_shader,
				   GetShaderLocation(light_shader, "groundColor"),
				   &groundColor,
				   SHADER_UNIFORM_VEC4);
	SetShaderValue(light_shader,
				   GetShaderLocation(light_shader, "fogColor"),
				   &fogColor,
				   SHADER_UNIFORM_VEC4);
	SetShaderValue(light_shader,
				   GetShaderLocation(light_shader, "fogDensity"),
				   &fogDensity,
				   SHADER_UNIFORM_FLOAT);

	shadow_map =
		LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
	SetTextureFilter(shadow_map.depth, TEXTURE_FILTER_BILINEAR);

	light_cam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	light_cam.fovy = 280.0f;
	light_cam.projection = CAMERA_ORTHOGRAPHIC;

	for (int i = 0; i < visual_model.materialCount; i++)
	{
		visual_model.materials[i].shader = light_shader;
	}

	collision_model = LoadModel(collisionPath);
	BuildCollisionTriangles();

	return true;
}

void Map_RenderShadowPass(Vector3 playerPos,
						  Vector3 lightDir,
						  void (*drawEntitiesCallback)(void))
{
	float shadowDistance = 100.0f;
	light_cam.position =
		Vector3Add(playerPos, Vector3Scale(lightDir, -shadowDistance));
	light_cam.target = playerPos;

	BeginTextureMode(shadow_map);
	ClearBackground(WHITE);
	BeginMode3D(light_cam);

	Matrix matView = rlGetMatrixModelview();
	Matrix matProj = rlGetMatrixProjection();
	Matrix lightVP = MatrixMultiply(matView, matProj);

	SetShaderValueMatrix(light_shader, lightVPLoc, lightVP);

	DrawModel(visual_model, (Vector3){ 0, 0, 0 }, 1.0f, WHITE);

	if (drawEntitiesCallback != NULL)
	{
		drawEntitiesCallback();
	}

	EndMode3D();
	EndTextureMode();
}

void Map_UpdateShader(Vector3 camPos)
{
	SetShaderValue(light_shader, viewPosLoc, &camPos, SHADER_UNIFORM_VEC3);
}

Shader Map_GetShader(void)
{
	return light_shader;
}

void Map_Draw(void)
{
	rlActiveTextureSlot(1);
	rlEnableTexture(shadow_map.depth.id);

	DrawModel(visual_model, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
	rlActiveTextureSlot(0);
}

void Map_Unload(void)
{
	if (all_tris != NULL)
	{
		MemFree(all_tris);
		all_tris = NULL;
	}
	if (local_tri_indices != NULL)
	{
		MemFree(local_tri_indices);
		local_tri_indices = NULL;
	}
	all_tri_count = 0;
	local_tri_count = 0;

	if (shadow_map.id > 0)
	{
		rlUnloadFramebuffer(shadow_map.id);
	}

	UnloadModel(visual_model);
	if (visual_texture.id != 0)
		UnloadTexture(visual_texture);
	UnloadModel(collision_model);
	UnloadShader(light_shader);
}

float Map_GetGroundHeight(Vector3 pos, bool *success)
{
	*success = false;
	if (all_tri_count == 0)
		return 0.0f;

	Ray ray = { .position = { pos.x, pos.y + 5.0f, pos.z },
				.direction = { 0.0f, -1.0f, 0.0f } };

	float best_y = -99999.0f;
	bool hit_any = false;

	for (int i = 0; i < all_tri_count; i++)
	{
		const CollisionTri *t = &all_tris[i];

		if (pos.x < t->min.x || pos.x > t->max.x || pos.z < t->min.z ||
			pos.z > t->max.z || t->max.y > ray.position.y)
		{
			continue;
		}

		RayCollision col = GetRayCollisionTriangle(ray, t->a, t->b, t->c);
		if (col.hit && col.point.y > best_y)
		{
			best_y = col.point.y;
			hit_any = true;
		}
	}

	if (hit_any)
	{
		*success = true;
		return best_y;
	}

	return 0.0f;
}

float Map_GetGroundInfo(Vector3 pos, bool *hit, Vector3 *outNormal)
{
	*hit = false;
	*outNormal = (Vector3){ 0.0f, 1.0f, 0.0f };
	if (all_tri_count == 0)
		return 0.0f;

	EnsureLocalCache(pos);

	float stepHeight = 0.60f;
	float maxDist = stepHeight + 0.60f;

	Vector3 offsets[5] = { { 0.0f, 0.0f, 0.0f },
						   { 0.12f, 0.0f, 0.0f },
						   { -0.12f, 0.0f, 0.0f },
						   { 0.0f, 0.0f, 0.12f },
						   { 0.0f, 0.0f, -0.12f } };

	float best_y = -99999.0f;

	for (int o = 0; o < 5; o++)
	{
		Ray ray = { .position = { pos.x + offsets[o].x,
								  pos.y + stepHeight,
								  pos.z + offsets[o].z },
					.direction = { 0.0f, -1.0f, 0.0f } };

		for (int i = 0; i < local_tri_count; i++)
		{
			const CollisionTri *t = &all_tris[local_tri_indices[i]];

			if (ray.position.x < t->min.x || ray.position.x > t->max.x ||
				ray.position.z < t->min.z || ray.position.z > t->max.z)
			{
				continue;
			}

			RayCollision col = GetRayCollisionTriangle(ray, t->a, t->b, t->c);
			if (col.hit && col.distance <= maxDist && col.normal.y > 0.1f &&
				col.point.y > best_y)
			{
				best_y = col.point.y;
				*outNormal = Vector3Normalize(col.normal);
				*hit = true;
			}
		}

		if (o == 0 && *hit && best_y >= pos.y - 0.25f)
			break;
	}

	return *hit ? best_y : 0.0f;
}

bool Map_CheckWallNormal(Vector3 pos,
						 float radius,
						 float playerHeight,
						 Vector3 *outNormal,
						 float *outPush)
{
	if (all_tri_count == 0)
		return false;

	EnsureLocalCache(pos);

	float heights[3] = { 0.35f, playerHeight * 0.55f, playerHeight - 0.15f };

	Vector3 dirs[8] = { { 1, 0, 0 },
						{ -1, 0, 0 },
						{ 0, 0, 1 },
						{ 0, 0, -1 },
						{ 0.707f, 0, 0.707f },
						{ -0.707f, 0, 0.707f },
						{ 0.707f, 0, -0.707f },
						{ -0.707f, 0, -0.707f } };

	bool hit_any = false;
	float max_push = 0.0f;

	for (int h = 0; h < 3; h++)
	{
		Vector3 center = { pos.x, pos.y + heights[h], pos.z };
		for (int d = 0; d < 8; d++)
		{
			Ray ray = { .position = center, .direction = dirs[d] };
			RayCollision col = RaycastLocal(ray, radius);

			if (!col.hit)
				continue;

			Vector3 n = Vector3Normalize(col.normal);

			if (h == 0 && n.y >= 0.7f)
				continue;

			float push = radius - col.distance;
			if (push > max_push)
			{
				max_push = push;

				if (n.y <= 0.1f)
				{
					Vector3 flat = { n.x, 0.0f, n.z };
					*outNormal = (Vector3LengthSqr(flat) > 0.001f)
									 ? Vector3Normalize(flat)
									 : n;
				}
				else
				{
					*outNormal = n;
				}

				*outPush = push;
				hit_any = true;
			}
		}
	}
	return hit_any;
}

bool Map_CheckCeiling(Vector3 pos,
					  float playerHeight,
					  float *outCeilingY,
					  Vector3 *outNormal)
{
	if (all_tri_count == 0)
		return false;

	EnsureLocalCache(pos);

	float startY = pos.y + (playerHeight * 0.5f);
	float checkDist = playerHeight - (playerHeight * 0.5f);

	Ray ray = { .position = { pos.x, startY, pos.z },
				.direction = { 0.0f, 1.0f, 0.0f } };

	bool hit_ceiling = false;
	float lowest_ceiling = 99999.0f;

	for (int i = 0; i < local_tri_count; i++)
	{
		const CollisionTri *t = &all_tris[local_tri_indices[i]];

		if (pos.x < t->min.x || pos.x > t->max.x || pos.z < t->min.z ||
			pos.z > t->max.z)
		{
			continue;
		}

		RayCollision col = GetRayCollisionTriangle(ray, t->a, t->b, t->c);
		if (col.hit && col.distance <= checkDist && col.normal.y < -0.1f &&
			col.point.y < lowest_ceiling)
		{
			lowest_ceiling = col.point.y;
			*outCeilingY = col.point.y;
			*outNormal = Vector3Normalize(col.normal);
			hit_ceiling = true;
		}
	}
	return hit_ceiling;
}

bool Map_CheckWall(Vector3 pos)
{
	if (all_tri_count == 0)
		return false;

	EnsureLocalCache(pos);

	Vector3 center = { pos.x, pos.y + 1.0f, pos.z };
	float radius = 0.4f;

	Vector3 directions[4] = { { 1.0f, 0.0f, 0.0f },
							  { -1.0f, 0.0f, 0.0f },
							  { 0.0f, 0.0f, 1.0f },
							  { 0.0f, 0.0f, -1.0f } };

	for (int d = 0; d < 4; d++)
	{
		Ray ray = { .position = center, .direction = directions[d] };
		RayCollision col = RaycastLocal(ray, radius);
		if (col.hit)
		{
			return true;
		}
	}

	return false;
}
