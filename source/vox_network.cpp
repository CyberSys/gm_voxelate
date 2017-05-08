#include "vox_engine.h"

#include "vox_network.h"

#include <vector>

ENetAddress address;

#ifdef VOXELATE_SERVER
ENetHost* server;
std::vector<ENetPeer*> peers;

#define VOX_ENET_HOST server
#else
ENetHost* client;
ENetPeer* peer;

#define VOX_ENET_HOST client
#endif

#include <string>
#include <thread>
#include <mutex>

#include "GarrysMod\LuaHelpers.hpp"

bool network_startup() {
	if (enet_initialize() != 0) {
		Msg("ENet initialization failed...\n");

		return false;
	}

	address.host = ENET_HOST_ANY;
	address.port = VOX_NETWORK_PORT;

#ifdef VOXELATE_SERVER
	server = enet_host_create(&address, 128, 2, 0, 0);

	if (server == NULL) {
		Msg("ENet initialization failed at server creation...\n");

		return false;
}
#else
	client = enet_host_create(NULL, 1, 2, 0, 0);

	if (client == NULL) {
		Msg("ENet initialization failed at client creation...\n");

		return false;
	}
#endif

	return true;
}

void network_shutdown() {
	enet_deinitialize();
}

// thank https://github.com/danielga/gm_enet/blob/master/source/main.cpp#L12
// modified to disable port numbers and ENET_HOST_ANY
static bool ParseAddress(lua_State *state, const char *addr, ENetAddress &outputAddress) {
	std::string addr_str = addr, host_str;
	size_t pos = addr_str.find(':');
	if (pos != addr_str.npos)
	{
		LUA->PushNil();
		LUA->PushString("port number not allowed");
		return false;
	}
	else {
		host_str = addr_str;
	}

	if (host_str.empty())
	{
		LUA->PushNil();
		LUA->PushString("invalid host name");
		return false;
	}

	if (host_str == "*") {
		LUA->PushNil();
		LUA->PushString("wildcard host name disabled");
		return false;
	}
	else if (enet_address_set_host(&outputAddress, host_str.c_str()) != 0)
	{
		LUA->PushNil();
		LUA->PushString("failed to resolve host name");
		return false;
	}

	return true;
}

int lua_network_sendpacket(lua_State* state) {
	std::string data = luaL_checkstring(state, 1);
	int size = luaL_checkinteger(state, 2);
	int unreliable = lua_toboolean(state, 3);

#ifdef VOXELATE_SERVER
	unsigned int peerID = luaL_checkinteger(state, 4);

	if (peerID >= peers.size()) {
		lua_pushstring(state, "bad peer ID");
		lua_error(state);
		return 0;
	}

	ENetPeer* peer = peers.at(peerID);

	if (peer == NULL) {
		lua_pushstring(state, "can't find peer");
		lua_error(state);
		return 0;
	}
#endif

	// this is probably exploitable :weary:

	ENetPacket* packet = enet_packet_create(&data, size, unreliable ? 0 : ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(peer, 0, packet);

	return 0;
}

#ifdef VOXELATE_CLIENT
int lua_network_connect(lua_State* state) {
	ENetEvent event;

	std::string addrStr = luaL_checkstring(state, 1);
	if (!ParseAddress(state, addrStr.c_str(), address)) {
		return 2;
	}

	peer = enet_host_connect(client, &address, 2, 0);
	if (peer == NULL) {
		lua_pushnil(state);
		lua_pushstring(state, "No available peers for initiating an ENet connection.\n");
		return 2;
	}

	if (enet_host_service(client, &event, 5000) > 0 &&
		event.type == ENET_EVENT_TYPE_CONNECT) {

		lua_pushboolean(state, true);
		return 1;
	}
	else {
		enet_peer_reset(peer);

		lua_pushnil(state);
		lua_pushstring(state, "Connection to server failed.");

		return 2;
	}
}
#endif

std::vector<ENetEvent> events;
std::mutex eventMutex;

void networkEventLoop() {
	ENetEvent event;
	/* Wait up to 1000 milliseconds for an event. */

	while (enet_host_service(VOX_ENET_HOST, &event, 1000) > 0) {
		eventMutex.lock();
		events.push_back(event);
		eventMutex.unlock();
	}
}

struct PeerData {
	int peerID;
	std::string steamID;
};

#ifdef VOXELATE_SERVER
int lua_network_getPeerSteamID(lua_State* state) {
	unsigned int peerID = luaL_checkinteger(state, 1);

	if (peerID >= peers.size()) {
		lua_pushstring(state, "bad peer ID");
		lua_error(state);
		return 0;
	}

	ENetPeer* peer = peers.at(peerID);

	if (peer == NULL) {
		lua_pushstring(state, "can't find peer");
		lua_error(state);
		return 0;
	}

	auto peerData = (PeerData*)peer->data;

	lua_pushstring(state, peerData->steamID.c_str());

	return 1;
}

int lua_network_setPeerSteamID(lua_State* state) {
	unsigned int peerID = luaL_checkinteger(state, 1);

	if (peerID >= peers.size()) {
		lua_pushstring(state, "bad peer ID");
		lua_error(state);
		return 0;
	}

	ENetPeer* peer = peers.at(peerID);

	if (peer == NULL) {
		lua_pushstring(state, "can't find peer");
		lua_error(state);
		return 0;
	}

	auto peerData = (PeerData*)peer->data;

	peerData->steamID = luaL_checkstring(state, 2);

	return 0;
}
#endif

int lua_network_pollForEvents(lua_State* state) {
	eventMutex.lock();
	for(auto event : events) {
		PeerData* peerData;

		switch (event.type) {
		case ENET_EVENT_TYPE_CONNECT:
			peerData = new PeerData();

#ifdef VOXELATE_SERVER
			peers.push_back(event.peer);

			peerData->steamID = "STEAM:0:0";
			peerData->peerID = peers.size();
#else
			peerData->steamID = "STEAM:0:0";
			peerData->peerID = 0;
#endif

			event.peer->data = peerData;

			LuaHelpers::PushHookRun(state->luabase, "VoxNetworkConnect");

			lua_pushnumber(state, peerData->peerID);
			lua_pushnumber(state, event.peer->address.host);

			LuaHelpers::CallHookRun(state->luabase, 2, 0);
			
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			peerData = (PeerData*)event.peer->data;

			LuaHelpers::PushHookRun(state->luabase, "VoxNetworkPacket");

			lua_pushnumber(state, peerData->peerID);
			lua_pushlstring(state, (char *)event.packet->data, event.packet->dataLength);
			lua_pushnumber(state, event.channelID);

			LuaHelpers::CallHookRun(state->luabase, 3, 0);

			enet_packet_destroy(event.packet);

			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			peerData = (PeerData*)event.peer->data;

			LuaHelpers::PushHookRun(state->luabase, "VoxNetworkDisconnect");

			lua_pushnumber(state, peerData->peerID);

			LuaHelpers::CallHookRun(state->luabase, 1, 0);

			event.peer->data = NULL;
		}
	}
	eventMutex.unlock();

	return 0;
}

void setupLuaNetworking(lua_State* state) {
#ifdef VOXELATE_CLIENT
	lua_pushcfunction(state, lua_network_connect);
	lua_setfield(state, -2, "networkConnect");
#endif

	lua_pushcfunction(state, lua_network_pollForEvents);
	lua_setfield(state, -2, "networkPoll");
}
