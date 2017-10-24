#include "vox_lua_advanced_vector.hpp"

static uint8_t metatype = GarrysMod::Lua::Type::NONE;
static const char *metaname = "AdvancedVector";

#if defined _WIN32
const char *tostring_format = "%s: [%f, %f, %f]";
#elif defined __linux || defined __APPLE__
const char *tostring_format = "%s: [%f, %f, %f]";
#endif

#define LUA state->luabase

VectorPtr* vox_lua_pushVector(lua_State* state, btVector3* vec) {
	VectorPtr* udata = LUA->NewUserType<VectorPtr>(metatype);

	udata->reset(vec);

	LUA->PushMetaTable(metatype);
	LUA->SetMetaTable(-2);

	return udata;
}

VectorPtr* vox_lua_pushVectorCopy(lua_State* state, btVector3 vecSrc) {
	auto vec = new btVector3(vecSrc); // fuck RAII amirite

	VectorPtr* udata = LUA->NewUserType<VectorPtr>(metatype);

	udata->reset(vec);

	LUA->PushMetaTable(metatype);
	LUA->SetMetaTable(-2);

	return udata;
}

VectorPtr* vox_lua_getVectorPtr(lua_State* state, int32_t index) {
	if (!LUA->IsType(index, metatype)) {
		luaL_typerror(LUA->GetState(), index, metaname);
	}

	return LUA->GetUserType<VectorPtr>(index, metatype);
}

btVector3* vox_lua_getVector(lua_State* state, int32_t index) {
	if (!LUA->IsType(index, metatype)) {
		luaL_typerror(LUA->GetState(), index, metaname);
	}

	auto vecPtr = LUA->GetUserType<VectorPtr>(index, metatype);

	if (vecPtr != NULL) {
		return vecPtr->get();
	}

	return NULL;
}

int vox_lua_vector_gc(lua_State* state) {
	auto vecptr = vox_lua_getVectorPtr(state, 1);

	vecptr->reset();
	LUA->SetUserType(1, nullptr);

	return 0;
}

int vox_lua_vector_tostring(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	lua_pushfstring(state, tostring_format, metaname, vecptr->x(), -vecptr->z(), vecptr->y());

	return 1;
}

int vox_lua_vector_toSourceVector(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	lua_getglobal(state, "Vector");

	lua_pushnumber(state, vecptr->x());
	lua_pushnumber(state, -vecptr->z());
	lua_pushnumber(state, vecptr->y());

	lua_call(state, 3, 1);

	return 1;
}

int vox_lua_vector_setAllValues(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	double x = luaL_checknumber(state, 2);
	double y = luaL_checknumber(state, 3);
	double z = luaL_checknumber(state, 4);

	vecptr->setX(x);
	vecptr->setY(y);
	vecptr->setZ(z);

	return 0;
}

int vox_lua_vector_setToZero(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	vecptr->setZero();

	return 0;
}

int vox_lua_vector_length(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	lua_pushnumber(state, vecptr->length());

	return 1;
}

int vox_lua_vector_lengthSquare(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	lua_pushnumber(state, vecptr->length2());

	return 1;
}

int vox_lua_vector_getUnit(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	auto result = vecptr->normalized();
	vox_lua_pushVectorCopy(state, result);

	return 1;
}

int vox_lua_vector_isUnit(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	lua_pushboolean(state, vecptr->length2() == 1);

	return 1;
}

int vox_lua_vector_isZero(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	lua_pushboolean(state, vecptr->isZero());

	return 1;
}

int vox_lua_vector_dot(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);
	auto vecptr2 = vox_lua_getVector(state, 2);

	lua_pushnumber(state, vecptr->dot(*vecptr2));

	return 1;
}

int vox_lua_vector_cross(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);
	auto vecptr2 = vox_lua_getVector(state, 2);

	auto result = vecptr->cross(*vecptr2);

	vox_lua_pushVectorCopy(state, result);

	return 1;
}

int vox_lua_vector_normalize(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	vecptr->normalize();

	return 0;
}

int vox_lua_vector_getAbsoluteVector(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	auto result = vecptr->absolute();
	vox_lua_pushVectorCopy(state, result);

	return 0;
}

int vox_lua_vector_getMinAxis(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	lua_pushnumber(state, vecptr->minAxis());

	return 1;
}

int vox_lua_vector_getMaxAxis(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	lua_pushnumber(state, vecptr->maxAxis());

	return 1;
}

