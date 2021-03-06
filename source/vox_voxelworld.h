#pragma once

#include <map>
#include <set>
#include <unordered_map>
#include <list>
#include <array>
#include <functional>
#include <vector>
#include <string>
#include <deque>

#include "materialsystem/imesh.h"

#include "glua.h"

#include "vox_util.h"

typedef uint16 BlockData;
typedef std::int32_t Coord;
typedef std::array<Coord, 3> XYZCoordinate;

// custom specialization of std::hash can be injected in namespace std
// thanks zerf
namespace std {
	template<> struct hash<XYZCoordinate> {
		std::size_t operator()(XYZCoordinate const& a) const {
			std::size_t h = 2166136261;

			for (const std::int32_t& e : a) {
				h = (h ^ (e >> 24)) * 16777619;
				h = (h ^ ((e >> 16) & 0xff)) * 16777619;
				h = (h ^ ((e >> 8) & 0xff)) * 16777619;
				h = (h ^ (e & 0xff)) * 16777619;
			}

			return h;
		}
	};
};

#define VOXEL_CHUNK_SIZE 16

class CPhysPolysoup;
class IPhysicsObject;
class CPhysCollide;

enum VoxelForm {
	VFORM_NULL,
	VFORM_CUBE
};

struct AtlasPos {
	AtlasPos() {};
	AtlasPos(int x, int y) { this->x = x; this->y = y; }
	int x;
	int y;
};

struct VoxelType {
	VoxelForm form = VFORM_NULL;
	AtlasPos side_xPos = AtlasPos(0, 0);
	AtlasPos side_xNeg = AtlasPos(0, 0);
	AtlasPos side_yPos = AtlasPos(0, 0);
	AtlasPos side_yNeg = AtlasPos(0, 0);
	AtlasPos side_zPos = AtlasPos(0, 0);
	AtlasPos side_zNeg = AtlasPos(0, 0);
};

struct VoxelTraceRes {
	double fraction = -1;
	Vector hitPos;
	Vector hitNormal = Vector(0,0,0);
	VoxelTraceRes& operator*(double n) { hitPos *= n; return *this; }
};

struct VoxelConfigClient;
struct VoxelConfigServer;

struct VoxelConfig {

	int dims_x = VOXEL_CHUNK_SIZE;
	int dims_y = VOXEL_CHUNK_SIZE;
	int dims_z = VOXEL_CHUNK_SIZE;

	bool huge = false;

	double scale = 32;

	bool buildPhysicsMesh = false;
	bool buildExterior = false;

	IMaterial* atlasMaterial = nullptr;

	int atlasWidth = 1;
	int atlasHeight = 1;

	double _padding_x = 0;
	double _padding_y = 0;

	VoxelType voxelTypes[256];
};

class VoxelWorld;
class VoxelChunk;

int newIndexedVoxelWorld(int index, VoxelConfig& config);

VoxelWorld* getIndexedVoxelWorld(int index);

void deleteIndexedVoxelWorld(int index);
void checkAllVoxelWorldsDeleted();

void voxelworld_initialise_networking_static();

class VoxelWorld {
	friend class VoxelChunk;
public:
	VoxelWorld(VoxelConfig& config);
	~VoxelWorld();

	int worldID = -1;

	VoxelChunk* initChunk(Coord x, Coord y, Coord z);
	VoxelChunk* getChunk(Coord x, Coord y, Coord z);

	const int getChunkData(Coord x, Coord y, Coord z, char * out);
	bool setChunkData(Coord x, Coord y, Coord z, const char* data_compressed, int data_len);

	bool loadFromString(std::string contents);

	std::tuple<char*, size_t> writeToString();

	void initialize();

	Vector getExtents();
	void getCellExtents(Coord& x, Coord &y, Coord &z);

	std::vector<XYZCoordinate> getAllChunkPositions(Vector origin);

#ifdef VOXELATE_SERVER
	bool sendChunk(int peerID, XYZCoordinate pos);
	bool sendChunksAround(int peerID, XYZCoordinate pos, Coord radius = 10);
#endif

	void sortUpdatesByDistance(Vector * origin);
	void doUpdates(int count, CBaseEntity * ent);

	VoxelTraceRes doTrace(Vector startPos, Vector delta);
	VoxelTraceRes doTraceHull(Vector startPos, Vector delta, Vector extents);

	VoxelTraceRes iTrace(Vector startPos, Vector delta, Vector defNormal);
	VoxelTraceRes iTraceHull(Vector startPos, Vector delta, Vector extents, Vector defNormal);

	void draw();

	BlockData get(Coord x, Coord y, Coord z);
	bool set(Coord x, Coord y, Coord z, BlockData d,bool flagChunks=true);

	//bool trackUpdates = false;
	//std::vector<XYZCoordinate> queued_block_updates;
private:
	//bool initialised = false;

	std::unordered_map<XYZCoordinate, VoxelChunk*> chunks_map; // ok zerf lmao

	void flagChunk(XYZCoordinate chunk_pos, bool high_priority);

	std::deque<XYZCoordinate> dirty_chunk_queue;
	std::set<XYZCoordinate> dirty_chunk_set;

	VoxelConfig config;
};

struct SliceFace {
	bool present;
#ifdef VOXELATE_CLIENT
	bool direction;
	AtlasPos texture;

	bool operator== (const SliceFace& other) const {
		return
			present == other.present &&
			direction == other.direction &&
			texture.x == other.texture.x &&
			texture.y == other.texture.y;
	}
#else
	bool operator== (const SliceFace& other) const {
		return present == other.present;
	}
#endif
};

class VoxelChunk {
public:
	VoxelChunk(VoxelWorld* sys, int x, int y, int z);
	~VoxelChunk();

	void generate();

	void build(CBaseEntity* ent);
	void draw(CMatRenderContextPtr& pRenderContext);

	XYZCoordinate getWorldCoords();

	BlockData get(int x, int y, int z);
	void set(int x, int y, int z, BlockData d, bool flagChunks);

	int posX, posY, posZ;

	BlockData voxel_data[VOXEL_CHUNK_SIZE*VOXEL_CHUNK_SIZE*VOXEL_CHUNK_SIZE] = {};
private:
	void meshClearAll();

	void meshStart();
	void meshStop(CBaseEntity* ent);
	
	void buildSlice(int slice, byte dir, SliceFace faces[VOXEL_CHUNK_SIZE][VOXEL_CHUNK_SIZE], int upper_bound_x, int upper_bound_y);

	void addFullVoxelFace(int x,int y,int z,int tx, int ty, byte dir);
	void addSliceFace(int slice, int x, int y, int w, int h, int tx, int ty, byte dir);

	VoxelWorld* system;
	CMeshBuilder meshBuilder;
	IMesh* current_mesh = nullptr;
	std::list<IMesh*> meshes;
	int verts_remaining = 0;

	CPhysPolysoup* phys_soup = nullptr;
	IPhysicsObject* phys_obj = nullptr;
	CPhysCollide* phys_collider = nullptr;
};