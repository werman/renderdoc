#!/usr/bin/env python3
"""
D3D12 Profile Intersection Script

Creates an intersection of two D3D12 profiles, where the resulting profile
represents the subset of capabilities that both devices support.
"""

import json
import sys
import argparse
from typing import Any, Dict, Union


# Most numeric feature values are limits or tiers, where the intersection is the
# lower value. These fields are either lower bounds or bitfields and need
# different handling.
MAX_FIELDS = {
    "WaveLaneCountMin",
    "MinimumABISupportVersion",
}

BITMASK_FIELDS = {
    "MinPrecisionSupport",
    "SupportFlags",
    "WriteBufferImmediateSupportFlags",
    "SupportedSampleCountsWithNoOutputs",
    "Flags",
}

# Inputs to CheckFeatureSupport must not be treated as capabilities. D3d12info
# currently serialises NodeIndex; preserve it when making a conservative value.
INPUT_FIELDS = {
    "NodeIndex",
    "CommandListType",
    "Priority",
    "Format",
    "Dimension",
    "DestHeapProperties",
    "Layout",
}


def _parse_numeric_string(value: Any):
    if not isinstance(value, str):
        return None
    try:
        return int(value, 0)
    except ValueError:
        return None


def _format_like(value: int, original: Any) -> Any:
    return str(value) if isinstance(original, str) else value


def intersect_numeric_value(field: str, val1: Union[int, float],
                            val2: Union[int, float]) -> Union[int, float]:
    """Intersect a numeric field."""
    if field in BITMASK_FIELDS and isinstance(val1, int) and isinstance(val2, int):
        return val1 & val2
    if field in MAX_FIELDS:
        return max(val1, val2)
    return min(val1, val2)


def conservative_value(field: str, value: Any) -> Any:
    """Return a value that does not claim support missing from one profile."""
    if field in INPUT_FIELDS:
        return value
    if isinstance(value, bool):
        return False
    if isinstance(value, (int, float)):
        return 0
    if isinstance(value, str):
        return "0" if _parse_numeric_string(value) is not None else ""
    if isinstance(value, dict):
        return {key: conservative_value(key, child) for key, child in value.items()}
    if isinstance(value, list):
        return []
    return value


def intersect_value(field: str, val1: Any, val2: Any) -> Any:
    """Intersect two values from the same feature-data field."""
    if isinstance(val1, bool) or isinstance(val2, bool):
        if isinstance(val1, bool) and isinstance(val2, bool):
            return val1 and val2
        return False

    if isinstance(val1, (int, float)) and isinstance(val2, (int, float)):
        return intersect_numeric_value(field, val1, val2)

    numeric1 = _parse_numeric_string(val1)
    numeric2 = _parse_numeric_string(val2)
    if numeric1 is not None and numeric2 is not None:
        result = intersect_numeric_value(field, numeric1, numeric2)
        return _format_like(result, val1)

    if isinstance(val1, dict) and isinstance(val2, dict):
        return intersect_dict(val1, val2)

    if isinstance(val1, list) and isinstance(val2, list):
        return [item for item in val1 if item in val2]

    # Non-numeric strings (notably shader-cache adapter families) are only
    # meaningful when equal. An empty string describes no shared value.
    if isinstance(val1, str) and isinstance(val2, str):
        return val1 if val1 == val2 else ""

    return val1 if val1 == val2 else conservative_value(field, val1)


def intersect_dict(dict1: Dict[str, Any], dict2: Dict[str, Any]) -> Dict[str, Any]:
    """Recursively intersect two dictionaries."""
    result = {}
    all_keys = set(dict1.keys()) | set(dict2.keys())

    for key in all_keys:
        if key not in dict1:
            result[key] = conservative_value(key, dict2[key])
        elif key not in dict2:
            result[key] = conservative_value(key, dict1[key])
        else:
            result[key] = intersect_value(key, dict1[key], dict2[key])

    return result


