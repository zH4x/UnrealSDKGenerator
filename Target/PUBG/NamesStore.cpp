#include <windows.h>

#include "PatternFinder.hpp"
#include "NamesStore.hpp"

#include "EngineClasses.hpp"

class FNameEntry
{
public:
	__int32 Index;
	char pad_0x0004[0xC];
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
		return index >= 0 && index < Num() && GetById(index) != nullptr;
	}

	ElementType const* const& GetById(int32_t index) const
	{
		return *GetItemPtr(index);
	}

private:
	ElementType const* const* GetItemPtr(int32_t Index) const
	{
		int32_t ChunkIndex = Index / ElementsPerChunk;
		int32_t WithinChunkIndex = Index % ElementsPerChunk;
		ElementType** Chunk = Chunks[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

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
	const auto address = FindPattern(GetModuleHandleW(nullptr), reinterpret_cast<const unsigned char*>("\xE8\x00\x00\x00\x00\x4C\x8B\xC0\x8B\x03"), "x????xxxxx");

	if (address == -1)
	{
		return false;
	}

	using GetGNamesPtrFn = void*(*)();
	using GetPointerFn = TNameEntryArray*(*)(intptr_t);
	using GetFnPointerTableFn = GetPointerFn*(*)();

	auto offset = *reinterpret_cast<int32_t*>(address + 0x1);
	const auto GetGNamesPointer = reinterpret_cast<GetGNamesPtrFn>(address + offset + 0x5);

	offset = *reinterpret_cast<int32_t*>(address + 0x62);
	const auto GetFnPointerTable = *reinterpret_cast<GetFnPointerTableFn>(address + offset + 0x66);

	offset = *reinterpret_cast<int32_t*>(address + 0x77);
	const auto FnPointerTable = reinterpret_cast<GetPointerFn*>(address + offset + 0x7B);

	const auto rol = [](uint8_t value, uint8_t count)
	{
		count &= 0x07;

		auto high = value >> (8 - count);

		value <<= count;
		value |= high;

		return value;
	};

	const auto GetNameArray = [&]()
	{
		auto namesPtr = GetGNamesPointer();

		GetPointerFn* table;
		if (GetFnPointerTable)
		{
			table = GetFnPointerTable();
		}
		else
		{
			table = FnPointerTable;
		}

		auto v7 = *static_cast<uint16_t*>(namesPtr);
		auto v8 = *(static_cast<intptr_t*>(namesPtr) + 1);

		auto index = rol(LOBYTE(v7) ^ HIBYTE(v7), LOBYTE(v7)) & 0x1F;

		return table[index](v8);
	};

	//GlobalNames = GetNameArray();

	const auto nameEntryArrayPtrAddress = (intptr_t)GetModuleHandleW(nullptr) + 0x3E6E7A0;
	auto nameEntryArrayPtr = *(std::intptr_t*)(nameEntryArrayPtrAddress);
	nameEntryArrayPtr = *(std::intptr_t*)(nameEntryArrayPtr + 0xF8);
	nameEntryArrayPtr = nameEntryArrayPtr + 0x530;

	GlobalNames = (decltype(GlobalNames))nameEntryArrayPtr;

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
	return GlobalNames->GetById(static_cast<int32_t>(id))->GetName();
}
