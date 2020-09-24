#include <windows.h>

#include "PatternFinder.hpp"
#include "ObjectsStore.hpp"

#include "EngineClasses.hpp"

// This class contains the class which allows the generator access to global objects list.

class FUObjectItem
{
public:
	UObject* Object; // 0x0000-0x0008
	__int32 Flags; // 0x0008-0x000C
	__int32 ClusterIndex; // 0x000C-0x0010
	__int32 SerialNumber; // 0x0010-0x0014
	unsigned char pad__0004[0x0004]; // 0x0014-0x0018
};

class TUObjectArray
{
public:
	FUObjectItem* Objects;
	int32_t MaxElements;
	int32_t NumElements;
};

class FUObjectArray
{
public:
	__int32 ObjFirstGCIndex; //0x0000
	__int32 ObjLastNonGCIndex; //0x0004
	__int32 MaxObjectsNotConsideredByGC; //0x0008
	__int32 OpenForDisregardForGC; //0x000C

	TUObjectArray ObjObjects; //0x0010
};

FUObjectArray* GlobalObjects = nullptr;

bool ObjectsStore::Initialize()
{	
	GlobalObjects = reinterpret_cast<decltype(GlobalObjects)>(reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr)) + 0x6B061B8);
	return true;
}

void* ObjectsStore::GetAddress()
{
	return GlobalObjects;
}

size_t ObjectsStore::GetObjectsNum() const
{
	return GlobalObjects->ObjObjects.NumElements;
}

UEObject ObjectsStore::GetById(size_t id) const
{
	return GlobalObjects->ObjObjects.Objects[id].Object;
}
