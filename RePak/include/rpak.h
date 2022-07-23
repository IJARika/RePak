#pragma once

#include <d3d11.h>

struct Vector3
{
	float x, y, z;

	Vector3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector3() {};
};

#pragma pack(push, 1)

// represents a "pointer" into a mempage by page index and offset
// when loaded, these usually get converted to a real pointer
struct RPakPtr
{
	uint32_t m_nIndex = 0;
	uint32_t m_nOffset = 0;
};

// Apex Legends RPak file header
struct RPakFileHeaderV8
{
	uint32_t m_nMagic = 0x6b615052;
	uint16_t m_nVersion = 0x8;
	uint8_t  m_nFlags[0x2];
	uint64_t m_nCreatedTime; // this is actually FILETIME, but if we don't make it uint64_t here, it'll break the struct when writing
	uint8_t  unk0[0x8];
	uint64_t m_nSizeDisk; // size of the rpak file on disk before decompression
	uint64_t m_nEmbeddedStarpakOffset = 0;
	uint8_t  unk1[0x8];
	uint64_t m_nSizeMemory; // actual data size of the rpak file after decompression
	uint64_t m_nEmbeddedStarpakSize = 0;
	uint8_t  unk3[0x8];
	uint16_t m_nStarpakReferenceSize = 0; // size in bytes of the section containing mandatory starpak paths
	uint16_t m_nStarpakOptReferenceSize = 0; // size in bytes of the section containing optional starpak paths
	uint16_t m_nVirtualSegmentCount = 0;
	uint16_t m_nPageCount = 0; // number of "mempages" in the rpak
	uint32_t m_nPatchIndex = 0;
	uint32_t m_nDescriptorCount = 0;
	uint32_t m_nAssetEntryCount = 0;
	uint32_t m_nGuidDescriptorCount = 0;
	uint32_t m_nRelationsCounts = 0;
	uint8_t  unk4[0x1c];
};

struct RPakPatchCompressedHeader // Comes after file header if its an patch rpak.
{
	uint64_t m_nSizeDisk;
	uint64_t m_nSizeMemory;
};

// Titanfall 2 RPak file header
struct RPakFileHeaderV7
{
	uint32_t m_nMagic = 0x6b615052;
	uint16_t m_nVersion = 0x7;
	uint8_t  m_nFlags[0x2];
	uint64_t m_nCreatedTime; // this is actually FILETIME, but if we don't make it uint64_t here, it'll break the struct when writing
	uint8_t  unk0[0x8];
	uint64_t m_nSizeDisk;
	uint8_t  unk1[0x8];
	uint64_t m_nSizeMemory;
	uint8_t  unk2[0x8];
	uint16_t m_nStarpakReferenceSize = 0;
	uint16_t m_nVirtualSegmentCount;
	uint16_t m_nPageCount;
	uint16_t m_nPatchIndex = 0;

	uint32_t m_nDescriptorCount = 0;
	uint32_t m_nAssetEntryCount = 0;
	uint32_t m_nUnknownFifthBlockCount = 0;
	uint32_t m_nUnknownSixedBlockCount = 0;

	uint32_t m_nUnknownSeventhBlockCount = 0;
	uint32_t m_nUnknownEighthBlockCount = 0;
};

// segment
// these probably aren't actually called virtual segments
// this struct doesn't really describe any real data segment, but collects info
// about the size of pages that are using specific flags/types/whatever
struct RPakVirtualSegment
{
	uint32_t m_nDataFlag = 0; // not sure what this actually is, doesn't seem to be used in that many places
	uint32_t m_nSomeType = 0;
	uint64_t m_nDataSize = 0;
};

// mem page
// describes an actual section in the file data. all pages are sequential
// with page at idx 0 being just after the asset relation data
// in patched rpaks (e.g. common(01).rpak), these sections don't fully line up with the data,
// because of both the patch edit stream and also missing pages that are only present in the base rpak
struct RPakPageInfo
{
	uint32_t m_nVSegIndex; // index into vseg array
	uint32_t m_nSomeType; // no idea
	uint32_t m_nDataSize; // actual size of page in bytes
};

