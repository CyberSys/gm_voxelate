#pragma once

#include <map>
#include <set>
#include <list>

#include "materialsystem/imesh.h"

#include "glua.h"

#include "vox_util.h"

#define VOXEL_CHUNK_SIZE 16

class CPhysPolysoup;
class IPhysicsObject;
class CPhysCollide;

enum VoxelForm {
	VFORM_NULL,
	VFORM_CUBE
};

struct AtlasPos {
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
	~VoxelConfig() {
		if (atlasMaterial)
			atlasMaterial->DecrementReferenceCount();
	}

	int dims_x = 1;
	int dims_y = 1;
	int dims_z = 1;

	bool huge = false;

	double scale = 1;

	bool buildPhysicsMesh = false;
	bool buildExterior = false;

	IMaterial* atlasMaterial = nullptr;

	int atlasWidth = 1;
	int atlasHeight = 1;

	double _padding_x = 0;
	double _padding_y = 0;

	VoxelType voxelTypes[256];
};

class Voxels;
class VoxelChunk;

int newIndexedVoxels(int index = -1);
Voxels* getIndexedVoxels(int index);
void deleteIndexedVoxels(int index);
void deleteAllIndexedVoxels();

class Voxels {
	friend class VoxelChunk;
public:
	~Voxels();

	VoxelChunk* addChunk(int chunk_num);
	VoxelChunk* getChunk(int x, int y, int z);

	const int getChunkData(int chunk_num, char* out);
	void setChunkData(int chunk_num, const char* data_compressed, int data_len);

	void initialize(VoxelConfig* config);
	bool isInitialized();

	Vector getExtents();
	void getCellExtents(int& x, int &y, int &z);

	void flagAllChunksForUpdate();

	void doUpdates(int count, CBaseEntity* ent);
	void enableUpdates(bool enable);

	VoxelTraceRes doTrace(Vector startPos, Vector delta);
	VoxelTraceRes doTraceHull(Vector startPos, Vector delta, Vector extents);

	VoxelTraceRes iTrace(Vector startPos, Vector delta, Vector defNormal);
	VoxelTraceRes iTraceHull(Vector startPos, Vector delta, Vector extents, Vector defNormal);

	void draw();

	uint16 get(int x, int y, int z);
	bool set(int x, int y, int z, uint16 d,bool flagChunks=true);
private:
	VoxelChunk** chunks = nullptr;
	std::set<VoxelChunk*> chunks_flagged_for_update;

	bool updates_enabled = false;

	VoxelConfig* config = nullptr;
};


class VoxelChunk {
public:
	VoxelChunk(Voxels* sys, int x, int y, int z);
	~VoxelChunk();
	void build(CBaseEntity* ent);
	void draw(CMatRenderContextPtr& pRenderContext);

	uint16 get(int x, int y, int z);
	void set(int x, int y, int z, uint16 d, bool flagChunks);

	uint16 voxel_data[VOXEL_CHUNK_SIZE*VOXEL_CHUNK_SIZE*VOXEL_CHUNK_SIZE] = {};
private:
	void meshClearAll();

	void meshStart();
	void meshStop(CBaseEntity* ent);

	void addFullVoxelFace(int x,int y,int z,int tx, int ty, byte dir);

	int posX, posY, posZ;

	Voxels* system;
	CMeshBuilder meshBuilder;
	IMesh* current_mesh;
	std::list<IMesh*> meshes;
	int verts_remaining;

	CPhysPolysoup* phys_soup;
	IPhysicsObject* phys_obj = nullptr;
	CPhysCollide* phys_collider = nullptr;
};