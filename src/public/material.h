#pragma once

enum MaterialShaderType_t : uint8_t
{
	RGDU = 0x0,
	RGDP = 0x1,
	RGDC = 0x2,
	SKNU = 0x3,
	SKNP = 0x4,
	SKNC = 0x5,
	WLDU = 0x6,
	WLDC = 0x7,
	PTCU = 0x8,
	PTCS = 0x9,

	// not real types
	_TYPECOUNT = 0xA,
	_TYPELEGACY = 0xFE, // r2 types
	_TYPEINVAILD = 0xFF,
};

#pragma pack(push, 1)

// bunch this up into a struct for ease of access and readability
struct uvTransform_t
{
	// this section is actually broken up into three parts.
	// c_uvRotScaleX
	float uvScaleX = 1.f;
	float uvRotationX = 0.f; // rotation, but w e i r d.
	// c_uvRotScaleY
	float uvRotationY = -0.f; //counter clockwise, 0-1, exceeding one causes Weird Stuff to happen.
	float uvScaleY = 1.f;
	// c_uvTranslate
	float uvTranslateX = 0.f;
	float uvTranslateY = 0.f;

	inline float* pFloat(int i) { return reinterpret_cast<float*>(this) + i; } // const ptr here mayhaps
};

// Titanfall 2 uses generally the same shader buffer so we can store the full struct (for now)
struct MaterialShaderBufferV12
{
	// the assignment of these depends on the shader set, they work similarly to texturetransforms in normal source.
	uvTransform_t c_uv1; // this is frequently used for detail textures.
	uvTransform_t c_uv2;
	uvTransform_t c_uv3;

	// u v
	float c_uvDistortionIntensity[2] = {0.f, 0.f}; // distortion on the { x, y } axis.
	float c_uvDistortion2Intensity[2] = {0.f, 0.f}; // see above, but for a second distortion texture.

	float c_fogColorFactor = 1.f;

	float c_layerBlendRamp = 0.f; // blend intensity (assumed), likely the hardness/softness of the two textures meshing.

	// r g b
	float c_albedoTint[3] = {1.f, 1.f, 1.f}; // color of the albedo texture.
	float c_opacity = 1.f; // untested.

	float c_useAlphaModulateSpecular = 0.f;
	float c_alphaEdgeFadeExponent = 0.f;
	float c_alphaEdgeFadeInner = 0.f;
	float c_alphaEdgeFadeOuter = 0.f;

	float c_useAlphaModulateEmissive = 1.f; // almost always set to 1.
	float c_emissiveEdgeFadeExponent = 0.f;
	float c_emissiveEdgeFadeInner = 0.f;
	float c_emissiveEdgeFadeOuter = 0.f;

	float c_alphaDistanceFadeScale = 10000.f;
	float c_alphaDistanceFadeBias = -0.f;
	float c_alphaTestReference = 0.f;

	float c_aspectRatioMulV = 1.778f; // this is equal to width divided by height see: 16/9 = 1.778~, not clear what it actually does.

	// r g b
	float c_emissiveTint[3] = {0.f, 0.f, 0.f}; // color of the emission, this is normally set to { 0.f, 0.f, 0.f } if you don't have an emission mask.

	float c_shadowBias = 0.f;

	float c_tsaaDepthAlphaThreshold = 0.f;
	float c_tsaaMotionAlphaThreshold = 0.9f;
	float c_tsaaMotionAlphaRamp = 10.f;
	int c_tsaaResponsiveFlag = 0x0; // this is 0 or 1 I think.

	float c_dofOpacityLuminanceScale = 1.f;

	union {
		int pad_CBufUberStatic[3] = { -1, -1, -1 }; // this is reserved space for special values, three sections by default.
		float c_glitchStrength; // only used  sometimes. on 'Glitch' shadersets, if used 'pad_CBufUberStatic' is only two sections.
	};

	float c_perfGloss = 1.f;

	// r g b
	float c_perfSpecColor[3] = {0.03f, 0.03f, 0.03f}; // specular color, consistent across most materials.

	inline char* AsCharPtr() { return reinterpret_cast<char*>(this); }
};

// from tripletake
struct MaterialShaderBufferV15
{
	// the assignment of these depends on the shader set, they work similarly to texturetransforms in normal source.
	uvTransform_t c_uv1; // this is frequently used for detail textures.
	uvTransform_t c_uv2;
	uvTransform_t c_uv3;
	uvTransform_t c_uv4;
	uvTransform_t c_uv5;