// defines the location of a data "pointer" within the pak's mem pages
// allows the engine to read the index/offset pair and replace it with an actual memory pointer at runtime
struct RPakDescriptor
{
	uint32_t m_nPageIdx;	 // page index
	uint32_t m_nPageOffset; // offset within page
};

// same kinda thing as RPakDescriptor, but this one tells the engine where
// guid references to other assets are within mem pages
typedef RPakDescriptor RPakGuidDescriptor;

// this definitely doesn't need to be in a struct but whatever
struct RPakRelationBlock
{
	uint32_t FileID;
};

// defines a bunch of values for registering/using an asset from the rpak
struct RPakAssetEntryV8
{
	RPakAssetEntryV8() = default;

	void InitAsset(uint64_t nGUID,
		uint32_t nSubHeaderBlockIdx,
		uint32_t nSubHeaderBlockOffset,
		uint32_t nSubHeaderSize,
		uint32_t nRawDataBlockIdx,
		uint32_t nRawDataBlockOffset,
		uint64_t nStarpakOffset,
		uint64_t nOptStarpakOffset,
		uint32_t Type)
	{
		this->m_nGUID = nGUID;
		this->m_nSubHeaderDataBlockIdx = nSubHeaderBlockIdx;
		this->m_nSubHeaderDataBlockOffset = nSubHeaderBlockOffset;
		this->m_nRawDataBlockIndex = nRawDataBlockIdx;
		this->m_nRawDataBlockOffset = nRawDataBlockOffset;
		this->m_nStarpakOffset = nStarpakOffset;
		this->m_nOptStarpakOffset = nOptStarpakOffset;
		this->m_nSubHeaderSize = nSubHeaderSize;
		this->m_nMagic = Type;
	}

	// hashed version of the asset path
	// used for referencing the asset from elsewhere
	//
	// - when referenced from other assets, the GUID is used directly
	// - when referenced from scripts, the GUID is calculated from the original asset path
	//   by a function such as RTech::StringToGuid
	uint64_t m_nGUID = 0;
	uint8_t  unk0[0x8];

	// page index and offset for where this asset's header is located
	uint32_t m_nSubHeaderDataBlockIdx = 0;
	uint32_t m_nSubHeaderDataBlockOffset = 0;

	// page index and offset for where this asset's data is located
	// note: this may not always be used for finding the data:
	//		 some assets use their own idx/offset pair from within the subheader
	//		 when adding pairs like this, you MUST register it as a descriptor
	//		 otherwise the pointer won't be converted
	uint32_t m_nRawDataBlockIndex = 0;
	uint32_t m_nRawDataBlockOffset = 0;

	// offset to any available streamed data
	// m_nStarpakOffset    = "mandatory" starpak file offset
	// m_nOptStarpakOffset = "optional" starpak file offset
	// 
	// in reality both are mandatory but respawn likes to do a little trolling
	// so "opt" starpaks are a thing
	uint64_t m_nStarpakOffset = -1;
	uint64_t m_nOptStarpakOffset = -1;

	uint16_t m_nPageEnd = 0; // highest mem page used by this asset
	uint16_t unk1 = 0;

	uint32_t m_nRelationsStartIdx = 0;

	uint32_t m_nUsesStartIdx = 0;
	uint32_t m_nRelationsCounts = 0;
	uint32_t m_nUsesCount = 0; // number of other assets that this asset uses

	// size of the asset header
	uint32_t m_nSubHeaderSize = 0;

	// this isn't always changed when the asset gets changed
	// but respawn calls it a version so i will as well
	uint32_t m_nVersion = 0; 

	// see AssetType enum below
	uint32_t m_nMagic = 0;
};
#pragma pack(pop)

// internal data structure for referencing file data to be written
struct RPakRawDataBlock
{
	uint32_t m_nPageIdx;
	uint64_t m_nDataSize;
	uint8_t* m_nDataPtr;
};

