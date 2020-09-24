#include <windows.h>

#include "PatternFinder.hpp"
#include "NamesStore.hpp"

#include "EngineClasses.hpp"

// This class contains the class which allows the generator access to global names list.

class FNameEntry
{
public:
	int32_t Index;
	char pad_0x0004[0x4];
	FNameEntry* HashNext;
	union
	{
		char AnsiName[1024];
		wchar_t WideName[1024];
	};

	const char* GetName() const
	{
		return AnsiName;
	}
};

template<typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
class TStaticIndirectArrayThreadSafeRead
{
public:
	int32_t Num() const
	{
		return NumElements;
	}

	bool IsValidIndex(int32_t index) const
	{
		return GetById(index) != nullptr;
	}

	ElementType* GetById(int32_t index) const
	{
		if (index < 0 || index >= Num()) return nullptr;

		int32_t ChunkIndex = index / ElementsPerChunk;
		int32_t WithinChunkIndex = index % ElementsPerChunk;

		if (ChunkIndex < 0 || ChunkIndex >= NumChunks) return nullptr;

		ElementType** Chunk = Chunks[ChunkIndex];

		if (!Chunk) return nullptr;

		return Chunk[WithinChunkIndex];
	}

private:
	enum
	{
		ChunkTableSize = (MaxTotalElements + ElementsPerChunk - 1) / ElementsPerChunk
	};

	ElementType** Chunks[ChunkTableSize];
	__int32 NumElements;
	__int32 NumChunks;
};

using TNameEntryArray = TStaticIndirectArrayThreadSafeRead<FNameEntry, 2 * 1024 * 1024, 16384>;

TNameEntryArray* GlobalNames = nullptr;

bool NamesStore::Initialize()
{
	GlobalNames = *(reinterpret_cast<TNameEntryArray**>(reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr)) + 0x6B01B00));
	return true;
}

void* NamesStore::GetAddress()
{
	return GlobalNames;
}

size_t NamesStore::GetNamesNum() const
{
	return GlobalNames->Num();
}

bool NamesStore::IsValid(size_t id) const
{
	return GlobalNames->IsValidIndex(static_cast<int32_t>(id));
}

std::string NamesStore::GetById(size_t id) const
{
	auto name = GlobalNames->GetById(static_cast<int32_t>(id));

	if (name) return name->GetName();

	return "__UNKNOWN_NAME__";
}