	float c_uvDistortionIntensity[2] = {1.f, 1.f};
	float c_uvDistortion2Intensity[2] = {1.f, 1.f};

	float c_L0_scatterDistanceScale = 0.166667f;

	float c_layerBlendRamp = 0.f;

	float c_opacity = 1.f;

	float c_useAlphaModulateSpecular = 0.f;
	float c_alphaEdgeFadeExponent = 0.f;
	float c_alphaEdgeFadeInner = 0.f;
	float c_alphaEdgeFadeOuter = 0.f;
	float c_useAlphaModulateEmissive = 1.f;
	float c_emissiveEdgeFadeExponent = 0.f;
	float c_emissiveEdgeFadeInner = 0.f;
	float c_emissiveEdgeFadeOuter = 0.f;
	float c_alphaDistanceFadeScale = 10000.f;
	float c_alphaDistanceFadeBias = -0.f;
	float c_alphaTestReference = 0.f;
	float c_aspectRatioMulV = 1.778000f;
	float c_shadowBias = 0.f;
	float c_shadowBiasStatic = 0.f;
	float c_dofOpacityLuminanceScale = 1.f;
	float c_tsaaDepthAlphaThreshold = 0.f;
	float c_tsaaMotionAlphaThreshold = 0.9f;
	float c_tsaaMotionAlphaRamp = 10.f;
	char UNIMPLEMENTED_c_tsaaResponsiveFlag[4];
	float c_outlineColorSDF[3] = {0.f, 0.f, 0.f};
	float c_outlineWidthSDF = 0.f;
	float c_shadowColorSDF[3] = { 0.f, 0.f, 0.f };
	float c_shadowWidthSDF = 0.f;
	float c_insideColorSDF[3] = {0.f, 0.f, 0.f};
	float c_outsideAlphaScalarSDF = 0.f;
	float c_glitchStrength = 0.f;
	float c_vertexDisplacementScale = 0.f;
	float c_innerFalloffWidthSDF = 0.f;
	float c_innerEdgeOffsetSDF = 0.f;
	float c_dropShadowOffsetSDF[2] = {0.f, 0.f};
	float c_normalMapEdgeWidthSDF = 0.f;
	float c_shadowFalloffSDF = 0.f;
	float c_L0_scatterAmount[3] = {0.f, 0.f, 0.f};
	float c_L0_scatterRatio = 0.f;
	float c_L0_transmittanceIntensityScale = 1.f;
	float c_vertexDisplacementDirection[3] = {0.f, 0.f, 0.f};
	float c_L0_transmittanceAmount = 0.f;
	float c_L0_transmittanceDistortionAmount = 0.5f;
	float c_zUpBlendingMinAngleCos = 1.f;
	float c_zUpBlendingMaxAngleCos = 1.f;
	float c_zUpBlendingVertexAlpha = 0.f;
	float c_L0_albedoTint[3] = {1.f, 1.f, 1.f};
	float c_depthBlendScalar = 1.f;
	float c_L0_emissiveTint[3] = {1.f, 1.f, 1.f};
	char UNIMPLEMENTED_c_subsurfaceMaterialID[4];
	float c_L0_perfSpecColor[3] = {0.037972f, 0.037972f, 0.037972f};
	float c_L0_perfGloss = 1.f;
	float c_L1_albedoTint[3] = {0.f, 0.f, 0.f};
	float c_L1_perfGloss = 0.f;
	float c_L1_emissiveTint[3] = {0.f, 0.f, 0.f};
	float c_L1_perfSpecColor[3] = {0.f, 0.f, 0.f};
	float c_splineMinPixelPercent = 0.f;
	float c_L0_anisoSpecCosSinTheta[2] = {1.f, 0.f};
	float c_L1_anisoSpecCosSinTheta[2] = {1.f, 0.f};
	float c_L0_anisoSpecStretchAmount = 0.f;
	float c_L1_anisoSpecStretchAmount = 0.f;
	float c_L0_emissiveHeightFalloff = 0.f;
	float c_L1_emissiveHeightFalloff = 0.f;
	float c_L1_transmittanceIntensityScale = 0.f;
	float c_L1_transmittanceAmount = 0.f;
	float c_L1_transmittanceDistortionAmount = 0.f;
	float c_L1_scatterDistanceScale = 0.f;
	float c_L1_scatterAmount[3] = {0.f, 0.f, 0.f};
	float c_L1_scatterRatio = 0.f;