// internal data structure for referencing streaming data to be written
struct SRPkDataEntry
{
	uint64_t m_nOffset = -1; // set when added
	uint64_t m_nDataSize = 0;
	uint8_t* m_nDataPtr;
};

enum class AssetType : uint32_t
{
	TEXTURE = 0x72747874, // b'txtr' - texture
	RMDL = 0x5f6c646d,    // b'mdl_' - model
	UIMG = 0x676d6975,    // b'uimg' - ui image atlas
	PTCH = 0x68637450,	  // b'Ptch' - patch
	DTBL = 0x6c627464,    // b'dtbl' - datatable
	MATL = 0x6c74616d,    // b'matl' - material
};

// identifies the data type for each column in a datatable asset
enum class dtblcoltype_t : uint32_t
{
	Bool,
	Int,
	Float,
	Vector,
	StringT,
	Asset,
	AssetNoPrecache
};

//
//	Assets
//
#pragma pack(push, 1)
struct TextureHeader
{
	uint64_t m_nAssetGUID = 0;
	RPakPtr  m_pDebugName;

	uint16_t m_nWidth = 0;
	uint16_t m_nHeight = 0;

	uint16_t unk0 = 0;
	uint16_t m_nFormat = 0;

	uint32_t m_nDataLength; // total data size across all sources
	uint8_t  unk1;
	uint8_t  m_nOptStreamedMipLevels; // why is this here and not below? respawn moment

	// d3d11 texture desc params
	uint8_t  m_nArraySize;
	uint8_t  m_nLayerCount;

	uint8_t  unk2;
	uint8_t  m_nPermanentMipLevels;
	uint8_t  m_nStreamedMipLevels;
	uint8_t  unk3[0x15];
};

struct UIImageHeader
{
	uint64_t unk0 = 0;
	uint16_t m_nWidth = 1;
	uint16_t m_nHeight = 1;
	uint16_t m_nTextureOffsetsCount = 0;
	uint16_t m_nTextureCount = 0;
	RPakPtr  m_pTextureOffsets{};
	RPakPtr  m_pTextureDims{};
	uint32_t unk1 = 0;
	uint32_t unk2 = 0;
	RPakPtr  m_pTextureHashes{};
	RPakPtr  m_pTextureNames{};
	uint64_t m_nAtlasGUID = 0;
};

struct UIImageUV
{
	// maybe the uv coords for top left?
	// just leave these as 0 and it should be fine
	float uv0x = 0;
	float uv0y = 0;

	// these two seem to be the uv coords for the bottom right corner
	// examples:
	// uv1x = 10;
	// | | | | | | | | | |
	// uv1x = 5;
	// | | | | |
	float uv1x = 1.f;
	float uv1y = 1.f;
};

// examples of changes from these values: https://imgur.com/a/l1YDXaz
struct UIImageOffset
{
	// these don't seem to matter all that much as long as they are a valid float number
	float f0 = 0.f;
	float f1 = 0.f;
	
	// endX and endY define where the edge of the image is, with 1.f being the full length of the image and 0.5f being half of the image
	float endX = 1.f;
	float endY = 1.f;

	// startX and startY define where the top left corner is in proportion to the full image dimensions
	float startX = 0.f;
	float startY = 0.f;

	// changing these 2 values causes the image to be distorted on each axis
	float unkX = 1.f;
	float unkY = 1.f;
};

struct DataTableColumn
{
	RPakPtr NamePtr;
	dtblcoltype_t Type;
	uint32_t RowOffset;
};

struct DataTableHeader
{
	uint32_t ColumnCount;
	uint32_t RowCount;

	RPakPtr ColumnHeaderPtr;
	RPakPtr RowHeaderPtr;
	uint32_t UnkHash;

	uint16_t Un1 = 0;
	uint16_t Un2 = 0;

	uint32_t RowStride;	// Number of bytes per row
	uint32_t Padding;
};

