#include "RaytracingHelpers.hpp"
#include <sstream>
#include <iomanip>


void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc)
{
    std::wstringstream wstr;
    wstr << L"\n";
    wstr << L"--------------------------------------------------------------------\n";
    wstr << L"| D3D12 State Object 0x" << static_cast<const void*>(desc) << L": ";
    if (desc->Type == D3D12_STATE_OBJECT_TYPE_COLLECTION) wstr << L"Collection\n";
    if (desc->Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE) wstr << L"Raytracing Pipeline\n";

    auto ExportTree = [](UINT depth, UINT numExports, const D3D12_EXPORT_DESC* exports)
    {
        std::wostringstream woss;
        for (UINT i = 0; i < numExports; i++)
        {
            woss << L"|";
            if (depth > 0)
            {
                for (UINT j = 0; j < 2 * depth - 1; j++) woss << L" ";
            }
            woss << L" [" << i << L"]: ";
            if (exports[i].ExportToRename) woss << exports[i].ExportToRename << L" --> ";
            woss << exports[i].Name << L"\n";
        }
        return woss.str();
    };

    for (UINT i = 0; i < desc->NumSubobjects; i++)
    {
        wstr << L"| [" << i << L"]: ";
        switch (desc->pSubobjects[i].Type)
        {
        case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
            wstr << L"Global Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
            break;
        case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
            wstr << L"Local Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
            break;
        case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
            wstr << L"Node Mask: 0x" << std::hex << std::setfill(L'0') << std::setw(8) << *static_cast<const UINT*>(desc->pSubobjects[i].pDesc) << std::setw(0) << std::dec << L"\n";
            break;
        case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY:
        {
            wstr << L"DXIL Library 0x";
            auto lib = static_cast<const D3D12_DXIL_LIBRARY_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << lib->DXILLibrary.pShaderBytecode << L", " << lib->DXILLibrary.BytecodeLength << L" bytes\n";
            wstr << ExportTree(1, lib->NumExports, lib->pExports);
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION:
        {
            wstr << L"Existing Library 0x";
            auto collection = static_cast<const D3D12_EXISTING_COLLECTION_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << collection->pExistingCollection << L"\n";
            wstr << ExportTree(1, collection->NumExports, collection->pExports);
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
        {
            wstr << L"Subobject to Exports Association (Subobject [";
            auto association = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
            UINT index = static_cast<UINT>(association->pSubobjectToAssociate - desc->pSubobjects);
            wstr << index << L"])\n";
            for (UINT j = 0; j < association->NumExports; j++)
            {
                wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
            }
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
        {
            wstr << L"DXIL Subobjects to Exports Association (";
            auto association = static_cast<const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
            wstr << association->SubobjectToAssociate << L")\n";
            for (UINT j = 0; j < association->NumExports; j++)
            {
                wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
            }
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
        {
            wstr << L"Raytracing Shader Config\n";
            auto config = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG*>(desc->pSubobjects[i].pDesc);
            wstr << L"|  [0]: Max Payload Size: " << config->MaxPayloadSizeInBytes << L" bytes\n";
            wstr << L"|  [1]: Max Attribute Size: " << config->MaxAttributeSizeInBytes << L" bytes\n";
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
        {
            wstr << L"Raytracing Pipeline Config\n";
            auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG*>(desc->pSubobjects[i].pDesc);
            wstr << L"|  [0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << L"\n";
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
        {
            wstr << L"Hit Group (";
            auto hitGroup = static_cast<const D3D12_HIT_GROUP_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << (hitGroup->HitGroupExport ? hitGroup->HitGroupExport : L"[none]") << L")\n";
            wstr << L"|  [0]: Any Hit Import: " << (hitGroup->AnyHitShaderImport ? hitGroup->AnyHitShaderImport : L"[none]") << L"\n";
            wstr << L"|  [1]: Closest Hit Import: " << (hitGroup->ClosestHitShaderImport ? hitGroup->ClosestHitShaderImport : L"[none]") << L"\n";
            wstr << L"|  [2]: Intersection Import: " << (hitGroup->IntersectionShaderImport ? hitGroup->IntersectionShaderImport : L"[none]") << L"\n";
            break;
        }
        }
        wstr << L"|--------------------------------------------------------------------\n";
    }
    wstr << L"\n";
    OutputDebugStringW(wstr.str().c_str());
}

ShaderTable::ShaderTable(ID3D12Device* device, UINT numShaderRecords, UINT shaderRecordSize, LPCWSTR resourceName) : m_name(resourceName)
{
    m_shaderRecordSize = Align(shaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
    m_shaderRecords.reserve(numShaderRecords);
    m_totalNumShaderRecords = numShaderRecords;
    UINT bufferSize = numShaderRecords * m_shaderRecordSize;
    Allocate(device, bufferSize, resourceName);
    m_mappedShaderRecords = MapCpuWriteOnly();
}

void ShaderTable::push_back(const ShaderRecord& shaderRecord)
{
    //ThrowIfFalse(m_shaderRecords.size() < m_shaderRecords.capacity(), "");
    m_shaderRecords.push_back(shaderRecord);
    shaderRecord.CopyTo(m_mappedShaderRecords);
    m_mappedShaderRecords += m_shaderRecordSize;
}
void ShaderTable::AddShaderRecordOrPushback(const ShaderRecord& shaderRecord, int instanceIndex)
{
    //ThrowIfFalse(m_shaderRecords.size() < m_shaderRecords.capacity());
    if (instanceIndex > m_shaderRecords.size())
    {
        push_back(shaderRecord);
    }
    else
    {
        m_shaderRecords[instanceIndex] = shaderRecord;
        m_shaderRecords.push_back(shaderRecord);
        shaderRecord.CopyTo(m_mappedShaderRecords);
    }
}

void ShaderTable::AddShaderRecordAtIndex(const ShaderRecord& shaderRecord, UINT instanceIndex)
{
    //ThrowIfFalse(m_shaderRecords.size() < m_shaderRecords.capacity());
    if (instanceIndex + 1 > m_shaderRecords.size())
    {
        m_shaderRecords.push_back(shaderRecord);
        shaderRecord.CopyTo(m_mappedShaderRecords);
        m_mappedShaderRecords += m_shaderRecordSize;
        return;
    }
    m_shaderRecords[instanceIndex] = shaderRecord;

    //---------Get destination of where Chunk at index resides in memory and give that as destination memory to get copied to---------------
    void* destination = m_mappedShaderRecords +  (instanceIndex * m_shaderRecordSize);
    shaderRecord.CopyToIndex(destination);

}
void ShaderTable::DebugPrint()
{
    std::unordered_map<void*, std::wstring> shaderIdToStringMap;
    std::wstringstream wstr;
    wstr << L"| --------------------------------------------------------------------\n";
    wstr << L"|Shader table - " << m_name.c_str() << L": "
        << m_shaderRecordSize << L" | "
        << m_shaderRecords.size() * m_shaderRecordSize << L" bytes\n";

    for (UINT i = 0; i < m_shaderRecords.size(); i++)
    {
        wstr << L"| [" << i << L"]: ";
        wstr << shaderIdToStringMap[m_shaderRecords[i].shaderData.ptr] << L", ";
        wstr << m_shaderRecords[i].shaderData.size << L" + " << m_shaderRecords[i].localRootArguments.size << L" bytes \n";
    }
    wstr << L"| --------------------------------------------------------------------\n";
    wstr << L"\n";

    OutputDebugStringW(wstr.str().c_str());
    
}
//----------------------------Copying shader record data to end of shader table identifier memory------------------
void ShaderRecord::CopyTo(void* dest) const
{
    uint8_t* byteDest = static_cast<uint8_t*>(dest);
    memcpy(byteDest, shaderData.ptr, shaderData.size);
    if (localRootArguments.ptr)
    {
        memcpy(byteDest + shaderData.size, localRootArguments.ptr, localRootArguments.size);
    }
}
//----------------------------------------------
void ShaderRecord::CopyToIndex(void* dest) const
{
    uint8_t* byteDest = static_cast<uint8_t*>(dest);
    memcpy(byteDest, shaderData.ptr, shaderData.size);
    if (localRootArguments.ptr)
    {
        memcpy(byteDest + shaderData.size, localRootArguments.ptr, localRootArguments.size);
    }
}
