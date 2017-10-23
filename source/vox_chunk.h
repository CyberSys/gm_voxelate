#pragma once

#include "vox_world_common.h"

#include "vox_worldgen_basic.h"

class VoxelWorld;

// Used for building chunk slice meshes.
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

	void build(CBaseEntity* ent, ELevelOfDetail lod);
	void draw(CMatRenderContextPtr& pRenderContext);

	VoxelCoordXYZ getWorldCoords();

	BlockData get(int x, int y, int z);
	void set(int x, int y, int z, BlockData d, bool flagChunks);

	int posX, posY, posZ;

	BlockData voxel_data[VOXEL_CHUNK_SIZE*VOXEL_CHUNK_SIZE*VOXEL_CHUNK_SIZE] = {0};
private:
	void physicsMeshClearAll();

	void physicsMeshStart();
	void physicsMeshStop(CBaseEntity* ent);

#ifdef VOXELATE_CLIENT
	void graphicsMeshClearAll();

	void graphicsMeshStart();
	void graphicsMeshStop();
#endif
	
	void buildSlice(int slice, byte dir, SliceFace faces[VOXEL_CHUNK_SIZE][VOXEL_CHUNK_SIZE], int upper_bound_x, int upper_bound_y, int scaling);

	void addSliceFace(int slice, int x, int y, int w, int h, int tx, int ty, byte dir, int scale);

	VoxelWorld* world;
	CMeshBuilder meshBuilder;
	IMesh* current_mesh = nullptr;
	std::list<IMesh*> meshes;
	int graphics_vertsRemaining = 0;

	CPhysPolysoup* phys_soup = nullptr;
	IPhysicsObject* phys_obj = nullptr;
	CPhysCollide* phys_collider = nullptr;

	// bullet physics
	
	btTriangleMesh* meshInterface = nullptr;
	btRigidBody* chunkBody = nullptr;
};

