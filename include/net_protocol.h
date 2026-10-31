#ifndef NET_PROTOCOL_H
#define NET_PROTOCOL_H

#include <stdint.h>
#include "raylib.h"

#define PORT 12345
#define MAX_PLAYERS 8
#define POTATO_TIME_LIMIT 25.0f

#pragma pack(push, 1)

typedef enum
{
	PKT_WELCOME,
	PKT_CLIENT_UPDATE,
	PKT_SERVER_STATE,
	PKT_TAG_EVENT,
	PKT_POTATO_EXPLODE
} PacketType;

typedef struct
{
	uint8_t type;
	int32_t my_id;
} PktWelcome;

typedef struct
{
	uint8_t type;
	Vector3 pos;
	Vector3 vel;
	float yaw;
} PktClientUpdate;

typedef struct
{
	int32_t id;
	Vector3 pos;
	float yaw;
	uint8_t active;
} PlayerData;

typedef struct
{
	uint8_t type;
	int32_t it_player_id;
	int32_t player_count;
	float potato_timer;
	PlayerData players[MAX_PLAYERS];
} PktServerState;

typedef struct
{
	uint8_t type;
	int32_t eliminated_id;
} PktPotatoExplode;

typedef struct
{
	uint8_t type;
	int32_t new_it_id;
} PktTagEvent;

#pragma pack(pop)

typedef struct
{
	Vector3 pos;
	Vector3 vel;
	uint8_t grounded;
} Player;

#endif