def intersect_feature_data(feature_name: str, adapter1: Dict[str, Any],
                           adapter2: Dict[str, Any]) -> Dict[str, Any]:
    """Intersect a D3D12 feature data structure."""
    if feature_name not in adapter1 and feature_name not in adapter2:
        return {}
    if feature_name not in adapter1:
        return conservative_value(feature_name, adapter2[feature_name])
    if feature_name not in adapter2:
        return conservative_value(feature_name, adapter1[feature_name])

    obj1 = adapter1[feature_name]
    obj2 = adapter2[feature_name]
    if feature_name == "D3D12_FEATURE_DATA_SHADERCACHE_ABI_SUPPORT":
        if obj1.get("szAdapterFamily") != obj2.get("szAdapterFamily"):
            return conservative_value(feature_name, obj1)

    result = intersect_dict(obj1, obj2)
    if feature_name == "D3D12_FEATURE_DATA_SHADERCACHE_ABI_SUPPORT":
        minimum = _parse_numeric_string(result.get("MinimumABISupportVersion"))
        maximum = _parse_numeric_string(result.get("MaximumABISupportVersion"))
        if minimum is None and isinstance(result.get("MinimumABISupportVersion"), int):
            minimum = result["MinimumABISupportVersion"]
        if maximum is None and isinstance(result.get("MaximumABISupportVersion"), int):
            maximum = result["MaximumABISupportVersion"]
        if minimum is not None and maximum is not None and minimum > maximum:
            return conservative_value(feature_name, result)

    return result


def intersect_descriptor_handle_increment_size(adapter1: Dict[str, Any],
                                               adapter2: Dict[str, Any]) -> Dict[str, Any]:
    """Intersect GetDescriptorHandleIncrementSize by taking MAX of each field."""
    feature_name = "GetDescriptorHandleIncrementSize"

    if feature_name not in adapter1 and feature_name not in adapter2:
        return {}

    if feature_name not in adapter1:
        return adapter2[feature_name].copy()
    if feature_name not in adapter2:
        return adapter1[feature_name].copy()

    obj1 = adapter1[feature_name]
    obj2 = adapter2[feature_name]
    result = {}

    all_fields = set(obj1.keys()) | set(obj2.keys())

    for field in all_fields:
        if field in obj1 and field in obj2:
            val1 = obj1[field]
            val2 = obj2[field]
            if isinstance(val1, (int, float)) and isinstance(val2, (int, float)):
                result[field] = max(val1, val2)
            else:
                result[field] = val1
        elif field in obj1:
            result[field] = obj1[field]
        elif field in obj2:
            result[field] = obj2[field]

    return result


def intersect_formats(adapter1: Dict[str, Any], adapter2: Dict[str, Any]) -> Dict[str, Any]:
    """Intersect Formats section - only formats present in both profiles."""
    feature_name = "Formats"

    if feature_name not in adapter1 or feature_name not in adapter2:
        return {}

    formats1 = adapter1[feature_name]
    formats2 = adapter2[feature_name]
    result = {}

    # Only include formats that exist in both profiles
    common_formats = set(formats1.keys()) & set(formats2.keys())

    for format_key in common_formats:
        fmt1 = formats1[format_key]
        fmt2 = formats2[format_key]
        fmt_result = {}

        if "Support1" in fmt1 and "Support1" in fmt2:
            fmt_result["Support1"] = fmt1["Support1"] & fmt2["Support1"]

        if "Support2" in fmt1 and "Support2" in fmt2:
            fmt_result["Support2"] = fmt1["Support2"] & fmt2["Support2"]

        if "PlaneCount" in fmt1 and "PlaneCount" in fmt2:
            fmt_result["PlaneCount"] = min(fmt1["PlaneCount"], fmt2["PlaneCount"])

        # Intersect MultisampleQualityLevels
        if "MultisampleQualityLevels" in fmt1 and "MultisampleQualityLevels" in fmt2:
            ms1 = fmt1["MultisampleQualityLevels"]
            ms2 = fmt2["MultisampleQualityLevels"]
            ms_result = {}

            # Only include sample counts that exist in both profiles
            common_sample_counts = set(ms1.keys()) & set(ms2.keys())

            for sample_count in common_sample_counts:
                ms_entry1 = ms1[sample_count]
                ms_entry2 = ms2[sample_count]
                ms_entry_result = {}

                ms_entry_result["NumQualityLevels"] = min(
                    ms_entry1["NumQualityLevels"], ms_entry2["NumQualityLevels"])
                ms_entry_result["Flags"] = ms_entry1["Flags"] & ms_entry2["Flags"]
                ms_result[sample_count] = ms_entry_result

            fmt_result["MultisampleQualityLevels"] = ms_result

        # Only add format if it has at least one field
        if fmt_result:
            result[format_key] = fmt_result

    return result


