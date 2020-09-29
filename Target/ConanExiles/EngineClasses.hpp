#pragma once

#include <set>
#include <string>
#include <windows.h>

typedef unsigned char 		uint8;		// 8-bit  unsigned.
typedef unsigned short int	uint16;		// 16-bit unsigned.
typedef unsigned int		uint32;		// 32-bit unsigned.
typedef unsigned long long	uint64;		// 64-bit unsigned.

typedef	signed char			int8;		// 8-bit  signed.
typedef signed short int	int16;		// 16-bit signed.
typedef signed int	 		int32;		// 32-bit signed.
typedef signed long long	int64;		// 64-bit signed.

struct FPointer
{
	uintptr_t Dummy;
};

struct FQWord
{
	int A;
	int B;
};

struct FName
{
	int32_t ComparisonIndex;
	int32_t Number;
};

template<class T>
struct TArray
{
	friend struct FString;

public:
	TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	size_t Num() const
	{
		return Count;
	};

	T& operator[](size_t i)
	{
		return Data[i];
	};

	const T& operator[](size_t i) const
	{
		return Data[i];
	};

	bool IsValidIndex(size_t i) const
	{
		return i < Num();
	}

private:
	T* Data;
	int32_t Count;
	int32_t Max;
};

template<typename KeyType, typename ValueType>
class TPair
{
public:
	KeyType   Key;
	ValueType Value;
};

struct FString : public TArray<wchar_t>
{
	std::string ToString() const
	{
		int size = WideCharToMultiByte(CP_UTF8, 0, Data, Count, nullptr, 0, nullptr, nullptr);
		std::string str(size, 0);
		WideCharToMultiByte(CP_UTF8, 0, Data, Count, &str[0], size, nullptr, nullptr);
		return str;
	}
};

class FScriptInterface
{
private:
	UObject* ObjectPointer;
	void* InterfacePointer;

public:
	UObject* GetObject() const
	{
		return ObjectPointer;
	}

	UObject*& GetObjectRef()
	{
		return ObjectPointer;
	}

	void* GetInterface() const
	{
		return ObjectPointer != nullptr ? InterfacePointer : nullptr;
	}
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
public:
	InterfaceType* operator->() const
	{
		return (InterfaceType*)GetInterface();
	}

	InterfaceType& operator*() const
	{
		return *((InterfaceType*)GetInterface());
	}

	operator bool() const
	{
		return GetInterface() != nullptr;
	}
};

class FSetElementId
{
public:
	int32_t Index;
};

template <typename InElementType>
class TSetElement
{
public:
	typedef InElementType ElementType;

	ElementType Value;

	FSetElementId HashNextId;

	int32_t HashIndex;
};

class FHeapAllocator
{
public:
	class ForAnyElementType
	{
	public:
		void* Data;
	};

	template<typename ElementType>
	class ForElementType : public ForAnyElementType
	{
	};
};

template<int32_t Size>
struct TAlignedBytes
{
	uint8_t Pad[Size];
};

template<typename ElementType>
struct TTypeCompatibleBytes : TAlignedBytes<sizeof(ElementType)>
{
};

template <uint32_t NumInlineElements, typename SecondaryAllocator = FHeapAllocator>
class TInlineAllocator
{
public:
	template<typename ElementType>
	class ForElementType
	{
	public:
		TTypeCompatibleBytes<ElementType> InlineData[NumInlineElements];

		typename SecondaryAllocator::template ForElementType<ElementType> SecondaryData;
	};

	typedef void ForAnyElementType;
};

template<typename Allocator = TInlineAllocator<4>>
class TBitArray
{
public:
	typedef typename Allocator::template ForElementType<uint32_t> AllocatorType;

	AllocatorType AllocatorInstance;
	int32_t NumBits;
	int32_t MaxBits;
};

template<typename ElementType>
union TSparseArrayElementOrFreeListLink
{
	ElementType ElementData;