	inline char* AsCharPtr() { return reinterpret_cast<char*>(this); }
};
static_assert(sizeof(MaterialShaderBufferV15) == 512);

// agony
struct GenericShaderBuffer
{
	// the assignment of these depends on the shader set, they work similarly to texturetransforms in normal source.
	uvTransform_t c_uv1; // this is frequently used for detail textures.
	uvTransform_t c_uv2;
	uvTransform_t c_uv3;
	uvTransform_t c_uv4;
	uvTransform_t c_uv5;

	float c_uvDistortionIntensity[2] = { 0.f, 0.f }; // distortion on the { x, y } axis.
	float c_uvDistortion2Intensity[2] = { 0.f, 0.f }; // see above, but for a second distortion texture.	

	float c_layerBlendRamp;

	float c_L0_albedoTint[3];
	float c_L1_albedoTint[3];

	float c_opacity;

	float c_L0_emissiveTint[3];
	float c_L1_emissiveTint[3];

	float c_L0_perfSpecColor[3];
	float c_L1_perfSpecColor[3];

	MaterialShaderBufferV12 GenericV12()
	{
		MaterialShaderBufferV12 out{};

		out.c_uv1 = this->c_uv1;
		out.c_uv2 = this->c_uv2;
		out.c_uv3 = this->c_uv3;

		out.c_uvDistortionIntensity[0] = this->c_uvDistortionIntensity[0];
		out.c_uvDistortionIntensity[1] = this->c_uvDistortionIntensity[1];
		out.c_uvDistortion2Intensity[0] = this->c_uvDistortion2Intensity[0];
		out.c_uvDistortion2Intensity[1] = this->c_uvDistortion2Intensity[1];

		out.c_layerBlendRamp = this->c_layerBlendRamp;

		for (int i = 0; i < 3; i++)
		{
			out.c_albedoTint[i] = this->c_L0_albedoTint[i];
		}

		out.c_opacity = this->c_opacity;

		for (int i = 0; i < 3; i++)
		{
			out.c_emissiveTint[i] = this->c_L0_emissiveTint[i];
		}

		return out;
	}

	MaterialShaderBufferV15 GenericV15()
	{
		MaterialShaderBufferV15 out{};

		out.c_uv1 = this->c_uv1;
		out.c_uv2 = this->c_uv2;
		out.c_uv3 = this->c_uv3;
		out.c_uv3 = this->c_uv4;
		out.c_uv3 = this->c_uv5;

		out.c_uvDistortionIntensity[0] = this->c_uvDistortionIntensity[0];
		out.c_uvDistortionIntensity[1] = this->c_uvDistortionIntensity[1];
		out.c_uvDistortion2Intensity[0] = this->c_uvDistortion2Intensity[0];
		out.c_uvDistortion2Intensity[1] = this->c_uvDistortion2Intensity[1];

		out.c_layerBlendRamp = this->c_layerBlendRamp;

		for (int i = 0; i < 3; i++)
		{
			out.c_L0_albedoTint[i] = this->c_L0_albedoTint[i];
		}

		out.c_opacity = this->c_opacity;

		for (int i = 0; i < 3; i++)
		{
			out.c_L0_emissiveTint[i] = this->c_L0_emissiveTint[i];
		}

		return out;
	}
};


struct MaterialAssetHeader_v12_t
{
	uint64_t vftableReserved; // Gets set to CMaterialGlue vtbl ptr
	char gap_8[0x8]; // unused?
	uint64_t guid; // guid of this material asset

	PagePtr_t materialName; // pointer to partial asset path
	PagePtr_t surfaceProp; // pointer to surfaceprop (as defined in surfaceproperties.rson)
	PagePtr_t surfaceProp2; // pointer to surfaceprop2 

	// IDX 1: DepthShadow
	// IDX 2: DepthPrepass
	// IDX 3: DepthVSM
	// IDX 4: ColPass
	// Titanfall is does not have 'DepthShadowTight'

	uint64_t depthShadowMaterial;
	uint64_t depthPrepassMaterial;
	uint64_t depthVSMMaterial;
	uint64_t colpassMaterial;

	// these blocks dont seem to change often but are the same?
	// these blocks relate to different render filters and flags. still not well understood.
	struct
	{
		// not sure how these work but 0xF0 -> 0x00 toggles them off and vice versa.
		// they seem to affect various rendering filters, said filters might actually be the used shaders.
		// the duplicate one is likely for the second set of textures which (probably) never gets used.
		uint32_t unkLighting;	// 0xF0138286
		uint32_t unkAliasing;	// 0xF0138286
		uint32_t unkDof;		// 0xF0008286
		uint32_t unkUnknown;	// 0x00138286

