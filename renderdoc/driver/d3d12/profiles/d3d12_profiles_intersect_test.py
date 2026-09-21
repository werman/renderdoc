#!/usr/bin/env python3

import pathlib
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from d3d12_profiles_intersect import intersect_profiles


def profile(adapter):
    return {"Header": {}, "SystemInfo": {}, "Adapters": [adapter]}


class D3D12ProfileIntersectionTests(unittest.TestCase):
    def test_intersects_current_d3d12info_features(self):
        adapter1 = {
            "D3D12_FEATURE_DATA_D3D12_OPTIONS": {"MinPrecisionSupport": 0b11},
            "D3D12_FEATURE_DATA_D3D12_OPTIONS22": {
                "ShaderExecutionReorderingActuallyReorders": True,
                "CreateByteOffsetViewsSupported": True,
                "Max1DDispatchSize": 1024,
                "Max1DDispatchMeshSize": 512,
            },
            "D3D12_FEATURE_DATA_SHADER_CACHE": {"SupportFlags": 0b1101},
            "D3D12_FEATURE_DATA_ROOT_SIGNATURE": {"HighestVersion": 3},
            "D3D12_FEATURE_DATA_BARRIER_LAYOUT": {
                "D3D12_COMMAND_LIST_TYPE_DIRECT": {"D3D12_BARRIER_LAYOUT_COMMON": True,
                                                   "D3D12_BARRIER_LAYOUT_RENDER_TARGET": True},
            },
        }
        adapter2 = {
            "D3D12_FEATURE_DATA_D3D12_OPTIONS": {"MinPrecisionSupport": 0b10},
            "D3D12_FEATURE_DATA_D3D12_OPTIONS22": {
                "ShaderExecutionReorderingActuallyReorders": False,
                "CreateByteOffsetViewsSupported": True,
                "Max1DDispatchSize": 512,
                "Max1DDispatchMeshSize": 1024,
            },
            "D3D12_FEATURE_DATA_SHADER_CACHE": {"SupportFlags": 0b1011},
            "D3D12_FEATURE_DATA_ROOT_SIGNATURE": {"HighestVersion": 3},
            "D3D12_FEATURE_DATA_BARRIER_LAYOUT": {
                "D3D12_COMMAND_LIST_TYPE_DIRECT": {"D3D12_BARRIER_LAYOUT_COMMON": True},
            },
        }

        result = intersect_profiles(profile(adapter1), profile(adapter2))["Adapters"][0]
        options22 = result["D3D12_FEATURE_DATA_D3D12_OPTIONS22"]
        self.assertFalse(options22["ShaderExecutionReorderingActuallyReorders"])
        self.assertTrue(options22["CreateByteOffsetViewsSupported"])
        self.assertEqual(options22["Max1DDispatchSize"], 512)
        self.assertEqual(options22["Max1DDispatchMeshSize"], 512)
        self.assertEqual(result["D3D12_FEATURE_DATA_SHADER_CACHE"]["SupportFlags"], 0b1001)
        self.assertEqual(result["D3D12_FEATURE_DATA_D3D12_OPTIONS"]["MinPrecisionSupport"], 0b10)
        self.assertEqual(result["D3D12_FEATURE_DATA_ROOT_SIGNATURE"]["HighestVersion"], 3)
        barriers = result["D3D12_FEATURE_DATA_BARRIER_LAYOUT"]["D3D12_COMMAND_LIST_TYPE_DIRECT"]
        self.assertTrue(barriers["D3D12_BARRIER_LAYOUT_COMMON"])
        self.assertFalse(barriers["D3D12_BARRIER_LAYOUT_RENDER_TARGET"])

    def test_missing_feature_is_conservative(self):
        adapter = {
            "D3D12_FEATURE_DATA_D3D12_TIGHT_ALIGNMENT": {"SupportTier": 1},
            "D3D12_FEATURE_DATA_ARCHITECTURE1": {
                "NodeIndex": 0,
                "UMA": True,
            },
        }
        result = intersect_profiles(profile(adapter), profile({}))["Adapters"][0]
        self.assertEqual(
            result["D3D12_FEATURE_DATA_D3D12_TIGHT_ALIGNMENT"]["SupportTier"], 0)
        architecture = result["D3D12_FEATURE_DATA_ARCHITECTURE1"]
        self.assertEqual(architecture["NodeIndex"], 0)
        self.assertFalse(architecture["UMA"])

    def test_intersects_uint64_strings(self):
        key = "D3D12_FEATURE_DATA_SHADERCACHE_ABI_SUPPORT"
        adapter1 = {key: {
            "szAdapterFamily": "family",
            "MinimumABISupportVersion": "10",
            "MaximumABISupportVersion": "30",
        }}
        adapter2 = {key: {
            "szAdapterFamily": "family",
            "MinimumABISupportVersion": "20",
            "MaximumABISupportVersion": "40",
        }}
        result = intersect_profiles(profile(adapter1), profile(adapter2))["Adapters"][0][key]
        self.assertEqual(result["szAdapterFamily"], "family")
        self.assertEqual(result["MinimumABISupportVersion"], "20")
        self.assertEqual(result["MaximumABISupportVersion"], "30")

    def test_rejects_incompatible_shader_cache_abi(self):
        key = "D3D12_FEATURE_DATA_SHADERCACHE_ABI_SUPPORT"
        adapter1 = {key: {
            "szAdapterFamily": "family-a",
            "MinimumABISupportVersion": "10",
            "MaximumABISupportVersion": "30",
        }}
        adapter2 = {key: {
            "szAdapterFamily": "family-b",
            "MinimumABISupportVersion": "20",
            "MaximumABISupportVersion": "40",
        }}
        result = intersect_profiles(profile(adapter1), profile(adapter2))["Adapters"][0][key]
        self.assertEqual(result["szAdapterFamily"], "")
        self.assertEqual(result["MinimumABISupportVersion"], "0")
        self.assertEqual(result["MaximumABISupportVersion"], "0")


if __name__ == "__main__":
    unittest.main()
