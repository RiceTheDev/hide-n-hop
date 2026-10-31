#include "../include/spritemanager.h"
#include "../include/player.h"

Sprite sprite_list[MAX_SPRITES] = { 0 };

int SpriteLoad(const char *fileName, uint8_t frames, float speed)
{
	if (frames == 0)
		frames = 1;

	for (int s = 0; s < MAX_SPRITES; s++)
	{
		if (sprite_list[s].texture.id == 0)
		{
			Texture2D tex = LoadTexture(fileName);
			if (tex.id == 0)
			{
				TraceLog(LOG_WARNING,
						 "spritemanager.c: failed to load texture: %s",
						 fileName);
				return -1;
			}

			sprite_list[s].texture = tex;
			sprite_list[s].frames = frames;
			sprite_list[s].frame = 0;
			sprite_list[s].size =
				(Vector2){ (float)tex.width / frames, (float)tex.height };
			sprite_list[s].timer = 0.0f;
			sprite_list[s].speed = speed;

			return s;
		}
	}

	TraceLog(LOG_WARNING,
			 "spritemanager.c: sprite_list is full (MAX_SPRITES = %d)",
			 MAX_SPRITES);
	return -1;
}

void SpriteUpdate()
{
	float dt = GetFrameTime();

	SetMaterialTexture(
		&player_material, MATERIAL_MAP_DIFFUSE, sprite_list[0].texture);

	for (int s = 0; s < MAX_SPRITES; s++)
	{
		if (sprite_list[s].texture.id == 0 || sprite_list[s].frames <= 1)
			continue;

		sprite_list[s].timer += dt;

		if (sprite_list[s].timer >= sprite_list[s].speed)
		{
			sprite_list[s].timer -= sprite_list[s].speed;
			sprite_list[s].frame =
				(sprite_list[s].frame + 1) % sprite_list[s].frames;
		}
	}
}

void SpriteDraw(Sprite sprite, Vector2 position)
{
	if (sprite.texture.id == 0)
		return;

	Rectangle source = { .x = (float)sprite.frame * sprite.size.x,
						 .y = 0.0f,
						 .width = sprite.size.x,
						 .height = sprite.size.y };

	DrawTextureRec(sprite.texture, source, position, WHITE);
}

void SpriteUnloadAll(void)
{
	for (int s = 0; s < MAX_SPRITES; s++)
	{
		if (sprite_list[s].texture.id != 0)
		{
			UnloadTexture(sprite_list[s].texture);
			sprite_list[s] = (Sprite){ 0 };
		}
	}
}