		//uint32_t unkShadow;
		//uint32_t unkPrepass;
		//uint32_t unkVSM;
		//uint32_t unkColpass;

		uint32_t unk; // 0x5
		uint16_t depthStencilFlags; // different render settings, such as opacity and transparency.
		uint16_t rasterizerFlags; // how the face is drawn, culling, wireframe, etc.

		uint64_t padding; // aligment to 16 bytes (probably)
	} unkSection[2];

	uint64_t shaderSet; // guid of the shaderset asset that this material uses

	PagePtr_t textureHandles; // TextureGUID Map
	PagePtr_t streamingTextureHandles; // Streamable TextureGUID Map
	short numStreamingTextureHandles; // Number of textures with streamed mip levels.

	uint32_t flags; // 0x503000

	short unk_AE;
	uint64_t unk_B0; // haven't observed anything here.

	// seems to be 0xFBA63181 for loadscreens
	uint32_t unk_B8; // no clue tbh, 0xFBA63181

	uint32_t unk_BC; // this might actually be "Alignment"

	uint64_t flags2;

	short width;
	short height;

	uint32_t unk_CC; // likely alignment

	/* flags
	0x050300 for loadscreens, 0x1D0300 for normal materials.
	0x1D has been observed, seems to invert lighting? used on some exceptionally weird materials.*/
};
static_assert(sizeof(MaterialAssetHeader_v12_t) == 208);

// some repeated section at the end of the material header (CMaterialGlue) struct
struct UnknownMaterialSectionV15
{
	// required but seems to follow a pattern. maybe related to "Unknown2" above?
	// nulling these bytes makes the material stop drawing entirely
	uint32_t unk_0[8];

	// last three are different I think, first few are related to depth materials
	// unk_0[5];
	// unk_14
	// unk_18
	// unk_1C

	uint32_t unk;
	uint16_t depthStencilFlags; // different render settings, such as opacity and transparency.
	uint16_t rasterizerFlags; // how the face is drawn, culling, wireframe, etc.

	uint64_t padding;
};

// start of CMaterialGlue class
struct MaterialAssetHeader_v15_t
{
	uint64_t vftableReserved; // reserved for virtual function table pointer (when copied into native CMaterialGlue)

	char gap_8[0x8]; // unused?
	uint64_t guid; // guid of this material asset

	PagePtr_t materialName; // pointer to partial asset path
	PagePtr_t surfaceProp; // pointer to surfaceprop (as defined in surfaceproperties.rson)
	PagePtr_t surfaceProp2; // pointer to surfaceprop2 

	uint64_t depthShadowMaterial;
	uint64_t depthPrepassMaterial;
	uint64_t depthVSMMaterial;
	uint64_t depthShadowTightMaterial;
	uint64_t colpassMaterial;

	uint64_t shaderSet; // guid of the shaderset asset that this material uses

	PagePtr_t textureHandles; // ptr to array of texture guids
	PagePtr_t streamingTextureHandles; // ptr to array of streamable texture guids (empty at build time)

	short numStreamingTextureHandles;// = 0x4; // Number of textures with streamed mip levels.
	short width;
	short height;
	short unk_76;

	uint32_t flags_78;// = 0x1D0300;
	uint32_t unk_7C;

	uint32_t unk_80;// = 0x1F5A92BD; // REQUIRED but why?

	uint32_t unk_84;

	uint64_t flags2;

	UnknownMaterialSectionV15 unkSections[2]; // seems to be used for setting up some D3D states?

	uint8_t bytef0;
	uint8_t bytef1;
	MaterialShaderType_t materialType;
	uint8_t bytef3; // used for unksections loading in UpdateMaterialAsset

	char pad_00F4[0x4];

	uint64_t textureAnimation;
};
static_assert(sizeof(MaterialAssetHeader_v15_t) == 256);

struct MaterialAsset_t
{
	uint64_t guid; // guid of this material asset

	PagePtr_t materialName; // pointer to partial asset path
	PagePtr_t surfaceProp; // pointer to surfaceprop (as defined in surfaceproperties.rson)
	PagePtr_t surfaceProp2; // pointer to surfaceprop2 

