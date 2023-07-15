#include "pch.h"
#include "assets.h"
#include "public/material.h"

// we need to take better account of textures once asset caching becomes a thing
void AutoAddMaterialAssetTextures(CPakFile* pak, std::vector<PakAsset_t>* assetEntries, rapidjson::Value& mapEntry)
{
    // cut if statement?
    if (JSON_IS_ARRAY(mapEntry, "textures"))
    {
        for (auto& it : mapEntry["textures"].GetArray())
        {
            if (!it.IsString())
                continue;

            if (it.GetStringLength() == 0)
                continue;

            // check if texture string is an asset guid (e.g., "0x5DCAT")
            if (RTech::ParseGUIDFromString(it.GetString()))
                continue;

            Assets::AddTextureAsset(pak, assetEntries, it.GetString(), JSON_GET_BOOL(mapEntry, "disableStreaming", false));
        }
    }
}

MaterialShaderType_t MaterialShaderTypeFromString(std::string& str)
{
    int type = *reinterpret_cast<const int*>(str.c_str());
    MaterialShaderType_t out = _TYPEINVAILD;

    switch (type)
    {
    case 'udgr':
        out = RGDU;
        break;
    case 'pdgr':
        out = RGDP;
        break;
    case 'cdgr':
        out = RGDC;
        break;
    case 'unks':
        out = SKNU;
        break;
    case 'pnks':
        out = SKNP;
        break;
    case 'cnks':
        out = SKNC;
        break;
    case 'udlw':
        out = WLDU;
        break;
    case 'cdlw':
        out = WLDC;
        break;
    case 'uctp':
        out = PTCU;
        break;
    case 'sctp':
        out = PTCS;
        break;
    case '\0neg':
    case '\0dlw':
    case '\0xif':
    case '\0nks':
        out = _TYPELEGACY;
        break;
    case '\0dgr': // in r2 but not used/able to be used
    default:
        out = _TYPEINVAILD;
        break;
    }

    return out;
}

