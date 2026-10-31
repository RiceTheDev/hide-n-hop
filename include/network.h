#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include "net_protocol.h"
#include "player.h"

extern int my_local_id;
extern PktServerState current_state;
extern bool connected;

bool Net_Init(const char *server_ip, uint16_t port);

void Net_PollEvents(int *my_local_id,
					PktServerState *server_state,
					bool *connected,
					PlayerPhysicsState *local_player);

void Net_SendUpdate(PlayerPhysicsState *local_player, float yaw);

void Net_Shutdown(void);

#endif
