#pragma once

#include <raylib.h>

void load_sounds();
void footstep_player(float delta, Vector3 velocity, bool grounded);
void jump_player(bool jumping);
void landing_player();