// ideally replace these with material file funcs
void SetUpMaterialAssetFromJson(MaterialAsset_t* matl, rapidjson::Value& mapEntry)
{
    // what shader type his this
    if (JSON_IS_STR(mapEntry, "type"))
        matl->materialTypeStr = mapEntry["type"].GetStdString();
    
    matl->materialType = MaterialShaderTypeFromString(matl->materialTypeStr);


    // material max dimensions
    matl->width = JSON_GET_INT(mapEntry, "width", 0); // Set material width.
    matl->height = JSON_GET_INT(mapEntry, "height", 0); // Set material height.
       


    // weird flags
    if (JSON_IS_STR(mapEntry, "flags")) // Set flags properly. Responsible for texture stretching, tiling etc.
        matl->flags = strtoul(("0x" + mapEntry["flags"].GetStdString()).c_str(), NULL, 0);
    else
        matl->flags = 0x1D0300;

    // more tradition vmt like flags
    if (JSON_IS_STR(mapEntry, "flags2")) // This does a lot of very important stuff.
        matl->flags2 = strtoull(("0x" + mapEntry["flags2"].GetStdString()).c_str(), NULL, 0);
    else
        matl->flags2 = (0x56000020 | 0x10000000000000); // beeg flag is used on most things

    // dx flags
    // !!!temp!!! - these should be replaced by proper flag string parsing when possible
    int unkFlags = JSON_GET_INT(mapEntry, "unkFlags", 0x4);
    short depthStencilFlags = JSON_GET_INT(mapEntry, "depthStencilFlags", 0x17);
    short rasterizerFlags = JSON_GET_INT(mapEntry, "depthStencilFlags", 0x6); // CULL_BACK

    for (int i = 0; i < 2; ++i)
    {
        // set default as apex values, set r2 later if needed
        for (int j = 0; j < 8; ++j)
            matl->unkSections[i].unk_0[j] = 0xf0000000;

        matl->unkSections[i].unk = unkFlags;
        matl->unkSections[i].depthStencilFlags = depthStencilFlags;
        matl->unkSections[i].rasterizerFlags = rasterizerFlags;
    }

    // surfaces are defined in scripts/surfaceproperties.txt or scripts/surfaceproperties.rson
    matl->surface = JSON_GET_STR(mapEntry, "surface", "default");

    // used for blend materials and the like
    matl->surface = JSON_GET_STR(mapEntry, "surface2", "");

    // optional depth material overrides
    // probably should add types and prefixes
    std::string depthPath{};

    // defaults
    //depthPath = "material/code_private/depth_shadow_" + ((rasterizerFlags & 0x4 && !(rasterizerFlags & 0x2)) ? "frontculled_" : "") + matl->materialTypeStr + ".rpak";
    depthPath = "material/code_private/depth_shadow_" + matl->materialTypeStr + ".rpak";
    matl->depthShadowMaterial = RTech::GetAssetGUIDFromString(depthPath.c_str());

    //depthPath = "material/code_private/depth_prepass_" + ((rasterizerFlags & 0x2 && !(rasterizerFlags & 0x4)) ? "twosided_" : "") + matl->materialTypeStr + ".rpak";
    depthPath = "material/code_private/depth_prepass_" + matl->materialTypeStr + ".rpak";
    matl->depthPrepassMaterial = RTech::GetAssetGUIDFromString(depthPath.c_str());

    depthPath = "material/code_private/depth_vsm_" + matl->materialTypeStr + ".rpak";
    matl->depthVSMMaterial = RTech::GetAssetGUIDFromString(depthPath.c_str());

    depthPath = "material/code_private/depth_shadow_tight_" + matl->materialTypeStr + ".rpak";
    matl->depthShadowTightMaterial = RTech::GetAssetGUIDFromString(depthPath.c_str());

    // check for overrides
    if (JSON_IS_STR(mapEntry, "depthShadowMaterial"))
    {
        depthPath = "material/" + mapEntry["depthShadowMaterial"].GetStdString() + "_" + matl->materialTypeStr + ".rpak";
        matl->depthShadowMaterial = RTech::GetAssetGUIDFromString(depthPath.c_str());
    }

    if (JSON_IS_STR(mapEntry, "depthPrepassMaterial"))
    {
        depthPath = "material/" + mapEntry["depthPrepassMaterial"].GetStdString() + "_" + matl->materialTypeStr + ".rpak";
        matl->depthPrepassMaterial = RTech::GetAssetGUIDFromString(depthPath.c_str());
    }

    if (JSON_IS_STR(mapEntry, "depthVSMMaterial"))
    {
        depthPath = "material/" + mapEntry["depthVSMMaterial"].GetStdString() + "_" + matl->materialTypeStr + ".rpak";
        matl->depthVSMMaterial = RTech::GetAssetGUIDFromString(depthPath.c_str());
    }

    if (JSON_IS_STR(mapEntry, "depthShadowTightMaterial"))
    {
        depthPath = "material/" + mapEntry["depthShadowTightMaterial"].GetStdString() + "_" + matl->materialTypeStr + ".rpak";
        matl->depthShadowTightMaterial = RTech::GetAssetGUIDFromString(depthPath.c_str());
    }


    // get referenced colpass material if exists
    if (JSON_IS_STR(mapEntry, "colpass"))
    {
        std::string colpassPath = "material/" + mapEntry["colpass"].GetStdString()+ "_" + matl->materialTypeStr + ".rpak"; // auto add type? remove if disagree. materials never have their types in names so I don't think this should be expected?
        matl->colpassMaterial = RTech::StringToGuid(colpassPath.c_str());
    }


    // optional shaderset override
    if (JSON_IS_STR(mapEntry, "shaderset"))
        matl->shaderSet = RTech::GetAssetGUIDFromString(mapEntry["shaderset"].GetString());

    // this is more desirable but would break guid input
    /*if (JSON_IS_STR(mapEntry, "shaderset"))
    {
        std::string shadersetPath = "shaderset/" + mapEntry["shaderset"].GetStdString() + ".rpak";
        matl->shaderSet = RTech::GetAssetGUIDFromString(shadersetPath.c_str());
    }*/
    

    // texan
    if (JSON_IS_STR(mapEntry, "textureAnimation"))
        matl->textureAnimation = RTech::GetAssetGUIDFromString(mapEntry["textureAnimation"].GetString());
}

