#include "../include/input.h"
#include "../include/player.h"
#include <raylib.h>
#include <raymath.h>

void Update_Input()
{
	Vector2 mouse = GetMouseDelta();
	look.x -= mouse.x * sens.x;
	look.y += mouse.y * sens.y;
	look.y = Clamp(look.y, -PI / 2.0f + 0.001f, PI / 2.0f - 0.001f);

	move = (Vector2){ (float)(IsKeyDown(KEY_D) - IsKeyDown(KEY_A)),
					  (float)(IsKeyDown(KEY_S) - IsKeyDown(KEY_W)) };

	if (Vector2LengthSqr(move) > 1.0f)
		move = Vector2Normalize(move);

	bool crouch = IsKeyDown(KEY_LEFT_CONTROL);

	UpdatePlayer(look.x, move, IsKeyDown(KEY_SPACE), crouch);
}
