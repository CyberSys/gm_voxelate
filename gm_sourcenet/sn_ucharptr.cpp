#include "sn_bf_common.hpp"
#include <sn_ucharptr.hpp>
#include <cstring>

namespace UCHARPTR
{

struct Container
{
	uint8_t *data;
	int32_t bits;
	bool own;
};

uint8_t metatype = GarrysMod::Lua::Type::NONE;
const char *metaname = "UCHARPTR";

uint8_t *Push( GarrysMod::Lua::ILuaBase *LUA, int32_t bits, uint8_t *data )
{
	if( bits <= 0 )
		sn_bf_common::ThrowError(
			LUA,
			"invalid amount of bits for a buffer (%d is less than or equal to zero)",
			bits
		);

	bool own = false;
	if( data == nullptr )
	{
		own = true;
		data = new( std::nothrow ) uint8_t[( bits + 7 ) >> 3];
		if( data == nullptr )
			LUA->ThrowError( "failed to allocate buffer" );
	}

	Container *container = LUA->NewUserType<Container>( metatype );
	container->data = data;
	container->bits = bits;
	container->own = own;

	LUA->PushMetaTable( metatype );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( LUA->GetState(), -2 );

	return data;
}

inline Container *GetUserData( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
{
	sn_bf_common::CheckType( LUA, index, metatype, metaname );
	return LUA->GetUserType<Container>( index, metatype );
}

uint8_t *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t *bits, bool *own )
{
	Container *container = GetUserData( LUA, index );
	uint8_t *ptr = container->data;
	if( ptr == nullptr )
		sn_bf_common::ThrowError( LUA, "invalid %s", metaname );

	if( bits != nullptr )
		*bits = container->bits;

	if( own != nullptr )
		*own = container->own;

	return ptr;
}

uint8_t *Release( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t *bits )
{
	Container *container = GetUserData( LUA, index );
	uint8_t *ptr = container->data;
	if( ptr == nullptr )
		sn_bf_common::ThrowError( LUA, "invalid %s", metaname );

	if( bits != nullptr )
		*bits = container->bits;

	LUA->SetUserType( index, nullptr );

	return ptr;
}

LUA_FUNCTION_STATIC( gc )
{
	Container *container = GetUserData( LUA, 1 );

	if( container->own )
		delete[] container->data;

	LUA->SetUserType( 1, nullptr );

	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	uint8_t *ptr1 = Get( LUA, 1 );
	uint8_t *ptr2 = Get( LUA, 2 );

	LUA->PushBool( ptr1 == ptr2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	uint8_t *ptr = Get( LUA, 1 );

	lua_pushfstring( LUA->GetState(), sn_bf_common::tostring_format, metaname, ptr );

	return 1;
}

LUA_FUNCTION_STATIC( IsValid )
{
	sn_bf_common::CheckType( LUA, 1, metatype, metaname );

	LUA->PushBool( GetUserData( LUA, 1 )->data != nullptr );

	return 1;
}

LUA_FUNCTION_STATIC( Size )
{
	int32_t bits = 0;
	Get( LUA, 1, &bits );

	LUA->PushNumber( bits );

	return 1;
}

LUA_FUNCTION_STATIC(Resize)
{
	sn_bf_common::CheckType(LUA, 1, metatype, metaname);
	auto bytes = LUA->CheckNumber(2);
	auto bits = static_cast<int32_t>(bytes) * 8;

	Container *container = GetUserData(LUA, 1);


	if (bits <= 0)
		sn_bf_common::ThrowError(
			LUA,
			"invalid amount of bits for a buffer (%d is less than or equal to zero)",
			bits
		);

	auto data = new(std::nothrow) uint8_t[(bits + 7) >> 3];
	if (data == nullptr)
		LUA->ThrowError("failed to allocate buffer");

	int32_t cpsize = (bits + 7) >> 3;

	if (bits > container->bits) {
		cpsize = (container->bits + 7) >> 3;
	}
	
	memcpy(data, container->data, cpsize);

	container->data = data;
	container->bits = bits;

	return 0;
}

LUA_FUNCTION_STATIC( Copy )
{
	int32_t bits = 0;
	uint8_t *src = Get( LUA, 1, &bits );

	uint8_t *data = Push( LUA, bits );
	memcpy( data, src, ( bits + 7 ) >> 3 );

	return 1;
}

LUA_FUNCTION_STATIC(GetString)
{
	Container *container = GetUserData(LUA, 1);

	int size = ceil(container->bits / 8);

	std::string out(reinterpret_cast<char*>(container->data), size);

	LUA->PushString(out.c_str(), size);

	return 1;
}

LUA_FUNCTION_STATIC( Constructor )
{
	LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );

	Push( LUA, static_cast<int32_t>( LUA->GetNumber( 1 ) ) * 8 );

	return 1;
}

LUA_FUNCTION_STATIC(ConstructorFromString)
{
	LUA->CheckType(1, GarrysMod::Lua::Type::STRING);
	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);

	auto data = Push(LUA, static_cast<int32_t>(LUA->GetNumber(2)) * 8);

	memcpy(data, LUA->GetString(1), LUA->GetNumber(2));

	return 1;
}

void Initialize( GarrysMod::Lua::ILuaBase *LUA )
{
	metatype = LUA->CreateMetaTable( metaname );

		LUA->PushCFunction( gc );
		LUA->SetField( -2, "__gc" );

		LUA->PushCFunction( eq );
		LUA->SetField( -2, "__eq" );

		LUA->PushCFunction( tostring );
		LUA->SetField( -2, "__tostring" );

		LUA->PushCFunction(sn_bf_common::index );
		LUA->SetField( -2, "__index" );

		LUA->PushCFunction(sn_bf_common::newindex );
		LUA->SetField( -2, "__newindex" );

		LUA->PushCFunction(sn_bf_common::GetTable );
		LUA->SetField( -2, "GetTable" );

		LUA->PushCFunction( IsValid );
		LUA->SetField( -2, "IsValid" );

		LUA->PushCFunction(Size);
		LUA->SetField(-2, "Size");

		LUA->PushCFunction(Resize);
		LUA->SetField(-2, "Resize");

		LUA->PushCFunction( Copy );
		LUA->SetField( -2, "Copy" );

		LUA->PushCFunction(GetString);
		LUA->SetField(-2, "GetString");

	LUA->Pop( 1 );

	LUA->PushCFunction( Constructor );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );

	LUA->PushCFunction(ConstructorFromString);
	LUA->SetField(GarrysMod::Lua::INDEX_GLOBAL, "UCHARPTR_FromString");
}

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );
}

}