// shader parsing eventually
void SetUpStaticDxBufferFromJson(GenericShaderBuffer* shaderBuf, rapidjson::Value& mapEntry)
{
    if (mapEntry.HasMember("emissiveTint"))
    {
        for (int i = 0; i < 3; i++)
        {
            auto& EmissiveFloat = mapEntry["emissiveTint"].GetArray()[i];

            shaderBuf->c_L0_emissiveTint[i] = EmissiveFloat.GetFloat();
        }
    }

    if (mapEntry.HasMember("albedoTint"))
    {
        for (int i = 0; i < 3; i++)
        {
            auto& EmissiveFloat = mapEntry["albedoTint"].GetArray()[i];

            shaderBuf->c_L0_albedoTint[i] = EmissiveFloat.GetFloat();
        }
    }

    if (mapEntry.HasMember("opacity"))
        shaderBuf->c_opacity = mapEntry["opacity"].GetFloat();

    // spec tint is not really needed currently

    if (mapEntry.HasMember("uv1"))
    {
        for (int i = 0; i < 6; i++)
        {
            auto& UVFloat = mapEntry["uv1"].GetArray()[i];

            *shaderBuf->c_uv1.pFloat(i) = UVFloat.GetFloat();
        }
    }

    if (mapEntry.HasMember("uv2"))
    {
        for (int i = 0; i < 6; i++)
        {
            auto& UVFloat = mapEntry["uv2"].GetArray()[i];

            *shaderBuf->c_uv1.pFloat(i) = UVFloat.GetFloat();
        }
    }

    if (mapEntry.HasMember("uv3"))
    {
        for (int i = 0; i < 6; i++)
        {
            auto& UVFloat = mapEntry["uv3"].GetArray()[i];

            *shaderBuf->c_uv1.pFloat(i) = UVFloat.GetFloat();
        }
    }

    if (mapEntry.HasMember("uv4"))
    {
        for (int i = 0; i < 6; i++)
        {
            auto& UVFloat = mapEntry["uv4"].GetArray()[i];

            *shaderBuf->c_uv1.pFloat(i) = UVFloat.GetFloat();
        }
    }

    if (mapEntry.HasMember("uv5"))
    {
        for (int i = 0; i < 6; i++)
        {
            auto& UVFloat = mapEntry["uv5"].GetArray()[i];

            *shaderBuf->c_uv1.pFloat(i) = UVFloat.GetFloat();
        }
    }
}