int vox_lua_vector_eq(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	if (!LUA->IsType(2, metatype)) {
		lua_pushboolean(state, false);
		return 1;
	}

	auto vecptr2 = vox_lua_getVector(state, 2);

	lua_pushboolean(state, vecptr == vecptr2);

	return 1;
}

int vox_lua_vector_neq(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	if (!LUA->IsType(2, metatype)) {
		lua_pushboolean(state, false);
		return 1;
	}

	auto vecptr2 = vox_lua_getVector(state, 2);

	lua_pushboolean(state, vecptr != vecptr2);

	return 1;
}

int vox_lua_vector_add(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);
	auto vecptr2 = vox_lua_getVector(state, 2);

	auto result = *vecptr + *vecptr2;
	vox_lua_pushVectorCopy(state, result);

	return 1;
}

int vox_lua_vector_sub(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);
	auto vecptr2 = vox_lua_getVector(state, 2);

	auto result = *vecptr - *vecptr2;
	vox_lua_pushVectorCopy(state, result);

	return 1;
}

int vox_lua_vector_mul(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	if (lua_isnumber(state, 2)) {
		double num = lua_tonumber(state, 2);

		auto result = *vecptr * num;
		vox_lua_pushVectorCopy(state, result);
	}
	else {
		auto vecptr2 = vox_lua_getVector(state, 2);

		auto result = *vecptr * *vecptr2;
		vox_lua_pushVectorCopy(state, result);
	}

	return 1;
}

int vox_lua_vector_div(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	if (lua_isnumber(state, 2)) {
		double num = lua_tonumber(state, 2);

		auto result = *vecptr / num;
		vox_lua_pushVectorCopy(state, result);
	}
	else {
		auto vecptr2 = vox_lua_getVector(state, 2);

		auto result = *vecptr / *vecptr2;
		vox_lua_pushVectorCopy(state, result);
	}

	return 1;
}

int vox_lua_vector_index(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	if (lua_isnumber(state, 2)) {
		auto idx = lua_tonumber(state, 2);

		if (idx == 1) {
			lua_pushnumber(state, vecptr->x());
			return 1;
		}
		else if (idx == 2) {
			lua_pushnumber(state, -vecptr->z());
			return 1;
		}
		else if (idx == 3) {
			lua_pushnumber(state, vecptr->y());
			return 1;
		}
	}

	if (lua_isstring(state, 2)) {
		auto idx = lua_tostring(state, 2);

		if (strcmp(idx, "x") == 0) {
			lua_pushnumber(state, vecptr->x());
			return 1;
		}
		else if (strcmp(idx, "y") == 0) {
			lua_pushnumber(state, -vecptr->z());
			return 1;
		}
		else if (strcmp(idx, "z") == 0) {
			lua_pushnumber(state, vecptr->y());
			return 1;
		}

		LUA->PushMetaTable(metatype);

		lua_getfield(state, -1, idx);
	}
	else {
		lua_pushnil(state);
	}

	return 1;
}

int vox_lua_vector_newindex(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	if (!lua_isnumber(state, 2)) {
		lua_pushstring(state, "bad newindex on AdvancedVector (value is not a number)");
		lua_error(state);
	}

	auto val = lua_tonumber(state, 3);

	if (lua_isnumber(state, 2)) {
		auto idx = lua_tonumber(state, 2);

		if (idx == 1) {
			vecptr->setX(val);
			return 0;
		}
		else if (idx == 2) {
			vecptr->setZ(-val);
			return 0;
		}
		else if (idx == 3) {
			vecptr->setY(val);
			return 0;
		}

		lua_pushstring(state, "bad newindex on AdvancedVector (index not equal to 1, 2 or 3)");
		lua_error(state);
	}

	if (lua_isstring(state, 2)) {
		auto idx = lua_tostring(state, 2);

		if (strcmp(idx, "x") == 0) {
			vecptr->setX(val);
			return 0;
		}
		else if (strcmp(idx, "y") == 0) {
			vecptr->setZ(-val);
			return 0;
		}
		else if (strcmp(idx, "z") == 0) {
			vecptr->setY(val);
			return 0;
		}

		lua_pushstring(state, "bad newindex on AdvancedVector (index not equal to x, y or z)");
		lua_error(state);
	}
	else {
		lua_pushstring(state, "bad newindex on AdvancedVector (index not string or number)");
		lua_error(state);
	}

	return 0;
}

int vox_lua_vector_lt(lua_State* state) {
	auto vecptr = vox_lua_getVector(state, 1);

	auto vecptr2 = vox_lua_getVector(state, 2);

	lua_pushboolean(state, vecptr < vecptr2);

	return 1;
}

