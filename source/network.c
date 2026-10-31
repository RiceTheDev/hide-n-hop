#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOMINMAX
#define CloseWindow Win32CloseWindow
#define ShowCursor Win32ShowCursor
#define Rectangle Win32Rectangle
#endif

#include <enet/enet.h>

#ifdef _WIN32
#undef CloseWindow
#undef ShowCursor
#undef Rectangle
#undef LoadImage
#undef DrawText
#undef DrawTextEx
#undef PlaySound
#endif

#include "../include/network.h"
#include "../include/player_physics.h"
#include "../include/death.h"
#include <stdio.h>

static ENetHost *client_host = NULL;
static ENetPeer *server_peer = NULL;

int my_local_id = -1;
PktServerState current_state = { 0 };
bool connected = false;

bool Net_Init(const char *server_ip, uint16_t port)
{
	if (enet_initialize() != 0)
		return false;

	ENetAddress client_address = { .host = ENET_HOST_ANY,
								   .port = ENET_PORT_ANY };
	client_host = enet_host_create(&client_address, 1, 2, 0, 0);
	if (!client_host)
		return false;

	ENetAddress address = { 0 };
	enet_address_set_host(&address, server_ip);
	address.port = port;

	server_peer = enet_host_connect(client_host, &address, 2, 0);
	if (!server_peer)
	{
		enet_host_destroy(client_host);
		return false;
	}

	return true;
}

void Net_PollEvents(int *my_local_id,
					PktServerState *server_state,
					bool *connected,
					PlayerPhysicsState *local_player)
{
	ENetEvent net_event;
	while (enet_host_service(client_host, &net_event, 0) > 0)
	{
		switch (net_event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
				*connected = true;
				break;

			case ENET_EVENT_TYPE_RECEIVE:
			{
				if (net_event.packet->dataLength > 0)
				{
					uint8_t type = net_event.packet->data[0];

					if (type == PKT_WELCOME &&
						net_event.packet->dataLength >= sizeof(PktWelcome))
					{
						PktWelcome *welcome =
							(PktWelcome *)net_event.packet->data;
						*my_local_id = welcome->my_id;
						local_player->pos.x = (float)((*my_local_id) * 3);
					}
					else if (type == PKT_SERVER_STATE &&
							 net_event.packet->dataLength >=
								 sizeof(PktServerState))
					{
						PktServerState *state =
							(PktServerState *)net_event.packet->data;
						*server_state = *state;
					}
					else if (type == PKT_TAG_EVENT &&
							 net_event.packet->dataLength >=
								 sizeof(PktTagEvent))
					{
						PktTagEvent *tag =
							(PktTagEvent *)net_event.packet->data;
						server_state->it_player_id = tag->new_it_id;
					}

					else if (type == PKT_POTATO_EXPLODE &&
							 net_event.packet->dataLength >=
								 sizeof(PktPotatoExplode))
					{
						PktPotatoExplode *exploded =
							(PktPotatoExplode *)net_event.packet->data;

						if (exploded->eliminated_id == *my_local_id)
						{
							TriggerPlayerDeath(local_player->pos);
						}
						else
						{
							for (int i = 0; i < MAX_PLAYERS; i++)
							{
								PlayerData *other = &server_state->players[i];

								if (other->active &&
									other->id == exploded->eliminated_id)
								{
									TriggerPlayerExplosion(other->pos);
									break;
								}
							}
						}
					}
				}
				enet_packet_destroy(net_event.packet);
				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT:
				*connected = false;
				*my_local_id = -1;
				break;
		}
	}
}

void Net_SendUpdate(PlayerPhysicsState *local_player, float yaw)
{
	if (!server_peer)
		return;

	PktClientUpdate update = { .type = PKT_CLIENT_UPDATE,
							   .pos = local_player->pos,
							   .vel = local_player->vel,
							   .yaw = yaw };
	ENetPacket *pkt = enet_packet_create(
		&update, sizeof(update), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
	enet_peer_send(server_peer, 0, pkt);
}

void Net_Shutdown(void)
{
	if (server_peer)
	{
		enet_peer_disconnect(server_peer, 0);
	}
	if (client_host)
	{
		enet_host_destroy(client_host);
	}
	enet_deinitialize();
}