// VERSION 7
void Assets::AddMaterialAsset_v12(CPakFile* pak, std::vector<PakAsset_t>* assetEntries, const char* assetPath, rapidjson::Value& mapEntry)
{
    Log("Adding matl asset '%s'\n", assetPath);

    if (JSON_IS_ARRAY(mapEntry, "textures"))
    {
        AutoAddMaterialAssetTextures(pak, assetEntries, mapEntry);
    }
    else
    {
        Warning("Trying to add material with no textures. Skipping asset...\n"); // shouldn't this be possible though??
        return;
    }

    // header data chunk and generic struct
    CPakDataChunk hdrChunk = pak->CreateDataChunk(sizeof(MaterialAssetHeader_v12_t), SF_HEAD, 16);
    MaterialAsset_t* matlAsset = new MaterialAsset_t{};
    std::string sAssetPath = std::string(assetPath); // hate this var name, love that it is different for every asset

    // some var declaration
    short externalDependencyCount = 0; // number of dependencies ouside this pak
    size_t textureRefSize = mapEntry["textures"].GetArray().Size() * 8; // size of the texture guid section.

    // default strings
    matlAsset->materialTypeStr = "skn";

    // parse json inputs for matl header
    SetUpMaterialAssetFromJson(matlAsset, mapEntry);

    // used only for string to guid
    std::string fullAssetPath = "material/" + sAssetPath + "_" + matlAsset->materialTypeStr + ".rpak"; // Make full rpak asset path.
    matlAsset->guid = RTech::StringToGuid(fullAssetPath.c_str()); // Convert full rpak asset path to guid and set it in the material header.
    
    // !!!R2 SPECIFIC!!!
    // always retained, not in other data chunk. not the same as apex so I will make it Quirky because I am a nerd
    {
        CPakDataChunk nameChunk = pak->CreateDataChunk(sAssetPath.size() + 1, SF_DEV | SF_CPU, 1);

        sprintf_s(nameChunk.Data(), sAssetPath.length() + 1, "%s", sAssetPath.c_str());

        matlAsset->materialName = nameChunk.GetPointer();

        pak->AddPointer(hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v12_t, materialName)));
    }

    // some janky setup you love to see
    if (matlAsset->materialType != _TYPELEGACY)
        Error("Type '%s' is not supported in Titanfall 2!!!", matlAsset->materialTypeStr.c_str());

    matlAsset->unk = matlAsset->materialTypeStr == "gen" ? 0xFBA63181 : 0x40D33E8F;

    if ((matlAsset->materialTypeStr == "fix" || matlAsset->materialTypeStr == "skn"))
    {
        for (int i = 0; i < 2; ++i)
        {
            matlAsset->unkSections[i].unk_0[0] = 0xF0138004;
            matlAsset->unkSections[i].unk_0[1] = 0xF0138004;
            matlAsset->unkSections[i].unk_0[2] = 0xF0138004;
            matlAsset->unkSections[i].unk_0[3] = 0x00138004;

            matlAsset->unkSections[i].unk = 0x4;
        }
    }
    else
    {
        for (int i = 0; i < 2; ++i)
        {
            matlAsset->unkSections[i].unk_0[0] = 0xF0138286;
            matlAsset->unkSections[i].unk_0[1] = 0xF0138286;
            matlAsset->unkSections[i].unk_0[2] = 0xF0008286;
            matlAsset->unkSections[i].unk_0[3] = 0x00138286;

            matlAsset->unkSections[i].unk = 0x5;
        }
    }

    uint32_t dataBufSize = (textureRefSize * 2) + (matlAsset->surface.length() + 1) + (mapEntry.HasMember("surface2") ? matlAsset->surface2.length() + 1 : 0);

    // asset data
    CPakDataChunk dataChunk = pak->CreateDataChunk(dataBufSize, SF_CPU /*| SF_CLIENT*/, 8);

    char* dataBuf = dataChunk.Data();

    std::vector<PakGuidRefHdr_t> guids{};

    int textureIdx = 0;
    for (auto& it : mapEntry["textures"].GetArray())
    {
        uint64_t textureGuid = RTech::GetAssetGUIDFromString(it.GetString(), true); // get texture guid

        *(uint64_t*)dataBuf = textureGuid;

        if (textureGuid != 0) // only deal with dependencies if the guid is not 0
        {
            pak->AddGuidDescriptor(&guids, dataChunk.GetPointer((textureIdx * sizeof(uint64_t)))); // register guid for this texture reference

            PakAsset_t* txtrAsset = pak->GetAssetByGuid(textureGuid, nullptr);

            if (txtrAsset)
                txtrAsset->AddRelation(assetEntries->size());
            else
            {
                externalDependencyCount++; // if the asset doesn't exist in the pak it has to be external, or missing
                Warning("unable to find texture '%s' for material '%s' within the local assets\n", it.GetString(), assetPath);
            }
        }

        dataBuf += sizeof(uint64_t);
        textureIdx++;
    }

    dataBuf += textureRefSize; // [rika]: already calculated, no need to do it again.

    // write the surface name into the buffer
    snprintf(dataBuf, matlAsset->surface.length() + 1, "%s", matlAsset->surface.c_str());

    // write surface2 name into the buffer if used
    if (matlAsset->surface2.length() > 0)
    {
        dataBuf += (matlAsset->surface.length() + 1);
        snprintf(dataBuf, matlAsset->surface2.length() + 1, "%s", matlAsset->surface2.c_str());
    }

    // ===============================
    // fill out the rest of the header
    
    size_t currentDataBufOffset = 0;
    
    matlAsset->textureHandles = dataChunk.GetPointer(currentDataBufOffset);
    pak->AddPointer(hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v12_t, textureHandles)));
    currentDataBufOffset += textureRefSize;

    matlAsset->streamingTextureHandles = dataChunk.GetPointer(currentDataBufOffset);
    pak->AddPointer(hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v12_t, streamingTextureHandles)));
    currentDataBufOffset += textureRefSize;


    matlAsset->surfaceProp = dataChunk.GetPointer(currentDataBufOffset);
    pak->AddPointer(hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v12_t, surfaceProp)));
    currentDataBufOffset += matlAsset->surface.length() + 1;

    if (matlAsset->surface2.length() > 0)
    {
        matlAsset->surfaceProp2 = dataChunk.GetPointer(currentDataBufOffset);
        pak->AddPointer(hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v12_t, surfaceProp2)));
        currentDataBufOffset += matlAsset->surface2.length() + 1;
    }

    //bool bColpass = false; // is this colpass material? how do you determine this

    // loop thru referenced assets (depth materials, colpass material) note: shaderset isn't inline with these vars in r2, so we set it after
    for (int i = 0; i < 3; ++i)
    {
        uint64_t guid = *((uint64_t*)&matlAsset->depthShadowMaterial + i);

        if (guid != 0)
        {
            pak->AddGuidDescriptor(&guids, hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v12_t, depthShadowMaterial) + (i * 8)));

            PakAsset_t* asset = pak->GetAssetByGuid(guid, nullptr, true);

            if (asset)
                asset->AddRelation(assetEntries->size());
            else
                externalDependencyCount++;
        }
    }

    // do colpass here because of how MaterialAsset_t is setup
    if (matlAsset->colpassMaterial != 0)
    {
        pak->AddGuidDescriptor(&guids, hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v12_t, colpassMaterial)));

        PakAsset_t* asset = pak->GetAssetByGuid(matlAsset->colpassMaterial, nullptr, true);

        if (asset)
            asset->AddRelation(assetEntries->size());
        else
            externalDependencyCount++;
    }

    // shaderset, see note above
    if (matlAsset->shaderSet != 0)
    {
        pak->AddGuidDescriptor(&guids, hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v12_t, shaderSet)));

        PakAsset_t* asset = pak->GetAssetByGuid(matlAsset->shaderSet, nullptr, true);

        if (asset)
            asset->AddRelation(assetEntries->size());
        else
            externalDependencyCount++;
    }

    // write header now that we are done setting it up
    matlAsset->WriteToBuffer(hdrChunk.Data(), pak->GetVersion());

    //////////////////////////////////////////
    /// cpu
    uint64_t dxStaticBufSize = 0;

    std::string cpuPath = pak->GetAssetPath() + sAssetPath + "_" + matlAsset->materialTypeStr + ".cpu";

    /* SETUP DX SHADER BUF */
    GenericShaderBuffer genericShaderBuf{};
    
    SetUpStaticDxBufferFromJson(&genericShaderBuf, mapEntry);

    MaterialShaderBufferV12 shaderBuf = genericShaderBuf.GenericV12();
    /* SETUP DX SHADER BUF */

    CPakDataChunk uberBufChunk;
    if (FILE_EXISTS(cpuPath))
    {
        dxStaticBufSize = Utils::GetFileSize(cpuPath);

        uberBufChunk = pak->CreateDataChunk(sizeof(MaterialCPUHeader) + dxStaticBufSize, SF_CPU | SF_TEMP, 8);

        std::ifstream cpuIn(cpuPath, std::ios::in | std::ios::binary);
        cpuIn.read(uberBufChunk.Data() + sizeof(MaterialCPUHeader), dxStaticBufSize);
        cpuIn.close();
    }
    else {
        dxStaticBufSize = sizeof(MaterialShaderBufferV12);
        uberBufChunk = pak->CreateDataChunk(sizeof(MaterialCPUHeader) + dxStaticBufSize, SF_CPU | SF_TEMP, 8);

        memcpy(uberBufChunk.Data() + sizeof(MaterialCPUHeader), shaderBuf.AsCharPtr(), dxStaticBufSize);
    }

    MaterialCPUHeader* cpuhdr = reinterpret_cast<MaterialCPUHeader*>(uberBufChunk.Data());
    cpuhdr->dataPtr = uberBufChunk.GetPointer(sizeof(MaterialCPUHeader));
    cpuhdr->dataSize = dxStaticBufSize;
    cpuhdr->unk_C = 3; // unsure what this value actually is but it changes

    pak->AddPointer(uberBufChunk.GetPointer(offsetof(MaterialCPUHeader, dataPtr)));

    //////////////////////////////////////////

    PakAsset_t asset;

    asset.InitAsset(matlAsset->guid, hdrChunk.GetPointer(), hdrChunk.GetSize(), uberBufChunk.GetPointer(), -1, -1, (std::uint32_t)AssetType::MATL);
    asset.version = 12;

    asset.pageEnd = pak->GetNumPages();
    asset.remainingDependencyCount = (asset.dependenciesCount - externalDependencyCount) + 1; // plus one for the asset itself (I think)

    asset.AddGuids(&guids);

    asset.EnsureUnique(assetEntries);
    assetEntries->push_back(asset);

    Log("\n");

    delete matlAsset;
}