struct DataTableColumnData
{
	dtblcoltype_t Type;
	bool bValue = 0;
	int iValue = -1;
	float fValue = -1;
	Vector3 vValue;
	std::string stringValue;
	std::string assetValue;
	std::string assetNPValue;
};

struct PtchHeader
{
	uint32_t unknown_1 = 255; // always FF 00 00 00?
	uint32_t patchedPakCount = 0;

	RPakPtr pPakNames;

	RPakPtr pPakPatchNums;
};

// size: 0x78 (120 bytes)
struct ModelHeader
{
	// IDST data
	// .mdl
	RPakPtr SkeletonPtr;
	uint64_t Padding = 0;

	// model path
	// e.g. mdl/vehicle/goblin_dropship/goblin_dropship.rmdl
	RPakPtr NamePtr;
	uint64_t Padding2 = 0;

	// .phy
	RPakPtr PhyPtr;
	uint64_t Padding3 = 0;

	// this data is usually kept in a mandatory starpak, but there's also fallback(?) data here
	// in rmdl v8, this is literally just .vtx and .vvd stuck together
	// in v9+ it's an increasingly mutated version, although it contains roughly the same data
	RPakPtr VGPtr;

	// pointer to data for the model's arig guid(s?)
	RPakPtr AnimRigRefPtr;

	// this is a guess based on the above ptr's data. i think this is == to the number of guids at where the ptr points to
	uint32_t AnimRigCount = 0;

	// size of the data kept in starpak
	uint32_t StreamedDataSize = 0;
	uint32_t DataCacheSize = 0; // when 97028: FS_CheckAsyncRequest returned error for model '<NOINFO>' offset -1 count 97028 -- Error 0x00000006

	uint64_t Padding6 = 0;

	// number of anim sequences directly associated with this model
	uint32_t AnimSequenceCount = 0;
	RPakPtr AnimSequencePtr;

	uint64_t Padding7 = 0;
	uint64_t Padding8 = 0;
	uint64_t Padding9 = 0;
};

// modified source engine studio mdl header struct
// majority of these members are the same as in the source sdk
// so if you wanna know what any of it does, check there
struct studiohdr_t
{
	studiohdr_t() {};

	int id;
	int version;
	int checksum;
	int nameTableOffset;

	char name[0x40];

	int m_nDataLength;

	Vector3 eyeposition;
	Vector3 illumposition;
	Vector3 hull_min;
	Vector3 hull_max;
	Vector3 view_bbmin;
	Vector3 view_bbmax;

	int flags; // 0x9c

	int bone_count; // 0xa0
	int bone_offset; // 0xa4

	int bonecontroller_count;
	int bonecontroller_offset;

	int hitboxset_count;
	int hitboxset_offset;

	int localanim_count;
	int localanim_offset;

	int localseq_count;
	int localseq_offset;

	int activitylistversion;
	int eventsindexed;

	int texture_count;
	int texture_offset;

	int texturedir_count;
	int texturedir_offset;

	int skinref_count;
	int skinfamily_count;
	int skinref_offset;

	int bodypart_count;
	int bodypart_offset;

	int attachment_count;
	int attachment_offset;

	uint8_t unknown_1[0x14];

	int submeshlods_count;

	uint8_t unknown_2[0x64];
	int OffsetToBoneRemapInfo;
	int boneremap_count;
};

// used for referencing a material from within a model
// pathoffset is the offset to the material's path (duh)
// guid is the material's asset guid (or 0 if it's a vmt, i think)
struct materialref_t
{
	uint32_t pathoffset;
	uint64_t guid;
};

// small struct to allow verification of the 0tVG section of starpak
// model data without needing to load the entire thing into memory for a simple
// validation check
struct BasicRMDLVGHeader
{
	uint32_t magic;
	uint32_t version;
};

// some repeated section at the end of the material header (CMaterialGlue) struct
struct UnknownMaterialSection
{
	// required but seems to follow a pattern. maybe related to "Unknown2" above?
	// nulling these bytes makes the material stop drawing entirely
	
