#pragma once

void update_fade(float dt);
void set_fade_callback(void (*cb)(void));
void start_fade_in(float duration);
void start_fade_out(float duration);

void draw_screen_fade(void);

void render_explosion();
