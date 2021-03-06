#pragma once

#ifndef VOX_NETWORK_HEADER
#define VOX_NETWORK_HEADER

#define VOX_NETWORK_PORT 42069

// #include "enet/enet.h"
#include "enetpp/client.h"
#include "enetpp/server.h"

#include "glua.h"

// looks like we have a max of 255 chans
#define VOX_NETWORK_CPP_CHANNEL_START 128
static enet_uint8 VOX_NETWORK_MAX_CHANNELS = 255;

#include <functional>
#include <string>

#include <bitbuf.h>

typedef std::function<void(int peerID, const char* data, size_t data_len)> networkCallback;

#define VOX_NETWORK_CHANNEL_CHUNKDATA_SINGLE 1
#define VOX_NETWORK_CHANNEL_CHUNKDATA_RADIUS 2

bool network_startup();

void network_shutdown();

void setupLuaNetworking(lua_State* state);

namespace networking {
#ifdef VOXELATE_SERVER
	bool channelSend(int peerID, uint16_t channelID, void* data, int size, bool unreliable = false);
#else
	bool channelSend(uint16_t channelID, void* data, int size, bool unreliable = false);
#endif
	void channelListen(uint16_t channelID, networkCallback callback);
};

#endif
