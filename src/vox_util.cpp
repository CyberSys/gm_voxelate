#include "vox_util.h"

#include "tier0/dbg.h"
#include "Color.h"

Color COLOR_LIME(0, 255, 0, 255);

void vox_print(const char* msg, ...) {
	char buffer[256];
	
	va_list args;
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);
	
	ConMsg("[");
	ConColorMsg(COLOR_LIME, "VOX");
	ConMsg("] ");
	ConMsg(buffer);
	ConMsg("\n");
}

Vector eent_getPos(CBaseEntity* ent) {
	byte* pos_ptr = reinterpret_cast<byte*>(ent)+772;
	return *reinterpret_cast<Vector*>(pos_ptr);
}