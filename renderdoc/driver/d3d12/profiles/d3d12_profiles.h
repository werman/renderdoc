/*
 * Copyright © 2025 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#pragma once

#define NOMINMAX

#include <memory>
#include <string>
#include <unordered_map>
#include "driver/dx/official/d3d12.h"

struct D3D12ProfileData
{
  // D3D12_FEATURE_DATA_D3D12_OPTIONS
  D3D12_FEATURE_DATA_D3D12_OPTIONS options;
  bool has_options = false;

  // D3D12_FEATURE_DATA_SHADER_MODEL
  D3D12_FEATURE_DATA_SHADER_MODEL shader_model;
  bool has_shader_model = false;

  // D3D12_FEATURE_DATA_ROOT_SIGNATURE
  D3D12_FEATURE_DATA_ROOT_SIGNATURE root_signature;
  bool has_root_signature = false;

  // D3D12_FEATURE_DATA_FEATURE_LEVELS
  D3D12_FEATURE_DATA_FEATURE_LEVELS feature_levels;
  bool has_feature_levels = false;

  // D3D12_FEATURE_DATA_D3D12_OPTIONS1 through OPTIONS21
  D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1;
  bool has_options1 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS2 options2;
  bool has_options2 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS3 options3;
  bool has_options3 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS4 options4;
  bool has_options4 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5;
  bool has_options5 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS6 options6;
  bool has_options6 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7;
  bool has_options7 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS8 options8;
  bool has_options8 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS9 options9;
  bool has_options9 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS10 options10;
  bool has_options10 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS11 options11;
  bool has_options11 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12;
  bool has_options12 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS13 options13;
  bool has_options13 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS14 options14;
  bool has_options14 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS15 options15;
  bool has_options15 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16;
  bool has_options16 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS17 options17;
  bool has_options17 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS18 options18;
  bool has_options18 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS19 options19;
  bool has_options19 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS20 options20;
  bool has_options20 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS21 options21;
  bool has_options21 = false;

  D3D12_FEATURE_DATA_D3D12_OPTIONS22 options22;
  bool has_options22 = false;

  // D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT
  D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT gpu_virtual_address_support;
  bool has_gpu_virtual_address_support = false;

  // D3D12_FEATURE_DATA_ARCHITECTURE1
  D3D12_FEATURE_DATA_ARCHITECTURE1 architecture1;
  bool has_architecture1 = false;

  // D3D12_FEATURE_DATA_SHADER_CACHE
  D3D12_FEATURE_DATA_SHADER_CACHE shader_cache;
  bool has_shader_cache = false;

  // D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY, keyed by command list type and priority.
  std::unordered_map<UINT64, BOOL> command_queue_priority;
  bool has_command_queue_priority = false;

  // D3D12_FEATURE_DATA_SERIALIZATION
  D3D12_FEATURE_DATA_SERIALIZATION serialization;
  bool has_serialization = false;

  // D3D12_FEATURE_DATA_CROSS_NODE
  D3D12_FEATURE_DATA_CROSS_NODE cross_node;
  bool has_cross_node = false;

  // D3D12_FEATURE_DATA_APPLICATION_SPECIFIC_DRIVER_STATE
  D3D12_FEATURE_DATA_APPLICATION_SPECIFIC_DRIVER_STATE application_specific_driver_state;
  bool has_application_specific_driver_state = false;

  // D3D12_FEATURE_DATA_EXISTING_HEAPS
  D3D12_FEATURE_DATA_EXISTING_HEAPS existing_heaps;
  bool has_existing_heaps = false;

  // D3D12_FEATURE_DATA_BYTECODE_BYPASS_HASH_SUPPORTED
  D3D12_FEATURE_DATA_BYTECODE_BYPASS_HASH_SUPPORTED bytecode_bypass_hash_supported;
  bool has_bytecode_bypass_hash_supported = false;

  // D3D12_FEATURE_DATA_TIGHT_ALIGNMENT
  D3D12_FEATURE_DATA_TIGHT_ALIGNMENT tight_alignment;
  bool has_tight_alignment = false;

  // D3D12_FEATURE_DATA_PREDICATION
  D3D12_FEATURE_DATA_PREDICATION predication;
  bool has_predication = false;

  // D3D12_FEATURE_DATA_HARDWARE_COPY
  D3D12_FEATURE_DATA_HARDWARE_COPY hardware_copy;
  bool has_hardware_copy = false;

  // D3D12_FEATURE_DATA_BARRIER_LAYOUT, keyed by command list type and layout.
  std::unordered_map<UINT64, BOOL> barrier_layout;
  bool has_barrier_layout = false;

  // D3D12_FEATURE_DATA_SHADERCACHE_ABI_SUPPORT
  D3D12_FEATURE_DATA_SHADERCACHE_ABI_SUPPORT shader_cache_abi_support;
  bool has_shader_cache_abi_support = false;

  // Descriptor handle increment sizes
  std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, UINT> descriptor_increment_sizes;

  // Format support data
  struct FormatData
  {
    UINT Support1 = 0;
    UINT Support2 = 0;
    UINT8 PlaneCount = 0;
    std::unordered_map<UINT, D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS> multisample_quality_levels;
    bool has_support = false;
    bool has_plane_count = false;
  };
  std::unordered_map<DXGI_FORMAT, FormatData> formats;

  // RenderDoc compatibility settings
  UINT64 min_alloc_alignment = 4096;
  float image_alloc_size_multiplier = 1.33f;
  UINT64 min_image_alloc_size_increase = 8192;
};

class D3D12Profile
{
public:
  D3D12Profile();
  ~D3D12Profile();

  // Load profile from JSON file
  bool LoadProfile(const std::string &filename);
  bool IsProfileLoaded() const { return m_profile_loaded; }

  HRESULT CheckFeatureSupport(ID3D12Device *pDevice, D3D12_FEATURE Feature,
                              void *pFeatureSupportData, UINT FeatureSupportDataSize) const;

  UINT GetDescriptorHandleIncrementSize(ID3D12Device *pDevice,
                                        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType) const;

  D3D12_RESOURCE_ALLOCATION_INFO GetResourceAllocationInfo(
      ID3D12Device *pDevice, UINT visibleMask, UINT numResourceDescs,
      const D3D12_RESOURCE_DESC *pResourceDescs) const;
  D3D12_RESOURCE_ALLOCATION_INFO GetResourceAllocationInfo1(
      ID3D12Device4 *pDevice, UINT visibleMask, UINT numResourceDescs,
      const D3D12_RESOURCE_DESC *pResourceDescs,
      D3D12_RESOURCE_ALLOCATION_INFO1 *pResourceAllocationInfo1) const;
  D3D12_RESOURCE_ALLOCATION_INFO GetResourceAllocationInfo2(
      ID3D12Device8 *pDevice, UINT visibleMask, UINT numResourceDescs,
      const D3D12_RESOURCE_DESC1 *pResourceDescs,
      D3D12_RESOURCE_ALLOCATION_INFO1 *pResourceAllocationInfo1) const;
  D3D12_RESOURCE_ALLOCATION_INFO GetResourceAllocationInfo3(
      ID3D12Device12 *pDevice, UINT visibleMask, UINT numResourceDescs,
      const D3D12_RESOURCE_DESC1 *pResourceDescs, const UINT32 *pNumCastableFormats,
      const DXGI_FORMAT *const *ppCastableFormats,
      D3D12_RESOURCE_ALLOCATION_INFO1 *pResourceAllocationInfo1) const;
private:
  bool ParseJSON(std::ifstream &json_file);

  D3D12ProfileData m_profile_data;
  bool m_profile_loaded;
};