// VERSION 8
void Assets::AddMaterialAsset_v15(CPakFile* pak, std::vector<PakAsset_t>* assetEntries, const char* assetPath, rapidjson::Value& mapEntry)
{
    Log("Adding matl asset '%s'\n", assetPath);

    if (JSON_IS_ARRAY(mapEntry, "textures"))
    {
        AutoAddMaterialAssetTextures(pak, assetEntries, mapEntry);
    }
    else
    {
        Warning("Trying to add material with no textures. Skipping asset...\n"); // shouldn't this be possible though??
        return;
    }

    // header data chunk and generic struct
    CPakDataChunk hdrChunk = pak->CreateDataChunk(sizeof(MaterialAssetHeader_v15_t), SF_HEAD, 16);
    MaterialAsset_t* matlAsset = new MaterialAsset_t{};
    std::string sAssetPath = std::string(assetPath); // hate this var name, love that it is different for every asset

    // some var declaration
    short externalDependencyCount = 0; // number of dependencies ouside this pak
    size_t textureRefSize = mapEntry["textures"].GetArray().Size() * 8; // size of the texture guid section.

    // default strings
    matlAsset->materialTypeStr = "sknp";

    // parse json inputs for matl header
    SetUpMaterialAssetFromJson(matlAsset, mapEntry);

    // used only for string to guid
    std::string fullAssetPath = "material/" + sAssetPath + "_" + matlAsset->materialTypeStr + ".rpak"; // Make full rpak asset path.
    matlAsset->guid = RTech::StringToGuid(fullAssetPath.c_str()); // Convert full rpak asset path to guid and set it in the material header.

    size_t alignedPathSize = IALIGN4(sAssetPath.length() + 1);
    uint32_t dataBufSize = alignedPathSize + (textureRefSize * 2) + (matlAsset->surface.length() + 1) + (mapEntry.HasMember("surface2") ? matlAsset->surface2.length() + 1 : 0);

    // asset data
    CPakDataChunk dataChunk = pak->CreateDataChunk(dataBufSize, SF_CPU /*| SF_CLIENT*/, 8);

    char* dataBuf = dataChunk.Data();

    std::vector<PakGuidRefHdr_t> guids{};

    int textureIdx = 0;
    for (auto& it : mapEntry["textures"].GetArray())
    {
        uint64_t textureGuid = RTech::GetAssetGUIDFromString(it.GetString(), true); // get texture guid

        *(uint64_t*)dataBuf = textureGuid;

        if (textureGuid != 0) // only deal with dependencies if the guid is not 0
        {
            pak->AddGuidDescriptor(&guids, dataChunk.GetPointer((textureIdx * sizeof(uint64_t)))); // register guid for this texture reference

            PakAsset_t* txtrAsset = pak->GetAssetByGuid(textureGuid, nullptr);

            if (txtrAsset)
                txtrAsset->AddRelation(assetEntries->size());
            else
            {
                externalDependencyCount++; // if the asset doesn't exist in the pak it has to be external, or missing
                Warning("unable to find texture '%s' for material '%s' within the local assets\n", it.GetString(), assetPath);
            }
        }

        dataBuf += sizeof(uint64_t);
        textureIdx++;
    }

    dataBuf += textureRefSize; // [rika]: already calculated, no need to do it again.

    // write the surface name into the buffer
    snprintf(dataBuf, matlAsset->surface.length() + 1, "%s", matlAsset->surface.c_str());

    // write surface2 name into the buffer if used
    if (matlAsset->surface2.length() > 0)
    {
        dataBuf += (matlAsset->surface.length() + 1);
        snprintf(dataBuf, matlAsset->surface2.length() + 1, "%s", matlAsset->surface2.c_str());
    }


    // ===============================
    // fill out the rest of the header

    size_t currentDataBufOffset = 0;

    matlAsset->materialName = dataChunk.GetPointer();
    pak->AddPointer(hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v15_t, materialName)));
    currentDataBufOffset += alignedPathSize;

    matlAsset->textureHandles = dataChunk.GetPointer(currentDataBufOffset);
    pak->AddPointer(hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v15_t, textureHandles)));
    currentDataBufOffset += textureRefSize;

    matlAsset->streamingTextureHandles = dataChunk.GetPointer(currentDataBufOffset);
    pak->AddPointer(hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v15_t, streamingTextureHandles)));
    currentDataBufOffset += textureRefSize;


    matlAsset->surfaceProp = dataChunk.GetPointer(currentDataBufOffset);
    pak->AddPointer(hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v15_t, surfaceProp)));
    currentDataBufOffset += matlAsset->surface.length() + 1;

    if (matlAsset->surface2.length() > 0)
    {
        matlAsset->surfaceProp2 = dataChunk.GetPointer(currentDataBufOffset);
        pak->AddPointer(hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v15_t, surfaceProp2)));
        currentDataBufOffset += matlAsset->surface2.length() + 1;
    }

    //bool bColpass = false; // is this colpass material? how do you determine this

    // loop thru referenced assets (depth materials, colpass material) note: shaderset isn't inline with these vars in r2, so we set it after
    for (int i = 0; i < 6; ++i)
    {
        uint64_t guid = *((uint64_t*)&matlAsset->depthShadowMaterial + i);

        if (guid != 0)
        {
            pak->AddGuidDescriptor(&guids, hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v15_t, depthShadowMaterial) + (i * 8)));

            PakAsset_t* asset = pak->GetAssetByGuid(guid, nullptr, true);

            if (asset)
                asset->AddRelation(assetEntries->size());
            else
                externalDependencyCount++;
        }
    }

    // do colpass here because of how MaterialAsset_t is setup
    if (matlAsset->textureAnimation != 0)
    {
        pak->AddGuidDescriptor(&guids, hdrChunk.GetPointer(offsetof(MaterialAssetHeader_v15_t, textureAnimation)));

        PakAsset_t* asset = pak->GetAssetByGuid(matlAsset->colpassMaterial, nullptr, true);

        if (asset)
            asset->AddRelation(assetEntries->size());
        else
            externalDependencyCount++;
    }

    matlAsset->unk = 0x1F5A92BD; // set a quirky little guy

    // write header now that we are done setting it up
    matlAsset->WriteToBuffer(hdrChunk.Data(), pak->GetVersion());

    //////////////////////////////////////////
    /// cpu
    uint64_t dxStaticBufSize = 0;

    std::string cpuPath = pak->GetAssetPath() + sAssetPath + "_" + matlAsset->materialTypeStr + ".cpu";

    /* SETUP DX SHADER BUF */
    GenericShaderBuffer genericShaderBuf{};

    SetUpStaticDxBufferFromJson(&genericShaderBuf, mapEntry);

    MaterialShaderBufferV15 shaderBuf = genericShaderBuf.GenericV15();
    /* SETUP DX SHADER BUF */

    CPakDataChunk uberBufChunk;
    if (FILE_EXISTS(cpuPath))
    {
        dxStaticBufSize = Utils::GetFileSize(cpuPath);

        uberBufChunk = pak->CreateDataChunk(sizeof(MaterialCPUHeader) + dxStaticBufSize, SF_CPU | SF_TEMP, 16);

        std::ifstream cpuIn(cpuPath, std::ios::in | std::ios::binary);
        cpuIn.read(uberBufChunk.Data() + sizeof(MaterialCPUHeader), dxStaticBufSize);
        cpuIn.close();
    }
    else {
        dxStaticBufSize = sizeof(MaterialShaderBufferV15);
        uberBufChunk = pak->CreateDataChunk(sizeof(MaterialCPUHeader) + dxStaticBufSize, SF_CPU | SF_TEMP, 16);

        memcpy(uberBufChunk.Data() + sizeof(MaterialCPUHeader), shaderBuf.AsCharPtr(), dxStaticBufSize);
    }

    MaterialCPUHeader* cpuhdr = reinterpret_cast<MaterialCPUHeader*>(uberBufChunk.Data());
    cpuhdr->dataPtr = uberBufChunk.GetPointer(sizeof(MaterialCPUHeader));
    cpuhdr->dataSize = dxStaticBufSize;

    pak->AddPointer(uberBufChunk.GetPointer(offsetof(MaterialCPUHeader, dataPtr)));

    //////////////////////////////////////////

    PakAsset_t asset;

    asset.InitAsset(matlAsset->guid, hdrChunk.GetPointer(), hdrChunk.GetSize(), uberBufChunk.GetPointer(), -1, -1, (std::uint32_t)AssetType::MATL);
    asset.version = 15;

    asset.pageEnd = pak->GetNumPages();
    asset.remainingDependencyCount = (asset.dependenciesCount - externalDependencyCount) + 1; // plus one for the asset itself (I think)

    asset.AddGuids(&guids);

    asset.EnsureUnique(assetEntries);
    assetEntries->push_back(asset);

    Log("\n");

    delete matlAsset;
}