	struct
	{
		int32_t PrevFreeIndex;
		int32_t NextFreeIndex;
	};
};

template<typename ElementType, typename Allocator>
class TSparseArray
{
public:
	typedef TSparseArrayElementOrFreeListLink<TAlignedBytes<sizeof(ElementType)>> FElementOrFreeListLink;

	typedef TArray<FElementOrFreeListLink> DataType;
	DataType Data;

	typedef TBitArray<typename Allocator::BitArrayAllocator> AllocationBitArrayType;
	AllocationBitArrayType AllocationFlags;

	int32_t FirstFreeIndex;

	int32_t NumFreeIndices;
};

template<typename InElementAllocator = FHeapAllocator, typename InBitArrayAllocator = TInlineAllocator<4>>
class TSparseArrayAllocator
{
public:
	typedef InElementAllocator ElementAllocator;
	typedef InBitArrayAllocator BitArrayAllocator;
};

template<typename InSparseArrayAllocator = TSparseArrayAllocator<>, typename InHashAllocator = TInlineAllocator<1, FHeapAllocator>>
class TSetAllocator
{
public:
	typedef InSparseArrayAllocator SparseArrayAllocator;
	typedef InHashAllocator HashAllocator;
};

template<typename InElementType, typename Allocator = TSetAllocator<>>
class TSet
{
public:
	typedef TSetElement<InElementType> SetElementType;
	typedef TSparseArray<SetElementType, typename Allocator::SparseArrayAllocator> ElementArrayType;
	typedef typename Allocator::HashAllocator::template ForElementType<FSetElementId> HashType;

	ElementArrayType Elements;

	HashType Hash;
	int32_t HashSize;
};

struct FTextData
{
	char pad_0000[40]; //0x0000
	wchar_t* Data; //0x0028
	int32_t Length; //0x0030
};

struct FText
{
	FTextData* Data;
	char pad_0000[16];
};

struct FWeakObjectPtr
{
	int32_t ObjectIndex;
	int32_t ObjectSerialNumber;
};

struct FStringAssetReference
{
	FString AssetLongPathname;
};

template<typename TObjectID>
class TPersistentObjectPtr
{
public:
	FWeakObjectPtr WeakPtr;
	int32_t TagAtLastTest;
	TObjectID ObjectID;
};

class FAssetPtr : public TPersistentObjectPtr<FStringAssetReference>
{

};

struct FSoftObjectPath
{
	FName AssetPathName;
	FString SubPathString;
};

class FSoftObjectPtr : public TPersistentObjectPtr<FSoftObjectPath>
{

};

struct FGuid
{
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
};

struct FUniqueObjectGuid
{
	FGuid Guid;
};

class FLazyObjectPtr : public TPersistentObjectPtr<FUniqueObjectGuid>
{

};

struct FScriptDelegate
{
	unsigned char UnknownData[20];
};

struct FScriptMulticastDelegate
{
	unsigned char UnknownData[16];
};

class UClass;

class UObject
{
public:
	FPointer VTableObject;
	int32_t ObjectFlags;
	int32_t InternalIndex;
	UClass* Class;
	FName Name;
	UObject* Outer;
};

class UField : public UObject
{
public:
	UField* Next;
};

class UEnum : public UField
{
public:
	FString CppType; //0x0030 
	TArray<TPair<FName, uint64_t>> Names; //0x0040 
	__int64 CppForm; //0x0050 
};

class UStruct : public UField
{
public:
	UStruct* SuperField;
	UField* Children;
	int32_t PropertySize;
	int32_t MinAlignment;
	char pad_0x0048[0x40];
};

class UScriptStruct : public UStruct
{
public:
	int32 Size;
	int32 Alignment;
};

class UFunction : public UStruct
{
public:

	int32_t FunctionFlags;
	int16_t RepOffset;
	int8_t NumParms;
	int16_t ParmsSize;
	int16_t ReturnValueOffset;
	int16_t RPCId;
	int16_t RPCResponseId;
	class UProperty* FirstPropertyToInit;
	UFunction* EventGraphFunction;
	int32_t EventGraphCallOffset;
	void* Func;
};

class UClass : public UStruct
{
public:
	unsigned char UnknownData00[0x1C8];
};

class UProperty : public UField
{
public:
	enum ELifetimeCondition
	{
		COND_None = 0,
		COND_InitialOnly = 1,
		COND_OwnerOnly = 2,
		COND_SkipOwner = 3,
		COND_SimulatedOnly = 4,
		COND_AutonomousOnly = 5,
		COND_SimulatedOrPhysics = 6,
		COND_InitialOrOwner = 7,
		COND_Custom = 8,
		COND_ReplayOrOwner = 9,
		COND_ReplayOnly = 10,
		COND_SimulatedOnlyNoReplay = 11,
		COND_SimulatedOrPhysicsNoReplay = 12,
		COND_SkipReplay = 13,
		COND_Max = 14
	};

	int32 ArrayDim;
	int32 ElementSize;
	FQWord PropertyFlags;
	uint16 RepIndex;
	unsigned char Pad[0x6];
	FName RepNotifyFunc;
	int32 Offset;
	ELifetimeCondition BlueprintReplicationCondition;
	class UProperty* PropertyLinkNext;
	class UProperty* NextRef;
	class UProperty* DestructorLinkNext;
	class UProperty* PostConstructLinkNext;
};

class UNumericProperty : public UProperty
{
public:
	
};

class UByteProperty : public UNumericProperty
{
public:
	UEnum*		Enum;										// 0x0088 (0x04)
};

class UUInt16Property : public UNumericProperty
{
public:

};

class UUInt32Property : public UNumericProperty
{
public:

};

class UUInt64Property : public UNumericProperty
{
public:

};

class UInt8Property : public UNumericProperty
{
public:

};

class UInt16Property : public UNumericProperty
{
public:

};

class UIntProperty : public UNumericProperty
{
public:

};

class UInt64Property : public UNumericProperty
{
public:

};

class UFloatProperty : public UNumericProperty
{
public:

};

class UDoubleProperty : public UNumericProperty
{
public:

};

class UBoolProperty : public UProperty
{
public:
	uint8_t FieldSize;
	uint8_t ByteOffset;
	uint8_t ByteMask;
	uint8_t FieldMask;
};

class UObjectPropertyBase : public UProperty
{
public:
	UClass* PropertyClass;
};

class UObjectProperty : public UObjectPropertyBase
{
public:

};

class UClassProperty : public UObjectProperty
{
public:
	UClass* MetaClass;
};

class UInterfaceProperty : public UProperty
{
public:
	UClass* InterfaceClass;
};

class UWeakObjectProperty : public UObjectPropertyBase
{
public:

};

class ULazyObjectProperty : public UObjectPropertyBase
{
public:

};

class UAssetObjectProperty : public UObjectPropertyBase
{
public:

};

class UAssetClassProperty : public UAssetObjectProperty
{
public:
	UClass* MetaClass;
};

class UNameProperty : public UProperty
{
public:

};

class UStructProperty : public UProperty
{
public:
	UScriptStruct* Struct;
};

class UStrProperty : public UProperty
{
public:

};

class UTextProperty : public UProperty
{
public:

};

class UArrayProperty : public UProperty
{
public:
	UProperty* Inner;
};

class UMapProperty : public UProperty
{
public:
	UProperty* KeyProp;
	UProperty* ValueProp;
};

class UDelegateProperty : public UProperty
{
public:
	UFunction* SignatureFunction;
};

class UMulticastDelegateProperty : public UProperty
{
public:
	UFunction* SignatureFunction;
};

class UEnumProperty : public UProperty
{
public:
	class UNumericProperty* UnderlyingProp;
	class UEnum* Enum;
};