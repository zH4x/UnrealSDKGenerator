#include <windows.h>
#include "PatternFinder.hpp"
#include "ObjectsStore.hpp"
#include "EngineClasses.hpp"

class FUObjectItem
{
public:
	UObject* Object;
	__int32 Flags;
	__int32 ClusterIndex;
	__int32 SerialNumber;

	enum class EInternalObjectFlags : int32_t
	{
		None = 0,
		Native = 1 << 25,
		Async = 1 << 26,
		AsyncLoading = 1 << 27,
		Unreachable = 1 << 28,
		PendingKill = 1 << 29,
		RootSet = 1 << 30,
		NoStrongReference = 1 << 31
	};

	inline bool IsUnreachable() const
	{
		return !!(Flags & static_cast<std::underlying_type_t<EInternalObjectFlags>>(EInternalObjectFlags::Unreachable));
	}

	inline bool IsPendingKill() const
	{
		return !!(Flags & static_cast<std::underlying_type_t<EInternalObjectFlags>>(EInternalObjectFlags::PendingKill));
	}

};

class TUObjectArray
{
public:
	inline int32_t Num() const
	{
		return NumElements;
	}

	inline UObject* GetByIndex(int32_t index) const
	{
		return Objects[index].Object;
	}

	inline FUObjectItem* GetItemByIndex(int32_t index) const
	{
		if (index < NumElements)
		{
			return &Objects[index];
		}
		return nullptr;
	}

	FUObjectItem* Objects;
	int32_t MaxElements;
	int32_t NumElements;
};

class FUObjectArray
{
public:
	TUObjectArray ObjObjects;
};

FUObjectArray* GlobalObjects = nullptr;

bool ObjectsStore::Initialize()
{
	GlobalObjects = reinterpret_cast<decltype(GlobalObjects)>(reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr)) + 0x543DFB0);
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
	return GlobalObjects->ObjObjects.GetByIndex(static_cast<int32_t>(id));
}
