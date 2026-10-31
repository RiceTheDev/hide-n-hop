#include "include/net_protocol.h"
#include <enet/enet.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif

static PktServerState state = { .type = PKT_SERVER_STATE,
								.it_player_id = -1,
								.player_count = 0,
								.potato_timer = POTATO_TIME_LIMIT };

static float tag_cooldown = 0.0f;

int GetFreeSlot(void)
{
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (!state.players[i].active)
			return i;
	}
	return -1;
}

int PickRandomActivePlayer(int exclude_id)
{
	int candidates[MAX_PLAYERS];
	int count = 0;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (state.players[i].active && i != exclude_id)
		{
			candidates[count++] = i;
		}
	}

	if (count == 0)
	{
		if (exclude_id >= 0 && state.players[exclude_id].active)
			return exclude_id;
		return -1;
	}

	return candidates[rand() % count];
}

float DistSqr(Vector3 a, Vector3 b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	return dx * dx + dy * dy + dz * dz;
}

int main(void)
{
	if (enet_initialize() != 0)
		return 1;

	ENetAddress address = { .host = ENET_HOST_ANY, .port = PORT };
	ENetHost *server = enet_host_create(&address, MAX_PLAYERS, 2, 0, 0);
	if (!server)
		return 1;

	printf("server running on port %d...\n", PORT);

	const float delta_tick = 0.016f;

	while (1)
	{
		ENetEvent event;
		while (enet_host_service(server, &event, 1) > 0)
		{
			switch (event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
				{
					int slot = GetFreeSlot();
					if (slot == -1)
					{
						enet_peer_reset(event.peer);
						break;
					}

					event.peer->data = (void *)(uintptr_t)slot;
					state.players[slot].id = slot;
					state.players[slot].active = 1;
					state.players[slot].pos =
						(Vector3){ (float)(slot * 3), 0.0f, 0.0f };
					state.players[slot].yaw = 0.0f;
					state.player_count++;

					if (state.it_player_id == -1)
					{
						state.it_player_id = slot;
						state.potato_timer = POTATO_TIME_LIMIT;
					}

					PktWelcome welcome = { .type = PKT_WELCOME, .my_id = slot };
					ENetPacket *pkt = enet_packet_create(
						&welcome, sizeof(welcome), ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(event.peer, 0, pkt);
					printf("player connected on slot %d\n", slot);
					break;
				}

				case ENET_EVENT_TYPE_RECEIVE:
				{
					int id = (int)(uintptr_t)event.peer->data;
					if (event.packet->dataLength > 0 && id >= 0 &&
						id < MAX_PLAYERS)
					{
						uint8_t type = event.packet->data[0];
						if (type == PKT_CLIENT_UPDATE &&
							event.packet->dataLength >= sizeof(PktClientUpdate))
						{
							PktClientUpdate *upd =
								(PktClientUpdate *)event.packet->data;
							state.players[id].pos = upd->pos;
							state.players[id].yaw = upd->yaw;
						}
					}
					enet_packet_destroy(event.packet);
					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT:
				{
					int id = (int)(uintptr_t)event.peer->data;
					if (id >= 0 && id < MAX_PLAYERS)
					{
						state.players[id].active = 0;
						state.player_count--;

						if (state.it_player_id == id)
						{
							state.it_player_id = PickRandomActivePlayer(-1);
							state.potato_timer = POTATO_TIME_LIMIT;
						}
						printf("player disconnected from slot %d\n", id);
					}
					event.peer->data = NULL;
					break;
				}
			}
		}

		if (tag_cooldown > 0.0f)
		{
			tag_cooldown -= delta_tick;
		}

		if (state.player_count >= 2 && state.it_player_id != -1)
		{
			state.potato_timer -= delta_tick;

			if (tag_cooldown <= 0.0f &&
				state.players[state.it_player_id].active)
			{
				Vector3 it_pos = state.players[state.it_player_id].pos;
				for (int i = 0; i < MAX_PLAYERS; i++)
				{
					if (i == state.it_player_id || !state.players[i].active)
						continue;

					if (DistSqr(it_pos, state.players[i].pos) <= 2.25f)
					{
						state.it_player_id = i;
						tag_cooldown = 1.0f;

						PktTagEvent tag = { .type = PKT_TAG_EVENT,
											.new_it_id = i };
						ENetPacket *pkt = enet_packet_create(
							&tag, sizeof(tag), ENET_PACKET_FLAG_RELIABLE);
						enet_host_broadcast(server, 0, pkt);
						printf("potato passed to %d\n", i);
						break;
					}
				}
			}

			if (state.potato_timer <= 0.0f)
			{
				int dead_id = state.it_player_id;
				printf("BOOM! Potato exploded on player %d!\n", dead_id);

				PktPotatoExplode explode = { .type = PKT_POTATO_EXPLODE,
											 .eliminated_id = dead_id };
				ENetPacket *pkt = enet_packet_create(
					&explode, sizeof(explode), ENET_PACKET_FLAG_RELIABLE);
				enet_host_broadcast(server, 0, pkt);

				state.it_player_id = PickRandomActivePlayer(dead_id);
				state.potato_timer = POTATO_TIME_LIMIT;
				tag_cooldown = 1.5f;
			}
		}
		else
		{
			state.potato_timer = POTATO_TIME_LIMIT;
		}

		ENetPacket *state_pkt = enet_packet_create(
			&state, sizeof(state), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
		enet_host_broadcast(server, 0, state_pkt);

		sleep_ms(16);
	}

	enet_host_destroy(server);
	enet_deinitialize();
	return 0;
}