	uint32_t m_Unknown1[8]{};

	uint32_t m_Unknown2 = 0;

	// seems to be some kind of render/visibility flag.
	uint16_t m_Flags1 = 0x17;
	uint16_t m_Flags2 = 0x6;

	uint64_t m_Padding = 0;
}; // total size = 0x30

// start of CMaterialGlue class
struct MaterialHeader
{
	uint64_t m_VtblReserved = 0; // Gets set to CMaterialGlue vtbl ptr
	uint8_t m_Padding[0x8]{}; // unused

	uint64_t m_nGUID = 0; // guid of this material asset

	RPakPtr m_pszName{}; // pointer to partial asset path
	RPakPtr m_pszSurfaceProp{}; // pointer to surfaceprop (as defined in surfaceproperties.rson)
	RPakPtr m_pszSurfaceProp2{}; // pointer to surfaceprop2 

	// IDX 1: DepthShadow
	// IDX 2: DepthPrepass
	// IDX 3: DepthVSM
	// IDX 4: DepthShadowTight
	// IDX 5: ColPass
	// They seem to be the exact same for all materials throughout the game.
	uint64_t m_GUIDRefs[5]{}; // Required to have proper textures.
	uint64_t m_pShaderSet = 0; // guid of the shaderset asset that this material uses

	/* 0x60 */ RPakPtr m_pTextureHandles{}; // TextureGUID Map 1

	// should be reserved - used to store the handles for any textures that have streaming mip levels
	/* 0x68 */ RPakPtr m_pStreamingTextureHandles{};

	/* 0x70 */ int16_t m_nStreamingTextureHandleCount = 0x4;
	/* 0x72 */ int16_t m_nWidth = 2048;
	/* 0x74 */ int16_t m_nHeight = 2048;
	/* 0x76 */ int16_t m_Unknown1 = 0;

	/* 0x78 */ uint32_t m_SomeFlags = 0x1D0300;
	/* 0x7C */ uint32_t m_Unknown2 = 0;

	/* 0x80 */ uint32_t m_Unknown3 = 0x1F5A92BD; // REQUIRED but why?

	/* 0x84 */ uint32_t m_Unknown4 = 0;

	/* 0x88 */ uint32_t something = 0;
	/* 0x8C */ uint32_t something2 = 0;

	/* 0x90 */ UnknownMaterialSection m_UnknownSections[2]{};
	/* 0xF0 */ uint8_t bytef0;
	/* 0xF1 */ uint8_t bytef1;
	/* 0xF2 */ uint8_t bytef2;
	/* 0xF3 */ uint8_t bytef3; // used for unksections loading in UpdateMaterialAsset
	/* 0xF4 */ char pad_00F4[12];
};

// header struct for the material asset cpu data
struct MaterialCPUHeader
{
	RPakPtr  m_nUnknownRPtr{}; // points to the rest of the cpu data. maybe for colour?
	uint32_t m_nDataSize = 0;
	uint32_t m_nVersionMaybe = 3; // every unknown is now either datasize, version, or flags
};

#pragma pack(pop)

// internal data structure for storing patch_master entries before being written
struct PtchEntry
{
	std::string FileName = "";
	uint8_t PatchNum = 0;
	uint32_t FileNamePageOffset = 0;
};

