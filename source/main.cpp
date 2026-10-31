
#include "../include/map.h"
#include "../include/net_protocol.h"
#include "../include/network.h"
#include "../include/ui.h"
#include "../include/player.h"
#include "../include/game.h"
#include "../include/input.h"
#include "../include/transition.h"
#include "../include/death.h"
#include "../include/sky.h"
#include "../include/audiomanager.h"

#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <rlgl.h>
#include <stdbool.h>
#include <stdio.h>

static int current_page = 0;

Texture2D player_sprite;
Material player_material;
Mesh player_mesh;

State game_state = GAME_MENU;

Color grass = { 105, 145, 100, 255 };
Color sky = { 204, 209, 217, 255 };
Color ink = { 37, 45, 39, 255 };
Color cream = { 244, 239, 218, 255 };
Color lime = { 202, 220, 126, 255 };

static void ApplySpriteFrameUniforms(const Sprite *spr)
{
    if (spr->frames == 0) return;

    Vector2 uvScale = { 1.0f / (float)spr->frames, 1.0f };
    Vector2 uvOffset = { (float)spr->frame / (float)spr->frames, 0.0f };

    SetShaderValue(player_material.shader, uvScaleLoc, &uvScale, SHADER_UNIFORM_VEC2);
    SetShaderValue(player_material.shader, uvOffsetLoc, &uvOffset, SHADER_UNIFORM_VEC2);
}

void DrawPlayersForPass(void)
{
	Sprite *spr = &sprite_list[0];
    ApplySpriteFrameUniforms(spr);
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerData other = current_state.players[i];

        if (!other.active)
            continue;

        bool is_me = (my_local_id != -1 && other.id == my_local_id);

        if (is_me)
            continue;

        Matrix matRot = MatrixRotateY(other.yaw);
        Matrix matRotX = MatrixRotateX(PI);
        Matrix matTrans = MatrixTranslate(
            other.pos.x,
            other.pos.y + 1.0f,
            other.pos.z
        );

        Matrix matTransform = MatrixMultiply(
            MatrixMultiply(matRotX, matRot),
            matTrans
        );

        DrawMesh(player_mesh, player_material, matTransform);
    }
}



static void draw_players(void)
{
	Sprite *spr = &sprite_list[0];
    ApplySpriteFrameUniforms(spr);
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerData other = current_state.players[i];

        if (!other.active)
            continue;

        bool is_me = (my_local_id != -1 && other.id == my_local_id);

        Vector3 renderPos = {
            other.pos.x,
            other.pos.y + 1.0f,
            other.pos.z
        };

        if (!is_me)
        {
            Matrix matRot = MatrixRotateY(-other.yaw);
            Matrix matRotX = MatrixRotateX(PI);
            Matrix matTrans = MatrixTranslate(
                other.pos.x,
                other.pos.y + 1.0f,
                other.pos.z
            );

            Matrix matTransform = MatrixMultiply(
                MatrixMultiply(matRotX, matRot),
                matTrans
            );
            Sprite spr = sprite_list[0];
            float frameWidthNorm = 1.0f / (float)spr.frames;

            player_material.maps[MATERIAL_MAP_DIFFUSE].value = 1.0f;
            SetMaterialTexture(&player_material, MATERIAL_MAP_DIFFUSE, player_sprite);
            DrawMesh(player_mesh, player_material, matTransform);

            Vector3 forward = {
                sinf(other.yaw) * 0.8f,
                0.0f,
                cosf(other.yaw) * 0.8f
            };

            DrawLine3D(
                renderPos,
                Vector3Add(renderPos, forward),
                BLACK
            );
        }

        bool hit = false;
        float groundY = Map_GetGroundHeight(other.pos, &hit);

        if (hit)
        {
            DrawCylinder(
                (Vector3){
                    other.pos.x,
                    groundY + 0.02f,
                    other.pos.z
                },
                0.55f,
                0.55f,
                0.01f,
                16,
                Fade(BLACK, 0.35f)
            );
        }
    }
}

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Hide n' Hop");
    InitAudioDevice();
    SetExitKey(0);
    SetWindowMonitor(0);

    if (!IsWindowReady())
    {
        printf("couldnt start window!\n");
        CloseWindow();
        return 1;
    }

    load_sounds();

    SetTargetFPS(0);
    ui_init();

    while (!WindowShouldClose())
    {
        switch (game_state)
        {
            case GAME_MENU:
            {
                ui_update();
                break;
            }

            case GAME_INGAME:
            {
                float delta = GetFrameTime();

                Net_PollEvents(
                    &my_local_id,
                    &current_state,
                    &connected,
                    &local_player
                );

                if (IsKeyPressed(KEY_R))
                    TriggerPlayerExplosion((Vector3){ 0.0f, 0.0f, 0.0f });
                if (IsKeyPressed(KEY_E)) {
                    TriggerPlayerDeath(local_player.pos);
                }

                UpdatePlayerExplosions(delta);

                if (!death_seq.is_dying)
                {
                    Update_Input();

                    if (connected)
                        Net_SendUpdate(&local_player, look.x);

                    UpdatePlayerCamera(delta);
                    footstep_player(delta, local_player.vel, local_player.grounded);
                }
                else
                {
                    UpdateDeathSequence(&cam, delta);
                }

                Map_UpdateShader(cam.position);

                BeginDrawing();
                SpriteUpdate();

                Vector3 lightDir = Vector3Normalize(
                    (Vector3){ -0.45f, -1.0f, -0.35f }
                );

                Map_RenderShadowPass(local_player.pos, lightDir, DrawPlayersForPass);

                Map_UpdateShader(cam.position);
                ClearBackground(sky);

                BeginMode3D(cam);

				Sky_Draw(cam);

                Map_Draw();
                draw_players();
                render_explosion();

                EndMode3D();

                update_fade(delta);

                if (!death_seq.is_dying)
                    ui_render_hand();

                ui_debug_menu();

                SpriteDraw(sprite_list[0], {0, 0});

                draw_screen_fade();
                EndDrawing();

                break;
            }

            case GAME_PAUSE:
            {
                break;
            }
        }
    }

    Map_Unload();
    Net_Shutdown();
    Sky_Unload();
    UnloadShader(sprite_shader);
    UnloadMesh(player_mesh);
    SpriteUnloadAll();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