int vox_lua_vector_ctor(lua_State* state) {
	double x = luaL_checknumber(state, 1);
	double y = luaL_checknumber(state, 2);
	double z = luaL_checknumber(state, 3);

	auto vec = new btVector3(x, z, -y);

	vox_lua_pushVector(state, vec);

	return 1;
}

int vox_lua_vector_fromSource(lua_State* state) {
	lua_getfield(state, 1, "x");
	float x = luaL_checknumber(state, -1);
	lua_pop(state, 1);

	lua_getfield(state, 1, "y");
	float y = luaL_checknumber(state, -1);
	lua_pop(state, 1);

	lua_getfield(state, 1, "z");
	float z = luaL_checknumber(state, -1);
	lua_pop(state, 1);

	auto vec = new btVector3(x, z, -y);

	vox_lua_pushVector(state, vec);

	return 1;
}

btVector3 luaL_checkbtVector3(lua_State* state, int loc) {
	return *vox_lua_getVector(state, loc);
}

void luaL_pushbtVector3(lua_State* state, btVector3 vecSrc) {
	auto vec = new btVector3(vecSrc);
	vox_lua_pushVector(state, vec);
}

void setupLuaAdvancedVectors(lua_State * state) {
	metatype = LUA->CreateMetaTable(metaname);
	
	lua_pushcfunction(state, vox_lua_vector_gc);
	lua_setfield(state, -2, "__gc");

	lua_pushcfunction(state, vox_lua_vector_tostring);
	lua_setfield(state, -2, "__tostring");

	lua_pushcfunction(state, vox_lua_vector_setAllValues);
	lua_setfield(state, -2, "setAllValues");

	lua_pushcfunction(state, vox_lua_vector_setToZero);
	lua_setfield(state, -2, "setToZero");

	lua_pushcfunction(state, vox_lua_vector_length);
	lua_setfield(state, -2, "length");

	lua_pushcfunction(state, vox_lua_vector_lengthSquare);
	lua_setfield(state, -2, "lengthSquare");

	lua_pushcfunction(state, vox_lua_vector_getUnit);
	lua_setfield(state, -2, "getUnit");

	lua_pushcfunction(state, vox_lua_vector_isUnit);
	lua_setfield(state, -2, "isUnit");

	lua_pushcfunction(state, vox_lua_vector_isZero);
	lua_setfield(state, -2, "isZero");

	lua_pushcfunction(state, vox_lua_vector_dot);
	lua_setfield(state, -2, "dot");

	lua_pushcfunction(state, vox_lua_vector_cross);
	lua_setfield(state, -2, "cross");

	lua_pushcfunction(state, vox_lua_vector_normalize);
	lua_setfield(state, -2, "normalize");

	lua_pushcfunction(state, vox_lua_vector_getAbsoluteVector);
	lua_setfield(state, -2, "getAbsoluteVector");

	lua_pushcfunction(state, vox_lua_vector_getMinAxis);
	lua_setfield(state, -2, "getMinAxis");

	lua_pushcfunction(state, vox_lua_vector_getMaxAxis);
	lua_setfield(state, -2, "getMaxAxis");

	lua_pushcfunction(state, vox_lua_vector_eq);
	lua_setfield(state, -2, "__eq");

	lua_pushcfunction(state, vox_lua_vector_neq);
	lua_setfield(state, -2, "__neq");

	lua_pushcfunction(state, vox_lua_vector_add);
	lua_setfield(state, -2, "__add");

	lua_pushcfunction(state, vox_lua_vector_sub);
	lua_setfield(state, -2, "__sub");

	lua_pushcfunction(state, vox_lua_vector_mul);
	lua_setfield(state, -2, "__mul");

	lua_pushcfunction(state, vox_lua_vector_div);
	lua_setfield(state, -2, "__div");

	lua_pushcfunction(state, vox_lua_vector_index);
	lua_setfield(state, -2, "__index");

	lua_pushcfunction(state, vox_lua_vector_newindex);
	lua_setfield(state, -2, "__newindex");

	lua_pushcfunction(state, vox_lua_vector_lt);
	lua_setfield(state, -2, "__lt");
	
	LUA->Pop(1);

	lua_pushcfunction(state, vox_lua_vector_ctor);
	lua_setglobal(state, "AdvancedVector");

	lua_pushcfunction(state, vox_lua_vector_fromSource);
	lua_setglobal(state, "AdvancedVectorFromSource");
}