// map of dxgi format to the corresponding txtr asset format value
static std::map<DXGI_FORMAT, uint16_t> s_txtrFormatMap{
	{ DXGI_FORMAT_BC1_UNORM, 0 },
	{ DXGI_FORMAT_BC1_UNORM_SRGB, 1 },
	{ DXGI_FORMAT_BC2_UNORM, 2 },
	{ DXGI_FORMAT_BC2_UNORM_SRGB, 3 },
	{ DXGI_FORMAT_BC3_UNORM, 4 },
	{ DXGI_FORMAT_BC3_UNORM_SRGB, 5 },
	{ DXGI_FORMAT_BC4_UNORM, 6 },
	{ DXGI_FORMAT_BC4_SNORM, 7 },
	{ DXGI_FORMAT_BC5_UNORM, 8 },
	{ DXGI_FORMAT_BC5_SNORM, 9 },
	{ DXGI_FORMAT_BC6H_UF16, 10 },
	{ DXGI_FORMAT_BC6H_SF16, 11 },
	{ DXGI_FORMAT_BC7_UNORM, 12 },
	{ DXGI_FORMAT_BC7_UNORM_SRGB, 13 },
	{ DXGI_FORMAT_R32G32B32A32_FLOAT, 14 },
	{ DXGI_FORMAT_R32G32B32A32_UINT, 15 },
	{ DXGI_FORMAT_R32G32B32A32_SINT, 16 },
	{ DXGI_FORMAT_R32G32B32_FLOAT, 17 },
	{ DXGI_FORMAT_R32G32B32_UINT, 18 },
	{ DXGI_FORMAT_R32G32B32_SINT, 19 },
	{ DXGI_FORMAT_R16G16B16A16_FLOAT, 20 },
	{ DXGI_FORMAT_R16G16B16A16_UNORM, 21 },
	{ DXGI_FORMAT_R16G16B16A16_UINT, 22 },
	{ DXGI_FORMAT_R16G16B16A16_SNORM, 23 },
	{ DXGI_FORMAT_R16G16B16A16_SINT, 24 },
	{ DXGI_FORMAT_R32G32_FLOAT, 25 },
	{ DXGI_FORMAT_R32G32_UINT, 26 },
	{ DXGI_FORMAT_R32G32_SINT, 27 },
	{ DXGI_FORMAT_R10G10B10A2_UNORM, 28 },
	{ DXGI_FORMAT_R10G10B10A2_UINT, 29 },
	{ DXGI_FORMAT_R11G11B10_FLOAT, 30 },
	{ DXGI_FORMAT_R8G8B8A8_UNORM, 31 },
	{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 32 },
	{ DXGI_FORMAT_R8G8B8A8_UINT, 33 },
	{ DXGI_FORMAT_R8G8B8A8_SNORM, 34 },
	{ DXGI_FORMAT_R8G8B8A8_SINT, 35 },
	{ DXGI_FORMAT_R16G16_FLOAT, 36 },
	{ DXGI_FORMAT_R16G16_UNORM, 37 },
	{ DXGI_FORMAT_R16G16_UINT, 38 },
	{ DXGI_FORMAT_R16G16_SNORM, 39 },
	{ DXGI_FORMAT_R16G16_SINT, 40 },
	{ DXGI_FORMAT_R32_FLOAT, 41 },
	{ DXGI_FORMAT_R32_UINT, 42 },
	{ DXGI_FORMAT_R32_SINT, 43 },
	{ DXGI_FORMAT_R8G8_UNORM, 44 },
	{ DXGI_FORMAT_R8G8_UINT, 45 },
	{ DXGI_FORMAT_R8G8_SNORM, 46 },
	{ DXGI_FORMAT_R8G8_SINT, 47 },
	{ DXGI_FORMAT_R16_FLOAT, 48 },
	{ DXGI_FORMAT_R16_UNORM, 49 },
	{ DXGI_FORMAT_R16_UINT, 50 },
	{ DXGI_FORMAT_R16_SNORM, 51 },
	{ DXGI_FORMAT_R16_SINT, 52 },
	{ DXGI_FORMAT_R8_UNORM, 53 },
	{ DXGI_FORMAT_R8_UINT, 54 },
	{ DXGI_FORMAT_R8_SNORM, 55 },
	{ DXGI_FORMAT_R8_SINT, 56 },
	{ DXGI_FORMAT_A8_UNORM, 57 },
	{ DXGI_FORMAT_R9G9B9E5_SHAREDEXP, 58 },
	{ DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM, 59 },
	{ DXGI_FORMAT_D32_FLOAT, 60 },
	{ DXGI_FORMAT_D16_UNORM, 61 },
};