def intersect_profiles(profile1: Dict[str, Any], profile2: Dict[str, Any]) -> Dict[str, Any]:
    """Create an intersection of two D3D12 profiles."""
    # Start with a copy of profile1's structure
    result = {
        "Header": profile1.get("Header", {}).copy(),
        "SystemInfo": profile1.get("SystemInfo", {}).copy(),
        "Adapters": []
    }

    # Get first adapter from each profile
    if "Adapters" not in profile1 or len(profile1["Adapters"]) == 0:
        raise ValueError("Profile 1 has no adapters")
    if "Adapters" not in profile2 or len(profile2["Adapters"]) == 0:
        raise ValueError("Profile 2 has no adapters")

    adapter1 = profile1["Adapters"][0]
    adapter2 = profile2["Adapters"][0]

    # Create intersection adapter
    intersection_adapter = {
        "AdapterIndex": 0,
    }

    # Copy non-feature fields from first adapter (like DXGI_ADAPTER_DESC3)
    # These are device-specific, so we'll keep from adapter1
    for key in adapter1:
        if (not key.startswith("D3D12_FEATURE_DATA_") and
            key != "GetDescriptorHandleIncrementSize" and
            key != "Formats"):
            intersection_adapter[key] = adapter1[key]

    # Intersect every feature structure emitted by D3d12info. This includes new
    # feature blocks automatically, so the producer and this script cannot drift
    # merely because another D3D12_OPTIONS revision was added.
    feature_names = sorted(
        key for key in set(adapter1.keys()) | set(adapter2.keys())
        if key.startswith("D3D12_FEATURE_DATA_")
    )
    for feature_name in feature_names:
        intersection = intersect_feature_data(feature_name, adapter1, adapter2)
        if intersection:
            intersection_adapter[feature_name] = intersection

    # Intersect GetDescriptorHandleIncrementSize (use MAX)
    descriptor_sizes = intersect_descriptor_handle_increment_size(
        adapter1, adapter2)
    if descriptor_sizes:
        intersection_adapter["GetDescriptorHandleIncrementSize"] = descriptor_sizes

    # Intersect Formats
    formats = intersect_formats(adapter1, adapter2)
    if formats:
        intersection_adapter["Formats"] = formats

    result["Adapters"] = [intersection_adapter]

    return result


def main():
    parser = argparse.ArgumentParser(
        description="Create an intersection of two D3D12 profiles"
    )
    parser.add_argument(
        "profile1", help="Path to first D3D12 profile JSON file")
    parser.add_argument(
        "profile2", help="Path to second D3D12 profile JSON file")
    parser.add_argument("-o", "--output", help="Output file path (default: stdout)",
                        default=None)

    args = parser.parse_args()

    # Load profiles
    try:
        with open(args.profile1, 'r') as f:
            profile1 = json.load(f)
    except Exception as e:
        print(
            f"Error loading profile 1 ({args.profile1}): {e}", file=sys.stderr)
        sys.exit(1)

    try:
        with open(args.profile2, 'r') as f:
            profile2 = json.load(f)
    except Exception as e:
        print(
            f"Error loading profile 2 ({args.profile2}): {e}", file=sys.stderr)
        sys.exit(1)

    # Create intersection
    try:
        intersection = intersect_profiles(profile1, profile2)
    except Exception as e:
        print(f"Error creating intersection: {e}", file=sys.stderr)
        sys.exit(1)

    # Output result
    output_json = json.dumps(intersection, indent=4)

    if args.output:
        try:
            with open(args.output, 'w') as f:
                f.write(output_json)
            print(f"Intersection saved to {args.output}", file=sys.stderr)
        except Exception as e:
            print(f"Error writing output file: {e}", file=sys.stderr)
            sys.exit(1)
    else:
        print(output_json)


if __name__ == "__main__":
    main()
