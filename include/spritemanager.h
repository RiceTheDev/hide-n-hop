#pragma once

#include <raylib.h>
#include <stdint.h>

#define MAX_SPRITES 32

typedef struct
{
	Texture2D texture;
	uint8_t frames;
	uint8_t frame;
	Vector2 size;
	float timer;
	float speed;
} Sprite;

extern Sprite sprite_list[MAX_SPRITES];

int SpriteLoad(const char *fileName, uint8_t frames, float speed);

void SpriteUpdate();
void SpriteDraw(Sprite sprite, Vector2 position);
void SpriteUnloadAll(void);