	uint64_t depthShadowMaterial;
	uint64_t depthPrepassMaterial;
	uint64_t depthVSMMaterial;
	uint64_t depthShadowTightMaterial;
	uint64_t colpassMaterial;

	uint64_t shaderSet = 0; // guid of the shaderset asset that this material uses

	uint64_t textureAnimation;

	PagePtr_t textureHandles; // ptr to array of texture guids
	PagePtr_t streamingTextureHandles; // ptr to array of streamable texture guids (empty at build time)

	short width;
	short height;

	uint32_t unk; // 0x1F5A92BD, REQUIRED but why?

	uint32_t flags;
	uint64_t flags2;

	UnknownMaterialSectionV15 unkSections[2]; // seems to be used for setting up some D3D states?

	std::string materialTypeStr;
	MaterialShaderType_t materialType;

	//std::string name;
	std::string surface;
	std::string surface2;

	void WriteToBuffer(char* buf, int pakVersion)
	{
		if (pakVersion <= 7)
		{
			MaterialAssetHeader_v12_t* matl = reinterpret_cast<MaterialAssetHeader_v12_t*>(buf);

			matl->guid = this->guid;

			matl->materialName = this->materialName;
			matl->surfaceProp = this->surfaceProp;
			matl->surfaceProp2 = this->surfaceProp2;

			matl->depthShadowMaterial = this->depthShadowMaterial;
			matl->depthPrepassMaterial = this->depthPrepassMaterial;
			matl->depthVSMMaterial = this->depthVSMMaterial;
			matl->colpassMaterial = this->colpassMaterial;
			matl->shaderSet = this->shaderSet;

			matl->textureHandles = this->textureHandles;
			matl->streamingTextureHandles = this->streamingTextureHandles;

			matl->width = this->width;
			matl->height = this->height;

			matl->unk_B8 = this->unk;

			matl->flags = this->flags;
			matl->flags2 = this->flags2;

			for (int i = 0; i < 2; i++)
			{
				matl->unkSection[i].unkLighting = this->unkSections[i].unk_0[0];
				matl->unkSection[i].unkAliasing = this->unkSections[i].unk_0[1];
				matl->unkSection[i].unkDof = this->unkSections[i].unk_0[2];
				matl->unkSection[i].unkUnknown = this->unkSections[i].unk_0[3];

				matl->unkSection[i].unk = this->unkSections[i].unk;
				matl->unkSection[i].depthStencilFlags = this->unkSections[i].depthStencilFlags;
				matl->unkSection[i].rasterizerFlags = this->unkSections[i].rasterizerFlags;
			}
		}
		else
		{
			MaterialAssetHeader_v15_t* matl = reinterpret_cast<MaterialAssetHeader_v15_t*>(buf);

			matl->guid = this->guid;

			matl->materialName = this->materialName;
			matl->surfaceProp = this->surfaceProp;
			matl->surfaceProp2 = this->surfaceProp2;

			matl->depthShadowMaterial = this->depthShadowMaterial;
			matl->depthPrepassMaterial = this->depthPrepassMaterial;
			matl->depthVSMMaterial = this->depthVSMMaterial;
			matl->depthShadowTightMaterial = this->depthShadowTightMaterial;
			matl->colpassMaterial = this->colpassMaterial;
			matl->shaderSet = this->shaderSet;

			matl->textureHandles = this->textureHandles;
			matl->streamingTextureHandles = this->streamingTextureHandles;

			matl->width = this->width;
			matl->height = this->height;

			matl->unk_80 = this->unk;

			matl->flags_78 = this->flags;
			matl->flags2 = this->flags2;

			matl->materialType = this->materialType;

			for (int i = 0; i < 2; i++)
			{
				for (int varIdx = 0; varIdx < 8; varIdx++)
				{
					matl->unkSections[i].unk_0[varIdx] = this->unkSections[i].unk_0[varIdx];
				}

				matl->unkSections[i].unk = this->unkSections[i].unk;
				matl->unkSections[i].depthStencilFlags = this->unkSections[i].depthStencilFlags;
				matl->unkSections[i].rasterizerFlags = this->unkSections[i].rasterizerFlags;
			}
		}
	}
};

// header struct for the material asset cpu data
struct MaterialCPUHeader
{
	PagePtr_t  dataPtr; // points to the rest of the cpu data. shader buffer.
	uint32_t dataSize;
	uint32_t unk_C; // every unknown is now either datasize, version, or flags. enum?
};
#pragma pack(pop)
