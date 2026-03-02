/*
 * Copyright © 2025 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "d3d12_profiles.h"
#include <fstream>
#include <sstream>
#include "json.hpp"

#include "common/common.h"

using json = nlohmann::json;

D3D12Profile::D3D12Profile() : m_profile_loaded(false)
{
}

D3D12Profile::~D3D12Profile()
{
}

bool D3D12Profile::LoadProfile(const std::string &filename)
{
  m_profile_data = D3D12ProfileData();
  try
  {
    std::ifstream file(filename);
    if(!file.is_open())
    {
      RDCLOG("Failed to open profile file: %s", filename.c_str());
      return false;
    }

    if(!ParseJSON(file))
    {
      RDCLOG("Failed to parse profile JSON: %s", filename.c_str());
      return false;
    }

    m_profile_loaded = true;
    RDCLOG("Successfully loaded D3D12 profile: %s", filename.c_str());
    return true;
  }
  catch(const std::exception &e)
  {
    RDCLOG("Exception loading profile: %s", e.what());
    return false;
  }
}

bool D3D12Profile::ParseJSON(std::ifstream &json_file)
{
  try
  {
    auto j = json::parse(json_file);

    // Navigate to the first adapter (Adapters is an array)
    if(!j.contains("Adapters") || !j["Adapters"].is_array() || j["Adapters"].empty())
    {
      RDCLOG("Profile JSON missing Adapters array");
      return false;
    }

    auto adapter = j["Adapters"][0];

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS"];
      this->m_profile_data.options.DoublePrecisionFloatShaderOps =
          opts.value("DoublePrecisionFloatShaderOps", false);
      this->m_profile_data.options.OutputMergerLogicOp = opts.value("OutputMergerLogicOp", false);
      this->m_profile_data.options.MinPrecisionSupport =
          static_cast<D3D12_SHADER_MIN_PRECISION_SUPPORT>(opts.value("MinPrecisionSupport", 0));
      this->m_profile_data.options.TiledResourcesTier =
          static_cast<D3D12_TILED_RESOURCES_TIER>(opts.value("TiledResourcesTier", 0));
      this->m_profile_data.options.ResourceBindingTier =
          static_cast<D3D12_RESOURCE_BINDING_TIER>(opts.value("ResourceBindingTier", 0));
      this->m_profile_data.options.PSSpecifiedStencilRefSupported =
          opts.value("PSSpecifiedStencilRefSupported", false);
      this->m_profile_data.options.TypedUAVLoadAdditionalFormats =
          opts.value("TypedUAVLoadAdditionalFormats", false);
      this->m_profile_data.options.ROVsSupported = opts.value("ROVsSupported", false);
      this->m_profile_data.options.ConservativeRasterizationTier =
          static_cast<D3D12_CONSERVATIVE_RASTERIZATION_TIER>(
              opts.value("ConservativeRasterizationTier", 0));
      this->m_profile_data.options.MaxGPUVirtualAddressBitsPerResource =
          opts.value("MaxGPUVirtualAddressBitsPerResource", 0);
      this->m_profile_data.options.StandardSwizzle64KBSupported =
          opts.value("StandardSwizzle64KBSupported", false);
      this->m_profile_data.options.CrossNodeSharingTier =
          static_cast<D3D12_CROSS_NODE_SHARING_TIER>(opts.value("CrossNodeSharingTier", 0));
      this->m_profile_data.options.CrossAdapterRowMajorTextureSupported =
          opts.value("CrossAdapterRowMajorTextureSupported", false);
      this->m_profile_data.options
          .VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation = opts.value(
          "VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation", false);
      this->m_profile_data.options.ResourceHeapTier =
          static_cast<D3D12_RESOURCE_HEAP_TIER>(opts.value("ResourceHeapTier", 0));
      this->m_profile_data.has_options = true;
    }

    // Parse D3D12_FEATURE_DATA_SHADER_MODEL
    if(adapter.contains("D3D12_FEATURE_DATA_SHADER_MODEL"))
    {
      auto sm = adapter["D3D12_FEATURE_DATA_SHADER_MODEL"];
      this->m_profile_data.shader_model.HighestShaderModel =
          static_cast<D3D_SHADER_MODEL>(sm.value("HighestShaderModel", 0));
      this->m_profile_data.has_shader_model = true;
    }

    // Parse D3D12_FEATURE_DATA_ROOT_SIGNATURE
    if(adapter.contains("D3D12_FEATURE_DATA_ROOT_SIGNATURE"))
    {
      auto rs = adapter["D3D12_FEATURE_DATA_ROOT_SIGNATURE"];
      this->m_profile_data.root_signature.HighestVersion =
          static_cast<D3D_ROOT_SIGNATURE_VERSION>(rs.value("HighestVersion", 0));
      this->m_profile_data.has_root_signature = true;
    }

    // Parse D3D12_FEATURE_DATA_FEATURE_LEVELS
    if(adapter.contains("D3D12_FEATURE_DATA_FEATURE_LEVELS"))
    {
      auto fl = adapter["D3D12_FEATURE_DATA_FEATURE_LEVELS"];
      this->m_profile_data.feature_levels.MaxSupportedFeatureLevel =
          static_cast<D3D_FEATURE_LEVEL>(fl.value("MaxSupportedFeatureLevel", 0));
      this->m_profile_data.has_feature_levels = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS1
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS1"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS1"];
      this->m_profile_data.options1.WaveOps = opts.value("WaveOps", false);
      this->m_profile_data.options1.WaveLaneCountMin = opts.value("WaveLaneCountMin", 0);
      this->m_profile_data.options1.WaveLaneCountMax = opts.value("WaveLaneCountMax", 0);
      this->m_profile_data.options1.TotalLaneCount = opts.value("TotalLaneCount", 0);
      this->m_profile_data.options1.ExpandedComputeResourceStates =
          opts.value("ExpandedComputeResourceStates", false);
      this->m_profile_data.options1.Int64ShaderOps = opts.value("Int64ShaderOps", false);
      this->m_profile_data.has_options1 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS2
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS2"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS2"];
      this->m_profile_data.options2.DepthBoundsTestSupported =
          opts.value("DepthBoundsTestSupported", false);
      this->m_profile_data.options2.ProgrammableSamplePositionsTier =
          static_cast<D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER>(
              opts.value("ProgrammableSamplePositionsTier", 0));
      this->m_profile_data.has_options2 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS3
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS3"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS3"];
      this->m_profile_data.options3.CopyQueueTimestampQueriesSupported =
          opts.value("CopyQueueTimestampQueriesSupported", false);
      this->m_profile_data.options3.CastingFullyTypedFormatSupported =
          opts.value("CastingFullyTypedFormatSupported", false);
      this->m_profile_data.options3.WriteBufferImmediateSupportFlags =
          static_cast<D3D12_COMMAND_LIST_SUPPORT_FLAGS>(
              opts.value("WriteBufferImmediateSupportFlags", 0));
      this->m_profile_data.options3.ViewInstancingTier =
          static_cast<D3D12_VIEW_INSTANCING_TIER>(opts.value("ViewInstancingTier", 0));
      this->m_profile_data.options3.BarycentricsSupported =
          opts.value("BarycentricsSupported", false);
      this->m_profile_data.has_options3 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS4
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS4"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS4"];
      this->m_profile_data.options4.MSAA64KBAlignedTextureSupported =
          opts.value("MSAA64KBAlignedTextureSupported", false);
      this->m_profile_data.options4.SharedResourceCompatibilityTier =
          static_cast<D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER>(
              opts.value("SharedResourceCompatibilityTier", 0));
      this->m_profile_data.options4.Native16BitShaderOpsSupported =
          opts.value("Native16BitShaderOpsSupported", false);
      this->m_profile_data.has_options4 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS5
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS5"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS5"];
      this->m_profile_data.options5.SRVOnlyTiledResourceTier3 =
          opts.value("SRVOnlyTiledResourceTier3", false);
      this->m_profile_data.options5.RenderPassesTier =
          static_cast<D3D12_RENDER_PASS_TIER>(opts.value("RenderPassesTier", 0));
      this->m_profile_data.options5.RaytracingTier =
          static_cast<D3D12_RAYTRACING_TIER>(opts.value("RaytracingTier", 0));
      this->m_profile_data.has_options5 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS6
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS6"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS6"];
      this->m_profile_data.options6.AdditionalShadingRatesSupported =
          opts.value("AdditionalShadingRatesSupported", false);
      this->m_profile_data.options6.PerPrimitiveShadingRateSupportedWithViewportIndexing =
          opts.value("PerPrimitiveShadingRateSupportedWithViewportIndexing", false);
      this->m_profile_data.options6.VariableShadingRateTier =
          static_cast<D3D12_VARIABLE_SHADING_RATE_TIER>(opts.value("VariableShadingRateTier", 0));
      this->m_profile_data.options6.ShadingRateImageTileSize =
          opts.value("ShadingRateImageTileSize", 0);
      this->m_profile_data.options6.BackgroundProcessingSupported =
          opts.value("BackgroundProcessingSupported", false);
      this->m_profile_data.has_options6 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS7
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS7"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS7"];
      this->m_profile_data.options7.MeshShaderTier =
          static_cast<D3D12_MESH_SHADER_TIER>(opts.value("MeshShaderTier", 0));
      this->m_profile_data.options7.SamplerFeedbackTier =
          static_cast<D3D12_SAMPLER_FEEDBACK_TIER>(opts.value("SamplerFeedbackTier", 0));
      this->m_profile_data.has_options7 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS8
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS8"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS8"];
      this->m_profile_data.options8.UnalignedBlockTexturesSupported =
          opts.value("UnalignedBlockTexturesSupported", false);
      this->m_profile_data.has_options8 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS9
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS9"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS9"];
      this->m_profile_data.options9.MeshShaderPipelineStatsSupported =
          opts.value("MeshShaderPipelineStatsSupported", false);
      this->m_profile_data.options9.MeshShaderSupportsFullRangeRenderTargetArrayIndex =
          opts.value("MeshShaderSupportsFullRangeRenderTargetArrayIndex", false);
      this->m_profile_data.options9.AtomicInt64OnTypedResourceSupported =
          opts.value("AtomicInt64OnTypedResourceSupported", false);
      this->m_profile_data.options9.AtomicInt64OnGroupSharedSupported =
          opts.value("AtomicInt64OnGroupSharedSupported", false);
      this->m_profile_data.options9.DerivativesInMeshAndAmplificationShadersSupported =
          opts.value("DerivativesInMeshAndAmplificationShadersSupported", false);
      this->m_profile_data.options9.WaveMMATier =
          static_cast<D3D12_WAVE_MMA_TIER>(opts.value("WaveMMATier", 0));
      this->m_profile_data.has_options9 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS10
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS10"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS10"];
      this->m_profile_data.options10.VariableRateShadingSumCombinerSupported =
          opts.value("VariableRateShadingSumCombinerSupported", false);
      this->m_profile_data.options10.MeshShaderPerPrimitiveShadingRateSupported =
          opts.value("MeshShaderPerPrimitiveShadingRateSupported", false);
      this->m_profile_data.has_options10 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS11
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS11"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS11"];
      this->m_profile_data.options11.AtomicInt64OnDescriptorHeapResourceSupported =
          opts.value("AtomicInt64OnDescriptorHeapResourceSupported", false);
      this->m_profile_data.has_options11 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS12
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS12"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS12"];
      this->m_profile_data.options12.MSPrimitivesPipelineStatisticIncludesCulledPrimitives =
          static_cast<D3D12_TRI_STATE>(
              opts.value("MSPrimitivesPipelineStatisticIncludesCulledPrimitives", 0));
      this->m_profile_data.options12.EnhancedBarriersSupported =
          opts.value("EnhancedBarriersSupported", false);
      this->m_profile_data.options12.RelaxedFormatCastingSupported =
          opts.value("RelaxedFormatCastingSupported", false);
      this->m_profile_data.has_options12 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS13
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS13"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS13"];
      this->m_profile_data.options13.UnrestrictedBufferTextureCopyPitchSupported =
          opts.value("UnrestrictedBufferTextureCopyPitchSupported", false);
      this->m_profile_data.options13.UnrestrictedVertexElementAlignmentSupported =
          opts.value("UnrestrictedVertexElementAlignmentSupported", false);
      this->m_profile_data.options13.InvertedViewportHeightFlipsYSupported =
          opts.value("InvertedViewportHeightFlipsYSupported", false);
      this->m_profile_data.options13.InvertedViewportDepthFlipsZSupported =
          opts.value("InvertedViewportDepthFlipsZSupported", false);
      this->m_profile_data.options13.TextureCopyBetweenDimensionsSupported =
          opts.value("TextureCopyBetweenDimensionsSupported", false);
      this->m_profile_data.options13.AlphaBlendFactorSupported =
          opts.value("AlphaBlendFactorSupported", false);
      this->m_profile_data.has_options13 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS14
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS14"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS14"];
      this->m_profile_data.options14.AdvancedTextureOpsSupported =
          opts.value("AdvancedTextureOpsSupported", false);
      this->m_profile_data.options14.WriteableMSAATexturesSupported =
          opts.value("WriteableMSAATexturesSupported", false);
      this->m_profile_data.options14.IndependentFrontAndBackStencilRefMaskSupported =
          opts.value("IndependentFrontAndBackStencilRefMaskSupported", false);
      this->m_profile_data.has_options14 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS15
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS15"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS15"];
      this->m_profile_data.options15.TriangleFanSupported = opts.value("TriangleFanSupported", false);
      this->m_profile_data.options15.DynamicIndexBufferStripCutSupported =
          opts.value("DynamicIndexBufferStripCutSupported", false);
      this->m_profile_data.has_options15 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS16
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS16"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS16"];
      this->m_profile_data.options16.DynamicDepthBiasSupported =
          opts.value("DynamicDepthBiasSupported", false);
      this->m_profile_data.options16.GPUUploadHeapSupported =
          opts.value("GPUUploadHeapSupported", false);
      this->m_profile_data.has_options16 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS17
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS17"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS17"];
      this->m_profile_data.options17.NonNormalizedCoordinateSamplersSupported =
          opts.value("NonNormalizedCoordinateSamplersSupported", false);
      this->m_profile_data.options17.ManualWriteTrackingResourceSupported =
          opts.value("ManualWriteTrackingResourceSupported", false);
      this->m_profile_data.has_options17 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS18
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS18"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS18"];
      this->m_profile_data.options18.RenderPassesValid = opts.value("RenderPassesValid", false);
      this->m_profile_data.has_options18 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS19
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS19"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS19"];
      this->m_profile_data.options19.MismatchingOutputDimensionsSupported =
          opts.value("MismatchingOutputDimensionsSupported", false);
      this->m_profile_data.options19.SupportedSampleCountsWithNoOutputs =
          opts.value("SupportedSampleCountsWithNoOutputs", 0);
      this->m_profile_data.options19.PointSamplingAddressesNeverRoundUp =
          opts.value("PointSamplingAddressesNeverRoundUp", false);
      this->m_profile_data.options19.RasterizerDesc2Supported =
          opts.value("RasterizerDesc2Supported", false);
      this->m_profile_data.options19.NarrowQuadrilateralLinesSupported =
          opts.value("NarrowQuadrilateralLinesSupported", false);
      this->m_profile_data.options19.AnisoFilterWithPointMipSupported =
          opts.value("AnisoFilterWithPointMipSupported", false);
      this->m_profile_data.options19.MaxSamplerDescriptorHeapSize =
          opts.value("MaxSamplerDescriptorHeapSize", 0);
      this->m_profile_data.options19.MaxSamplerDescriptorHeapSizeWithStaticSamplers =
          opts.value("MaxSamplerDescriptorHeapSizeWithStaticSamplers", 0);
      this->m_profile_data.options19.MaxViewDescriptorHeapSize =
          opts.value("MaxViewDescriptorHeapSize", 0);
      this->m_profile_data.options19.ComputeOnlyCustomHeapSupported =
          opts.value("ComputeOnlyCustomHeapSupported", false);
      this->m_profile_data.has_options19 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS20
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS20"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS20"];
      this->m_profile_data.options20.ComputeOnlyWriteWatchSupported =
          opts.value("ComputeOnlyWriteWatchSupported", false);
      this->m_profile_data.options20.RecreateAtTier =
          static_cast<D3D12_RECREATE_AT_TIER>(opts.value("RecreateAtTier", 0));
      this->m_profile_data.has_options20 = true;
    }

    // Parse D3D12_FEATURE_DATA_D3D12_OPTIONS21
    if(adapter.contains("D3D12_FEATURE_DATA_D3D12_OPTIONS21"))
    {
      auto opts = adapter["D3D12_FEATURE_DATA_D3D12_OPTIONS21"];
      this->m_profile_data.options21.WorkGraphsTier =
          static_cast<D3D12_WORK_GRAPHS_TIER>(opts.value("WorkGraphsTier", 0));
      this->m_profile_data.options21.ExecuteIndirectTier =
          static_cast<D3D12_EXECUTE_INDIRECT_TIER>(opts.value("ExecuteIndirectTier", 0));
      this->m_profile_data.options21.SampleCmpGradientAndBiasSupported =
          opts.value("SampleCmpGradientAndBiasSupported", false);
      this->m_profile_data.options21.ExtendedCommandInfoSupported =
          opts.value("ExtendedCommandInfoSupported", false);
      this->m_profile_data.has_options21 = true;
    }

    // Parse GetDescriptorHandleIncrementSize
    if(adapter.contains("GetDescriptorHandleIncrementSize"))
    {
      auto desc_sizes = adapter["GetDescriptorHandleIncrementSize"];
      if(desc_sizes.contains("D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV"))
      {
        this->m_profile_data.descriptor_increment_sizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] =
            desc_sizes["D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV"];
      }
      if(desc_sizes.contains("D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER"))
      {
        this->m_profile_data.descriptor_increment_sizes[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] =
            desc_sizes["D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER"];
      }
      if(desc_sizes.contains("D3D12_DESCRIPTOR_HEAP_TYPE_RTV"))
      {
        this->m_profile_data.descriptor_increment_sizes[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] =
            desc_sizes["D3D12_DESCRIPTOR_HEAP_TYPE_RTV"];
      }
      if(desc_sizes.contains("D3D12_DESCRIPTOR_HEAP_TYPE_DSV"))
      {
        this->m_profile_data.descriptor_increment_sizes[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] =
            desc_sizes["D3D12_DESCRIPTOR_HEAP_TYPE_DSV"];
      }
    }

    // Parse Formats
    if(adapter.contains("Formats"))
    {
      auto formats_obj = adapter["Formats"];
      for(auto &format_entry : formats_obj.items())
      {
        DXGI_FORMAT format = static_cast<DXGI_FORMAT>(std::stoul(format_entry.key()));
        auto &format_data = this->m_profile_data.formats[format];

        if(format_entry.value().contains("Support1"))
        {
          format_data.Support1 = format_entry.value()["Support1"];
          format_data.has_support = true;
        }
        if(format_entry.value().contains("Support2"))
        {
          format_data.Support2 = format_entry.value()["Support2"];
          format_data.has_support = true;
        }
        if(format_entry.value().contains("PlaneCount"))
        {
          format_data.PlaneCount = format_entry.value()["PlaneCount"];
          format_data.has_plane_count = true;
        }
        if(format_entry.value().contains("MultisampleQualityLevels"))
        {
          auto ms_levels = format_entry.value()["MultisampleQualityLevels"];
          for(auto &ms_entry : ms_levels.items())
          {
            UINT sample_count = static_cast<UINT>(std::stoul(ms_entry.key()));
            D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_data = {};
            ms_data.Format = format;
            ms_data.SampleCount = sample_count;
            ms_data.NumQualityLevels = ms_entry.value().value("NumQualityLevels", 0);
            ms_data.Flags = static_cast<D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS>(
                ms_entry.value().value("Flags", 0));
            format_data.multisample_quality_levels[sample_count] = ms_data;
          }
        }
      }
    }

    // Parse RdocCompatibility
    if(adapter.contains("RdocCompatibility"))
    {
      auto rdoc_compat = adapter["RdocCompatibility"];
      if(rdoc_compat.contains("MinAllocAlignment"))
      {
        this->m_profile_data.min_alloc_alignment = rdoc_compat.value("MinAllocAlignment", 4096ULL);
      }
      if(rdoc_compat.contains("ImageAllocSizeMultiplier"))
      {
        this->m_profile_data.image_alloc_size_multiplier =
            rdoc_compat.value("ImageAllocSizeMultiplier", 1.33f);
      }
      if(rdoc_compat.contains("MinImageAllocSizeIncrease"))
      {
        this->m_profile_data.min_image_alloc_size_increase =
            rdoc_compat.value("MinImageAllocSizeIncrease", 8192ULL);
      }
    }

    return true;
  }
  catch(const json::exception &e)
  {
    RDCLOG("JSON parsing error: %s", e.what());
    return false;
  }
}

#define CHECK_FEATURE_SUPPORT(FEATURE_ENUM, HAS_FLAG, DATA_MEMBER, DATA_TYPE)            \
  case FEATURE_ENUM:                                                                     \
    if(this->m_profile_data.HAS_FLAG && FeatureSupportDataSize == sizeof(DATA_TYPE))     \
    {                                                                                    \
      memcpy(pFeatureSupportData, &this->m_profile_data.DATA_MEMBER, sizeof(DATA_TYPE)); \
      return S_OK;                                                                       \
    }                                                                                    \
    break;

HRESULT D3D12Profile::CheckFeatureSupport(ID3D12Device *pDevice, D3D12_FEATURE Feature,
                                          void *pFeatureSupportData, UINT FeatureSupportDataSize) const
{
  switch(Feature)
  {
    CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS, has_options, options,
                          D3D12_FEATURE_DATA_D3D12_OPTIONS)
    CHECK_FEATURE_SUPPORT(D3D12_FEATURE_SHADER_MODEL, has_shader_model, shader_model,
                          D3D12_FEATURE_DATA_SHADER_MODEL)
    CHECK_FEATURE_SUPPORT(D3D12_FEATURE_ROOT_SIGNATURE, has_root_signature, root_signature,
                          D3D12_FEATURE_DATA_ROOT_SIGNATURE)

    case D3D12_FEATURE_FEATURE_LEVELS:
      if(this->m_profile_data.has_feature_levels &&
         FeatureSupportDataSize == sizeof(D3D12_FEATURE_DATA_FEATURE_LEVELS))
      {
        // Copy only MaxSupportedFeatureLevel, as the input parameters are provided by the application
        auto *data = static_cast<D3D12_FEATURE_DATA_FEATURE_LEVELS *>(pFeatureSupportData);
        data->MaxSupportedFeatureLevel = this->m_profile_data.feature_levels.MaxSupportedFeatureLevel;
        return S_OK;
      }
      break;

      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS1, has_options1, options1,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS1)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS2, has_options2, options2,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS2)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS3, has_options3, options3,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS3)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS4, has_options4, options4,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS4)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS5, has_options5, options5,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS5)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS6, has_options6, options6,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS6)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS7, has_options7, options7,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS7)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS8, has_options8, options8,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS8)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS9, has_options9, options9,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS9)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS10, has_options10, options10,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS10)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS11, has_options11, options11,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS11)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS12, has_options12, options12,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS12)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS13, has_options13, options13,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS13)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS14, has_options14, options14,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS14)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS15, has_options15, options15,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS15)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS16, has_options16, options16,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS16)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS17, has_options17, options17,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS17)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS18, has_options18, options18,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS18)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS19, has_options19, options19,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS19)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS20, has_options20, options20,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS20)
      CHECK_FEATURE_SUPPORT(D3D12_FEATURE_D3D12_OPTIONS21, has_options21, options21,
                            D3D12_FEATURE_DATA_D3D12_OPTIONS21)

    case D3D12_FEATURE_FORMAT_SUPPORT:
      if(this->m_profile_data.formats.empty())
        break;

      if(FeatureSupportDataSize == sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT))
      {
        auto *data = static_cast<D3D12_FEATURE_DATA_FORMAT_SUPPORT *>(pFeatureSupportData);
        auto it = this->m_profile_data.formats.find(data->Format);
        if(it != this->m_profile_data.formats.end() && it->second.has_support)
        {
          data->Support1 = static_cast<D3D12_FORMAT_SUPPORT1>(it->second.Support1);
          data->Support2 = static_cast<D3D12_FORMAT_SUPPORT2>(it->second.Support2);
          return S_OK;
        }

        data->Support1 = static_cast<D3D12_FORMAT_SUPPORT1>(0);
        data->Support2 = static_cast<D3D12_FORMAT_SUPPORT2>(0);
        return S_OK;
      }
      break;

    case D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS:
      if(this->m_profile_data.formats.empty())
        break;

      if(FeatureSupportDataSize == sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))
      {
        auto *data =
            static_cast<D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS *>(pFeatureSupportData);
        auto it = this->m_profile_data.formats.find(data->Format);
        if(it != this->m_profile_data.formats.end())
        {
          auto ms_it = it->second.multisample_quality_levels.find(data->SampleCount);
          if(ms_it != it->second.multisample_quality_levels.end())
          {
            data->NumQualityLevels = ms_it->second.NumQualityLevels;
            data->Flags = ms_it->second.Flags;
            return S_OK;
          }
        }

        data->NumQualityLevels = 0;
        data->Flags = static_cast<D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS>(0);
        return S_OK;
      }
      break;

    case D3D12_FEATURE_FORMAT_INFO:
      if(this->m_profile_data.formats.empty())
        break;

      if(FeatureSupportDataSize == sizeof(D3D12_FEATURE_DATA_FORMAT_INFO))
      {
        auto *data = static_cast<D3D12_FEATURE_DATA_FORMAT_INFO *>(pFeatureSupportData);
        auto it = this->m_profile_data.formats.find(data->Format);
        if(it != this->m_profile_data.formats.end() && it->second.has_plane_count)
        {
          data->PlaneCount = it->second.PlaneCount;
          return S_OK;
        }

        data->PlaneCount = 0;
        return S_OK;
      }
      break;

    default:
      // For features not in profile, pass through to real device
      break;
  }

  return pDevice->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
}

#undef CHECK_FEATURE_SUPPORT

UINT D3D12Profile::GetDescriptorHandleIncrementSize(ID3D12Device *pDevice,
                                                    D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType) const
{
  // We shouldn't override GetDescriptorHandleIncrementSize,
  // RenderDoc already can deal with incompatibility.

  //   auto it = this->m_profile_data.descriptor_increment_sizes.find(DescriptorHeapType);
  //   if(it != this->m_profile_data.descriptor_increment_sizes.end())
  //   {
  //     RDCLOG("Intercepted GetDescriptorHandleIncrementSize for heap type %d",
  //     DescriptorHeapType); return it->second;
  //   }
  return pDevice->GetDescriptorHandleIncrementSize(DescriptorHeapType);
}

template <typename DescT>
using GetResourceAllocationInfoFunc =
    std::function<D3D12_RESOURCE_ALLOCATION_INFO(uint32_t resourceIdx, const DescT *pCustomDesc)>;

template <typename DescT>
void ApplyAllocTuning(D3D12_RESOURCE_ALLOCATION_INFO &info, UINT numResourceDescs,
                      const DescT *pResourceDescs, UINT64 min_alloc_alignment,
                      float image_alloc_size_multiplier, UINT64 min_image_alloc_size_increase,
                      GetResourceAllocationInfoFunc<DescT> getResourceAllocationInfo)
{
  UINT64 offset = 0;
  UINT64 max_alignment = 0;

  for(UINT i = 0; i < numResourceDescs; i++)
  {
    const auto &desc = pResourceDescs[i];

    D3D12_RESOURCE_ALLOCATION_INFO resInfo = getResourceAllocationInfo(i, nullptr);

    if(resInfo.SizeInBytes == -1)
    {
      return;
    }

    if(desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
    {
      resInfo.Alignment = std::max(min_alloc_alignment, resInfo.Alignment);

      UINT64 baseSize = resInfo.SizeInBytes;
      UINT64 multiplier = 1;

      // For 3D textures, apply tuning per-slice instead of to the whole texture
      if(desc.DepthOrArraySize > 1)
      {
        // Create a single-slice descriptor to query the actual per-slice allocation size
        DescT singleSliceDesc = desc;
        singleSliceDesc.DepthOrArraySize = 1;

        D3D12_RESOURCE_ALLOCATION_INFO sliceInfo = getResourceAllocationInfo(i, &singleSliceDesc);

        if(sliceInfo.SizeInBytes == -1)
        {
          return;
        }

        baseSize = sliceInfo.SizeInBytes;
        multiplier = desc.DepthOrArraySize;
      }

      // if(desc.MipLevels > 1)
      // {
      //   baseSize *= 2;
      // }

      // if (desc.Width <= 64u || desc.Height <= 64u)
      // {
      //   baseSize *= std::max(desc.MipLevels, (UINT16)4u);
      // }

      // Apply tuning to base size (either single slice or whole texture)
      UINT64 tunedSize = std::max((UINT64)(baseSize * image_alloc_size_multiplier),
                                  baseSize + min_image_alloc_size_increase);
      tunedSize = AlignUp(tunedSize, resInfo.Alignment);

      resInfo.SizeInBytes = tunedSize * multiplier;
    }

    max_alignment = std::max(max_alignment, resInfo.Alignment);

    offset = AlignUp(offset, resInfo.Alignment);
    offset += resInfo.SizeInBytes;
  }

  info.Alignment = max_alignment;
  info.SizeInBytes = AlignUp(offset, info.Alignment);
}

template <typename DescT>
void ApplyAllocTuning1(D3D12_RESOURCE_ALLOCATION_INFO &info, UINT numResourceDescs,
                       const DescT *pResourceDescs,
                       D3D12_RESOURCE_ALLOCATION_INFO1 *pResourceAllocationInfo1,
                       UINT64 min_alloc_alignment, float image_alloc_size_multiplier,
                       UINT64 min_image_alloc_size_increase,
                       GetResourceAllocationInfoFunc<DescT> getResourceAllocationInfo)
{
  if(!pResourceAllocationInfo1)
  {
    ApplyAllocTuning<DescT>(info, numResourceDescs, pResourceDescs, min_alloc_alignment,
                            image_alloc_size_multiplier, min_image_alloc_size_increase,
                            getResourceAllocationInfo);
    return;
  }

  UINT64 offset = 0;
  UINT64 max_alignment = 0;

  for(UINT i = 0; i < numResourceDescs; i++)
  {
    if(pResourceAllocationInfo1[i].SizeInBytes == -1)
    {
      return;
    }

    if(pResourceDescs[i].Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
    {
      pResourceAllocationInfo1[i].Alignment =
          std::max(min_alloc_alignment, pResourceAllocationInfo1[i].Alignment);

      UINT64 baseSize = pResourceAllocationInfo1[i].SizeInBytes;
      UINT64 multiplier = 1;

      // For 3D textures, apply tuning per-slice instead of to the whole texture
      if(pResourceDescs[i].DepthOrArraySize > 1)
      {
        // Create a single-slice descriptor to query the actual per-slice allocation size
        DescT singleSliceDesc = pResourceDescs[i];
        singleSliceDesc.DepthOrArraySize = 1;

        D3D12_RESOURCE_ALLOCATION_INFO sliceInfo = getResourceAllocationInfo(i, &singleSliceDesc);

        if(sliceInfo.SizeInBytes == -1)
        {
          return;
        }

        baseSize = sliceInfo.SizeInBytes;
        multiplier = pResourceDescs[i].DepthOrArraySize;
      }

      // if(pResourceDescs[i].MipLevels > 1)
      // {
      //   baseSize *= 2;
      // }

      // if(pResourceDescs[i].Width <= 64u || pResourceDescs[i].Height <= 64u)
      // {
      //   baseSize *= std::max(pResourceDescs[i].MipLevels, (UINT16)4u);
      // }

      // Apply tuning to base size (either single slice or whole texture)
      UINT64 tunedSize = std::max((UINT64)(baseSize * image_alloc_size_multiplier),
                                  baseSize + min_image_alloc_size_increase);
      tunedSize = AlignUp(tunedSize, pResourceAllocationInfo1[i].Alignment);

      // Multiply by slice count for 3D textures
      pResourceAllocationInfo1[i].SizeInBytes = tunedSize * multiplier;
    }

    max_alignment = std::max(max_alignment, pResourceAllocationInfo1[i].Alignment);

    offset = AlignUp(offset, pResourceAllocationInfo1[i].Alignment);
    pResourceAllocationInfo1[i].Offset = offset;

    offset += pResourceAllocationInfo1[i].SizeInBytes;
  }

  info.Alignment = max_alignment;
  info.SizeInBytes = AlignUp(offset, info.Alignment);
}

D3D12_RESOURCE_ALLOCATION_INFO D3D12Profile::GetResourceAllocationInfo(
    ID3D12Device *pDevice, UINT visibleMask, UINT numResourceDescs,
    const D3D12_RESOURCE_DESC *pResourceDescs) const
{
  D3D12_RESOURCE_ALLOCATION_INFO info =
      pDevice->GetResourceAllocationInfo(visibleMask, numResourceDescs, pResourceDescs);

  if(this->m_profile_loaded)
  {
    ApplyAllocTuning<D3D12_RESOURCE_DESC>(
        info, numResourceDescs, pResourceDescs, this->m_profile_data.min_alloc_alignment,
        this->m_profile_data.image_alloc_size_multiplier,
        this->m_profile_data.min_image_alloc_size_increase,
        [&](uint32_t resourceIdx, const D3D12_RESOURCE_DESC *pCustomDesc) {
          const D3D12_RESOURCE_DESC *pDesc = pCustomDesc ? pCustomDesc : &pResourceDescs[resourceIdx];
          return pDevice->GetResourceAllocationInfo(visibleMask, 1, pDesc);
        });
  }
  return info;
}

D3D12_RESOURCE_ALLOCATION_INFO D3D12Profile::GetResourceAllocationInfo1(
    ID3D12Device4 *pDevice, UINT visibleMask, UINT numResourceDescs,
    const D3D12_RESOURCE_DESC *pResourceDescs,
    D3D12_RESOURCE_ALLOCATION_INFO1 *pResourceAllocationInfo1) const
{
  D3D12_RESOURCE_ALLOCATION_INFO info = pDevice->GetResourceAllocationInfo1(
      visibleMask, numResourceDescs, pResourceDescs, pResourceAllocationInfo1);

  if(this->m_profile_loaded)
  {
    ApplyAllocTuning1<D3D12_RESOURCE_DESC>(
        info, numResourceDescs, pResourceDescs, pResourceAllocationInfo1,
        this->m_profile_data.min_alloc_alignment, this->m_profile_data.image_alloc_size_multiplier,
        this->m_profile_data.min_image_alloc_size_increase,
        [&](uint32_t resourceIdx, const D3D12_RESOURCE_DESC *pCustomDesc) {
          const D3D12_RESOURCE_DESC *pDesc = pCustomDesc ? pCustomDesc : &pResourceDescs[resourceIdx];
          return pDevice->GetResourceAllocationInfo1(visibleMask, 1, pDesc, NULL);
        });
  }
  return info;
}

D3D12_RESOURCE_ALLOCATION_INFO D3D12Profile::GetResourceAllocationInfo2(
    ID3D12Device8 *pDevice, UINT visibleMask, UINT numResourceDescs,
    const D3D12_RESOURCE_DESC1 *pResourceDescs,
    D3D12_RESOURCE_ALLOCATION_INFO1 *pResourceAllocationInfo1) const
{
  D3D12_RESOURCE_ALLOCATION_INFO info = pDevice->GetResourceAllocationInfo2(
      visibleMask, numResourceDescs, pResourceDescs, pResourceAllocationInfo1);
  if(this->m_profile_loaded)
  {
    ApplyAllocTuning1<D3D12_RESOURCE_DESC1>(
        info, numResourceDescs, pResourceDescs, pResourceAllocationInfo1,
        this->m_profile_data.min_alloc_alignment, this->m_profile_data.image_alloc_size_multiplier,
        this->m_profile_data.min_image_alloc_size_increase,
        [&](uint32_t resourceIdx, const D3D12_RESOURCE_DESC1 *pCustomDesc) {
          const D3D12_RESOURCE_DESC1 *pDesc =
              pCustomDesc ? pCustomDesc : &pResourceDescs[resourceIdx];
          return pDevice->GetResourceAllocationInfo2(visibleMask, 1, pDesc, NULL);
        });
  }
  return info;
}

D3D12_RESOURCE_ALLOCATION_INFO D3D12Profile::GetResourceAllocationInfo3(
    ID3D12Device12 *pDevice, UINT visibleMask, UINT numResourceDescs,
    const D3D12_RESOURCE_DESC1 *pResourceDescs, const UINT32 *pNumCastableFormats,
    const DXGI_FORMAT *const *ppCastableFormats,
    D3D12_RESOURCE_ALLOCATION_INFO1 *pResourceAllocationInfo1) const
{
  D3D12_RESOURCE_ALLOCATION_INFO info = pDevice->GetResourceAllocationInfo3(
      visibleMask, numResourceDescs, pResourceDescs, pNumCastableFormats, ppCastableFormats,
      pResourceAllocationInfo1);
  if(this->m_profile_loaded)
  {
    ApplyAllocTuning1<D3D12_RESOURCE_DESC1>(
        info, numResourceDescs, pResourceDescs, pResourceAllocationInfo1,
        this->m_profile_data.min_alloc_alignment, this->m_profile_data.image_alloc_size_multiplier,
        this->m_profile_data.min_image_alloc_size_increase,
        [&](uint32_t resourceIdx, const D3D12_RESOURCE_DESC1 *pCustomDesc) {
          const D3D12_RESOURCE_DESC1 *pDesc =
              pCustomDesc ? pCustomDesc : &pResourceDescs[resourceIdx];
          return pDevice->GetResourceAllocationInfo3(visibleMask, 1, pDesc,
                                                     &pNumCastableFormats[resourceIdx],
                                                     &ppCastableFormats[resourceIdx], NULL);
        });
  }
  return info;
}