#!/usr/bin/env python3
"""
D3D12 Profile Intersection Script

Creates an intersection of two D3D12 profiles, where the resulting profile
represents the subset of capabilities that both devices support.
"""

import json
import sys
import argparse
from typing import Any, Dict, List, Union


def intersect_numeric_fields(obj1: Dict[str, Any], obj2: Dict[str, Any],
                             result: Dict[str, Any], field: str) -> None:
    """Intersect a numeric field by taking the minimum."""
    if field in obj1 and field in obj2:
        val1 = obj1[field]
        val2 = obj2[field]
        if isinstance(val1, (int, float)) and isinstance(val2, (int, float)):
            result[field] = min(val1, val2)
        else:
            # If types don't match, keep the first one
            result[field] = val1
    elif field in obj1:
        result[field] = obj1[field]
    elif field in obj2:
        result[field] = obj2[field]


def intersect_boolean_fields(obj1: Dict[str, Any], obj2: Dict[str, Any],
                             result: Dict[str, Any], field: str) -> None:
    """Intersect a boolean field by taking AND (both must be true)."""
    if field in obj1 and field in obj2:
        result[field] = obj1[field] and obj2[field]
    elif field in obj1:
        result[field] = False  # If missing in one, assume false
    elif field in obj2:
        result[field] = False  # If missing in one, assume false


def intersect_feature_data(feature_name: str, adapter1: Dict[str, Any],
                           adapter2: Dict[str, Any]) -> Dict[str, Any]:
    """Intersect a D3D12 feature data structure."""
    if feature_name not in adapter1 and feature_name not in adapter2:
        return {}

    if feature_name not in adapter1:
        return adapter2[feature_name].copy()
    if feature_name not in adapter2:
        return adapter1[feature_name].copy()

    obj1 = adapter1[feature_name]
    obj2 = adapter2[feature_name]
    result = {}

    # Process all fields
    all_fields = set(obj1.keys()) | set(obj2.keys())

    for field in all_fields:
        val1 = obj1.get(field)
        val2 = obj2.get(field)

        # Determine field type and intersect accordingly
        if isinstance(val1, bool) or isinstance(val2, bool):
            # Boolean field - use AND
            intersect_boolean_fields(obj1, obj2, result, field)
        elif isinstance(val1, (int, float)) or isinstance(val2, (int, float)):
            # Numeric field - use MIN
            intersect_numeric_fields(obj1, obj2, result, field)
        elif isinstance(val1, str) or isinstance(val2, str):
            # String field - keep from first profile
            result[field] = obj1.get(field, obj2.get(field))
        elif isinstance(val1, dict) or isinstance(val2, dict):
            # Nested dict - recursively intersect
            if field in obj1 and field in obj2:
                result[field] = intersect_dict(obj1[field], obj2[field])
            else:
                result[field] = obj1.get(field, obj2.get(field))
        else:
            # Other types - keep from first profile
            result[field] = obj1.get(field, obj2.get(field))

    return result


def intersect_dict(dict1: Dict[str, Any], dict2: Dict[str, Any]) -> Dict[str, Any]:
    """Recursively intersect two dictionaries."""
    result = {}
    all_keys = set(dict1.keys()) | set(dict2.keys())

    for key in all_keys:
        if key not in dict1:
            result[key] = dict2[key]
        elif key not in dict2:
            result[key] = dict1[key]
        elif isinstance(dict1[key], dict) and isinstance(dict2[key], dict):
            result[key] = intersect_dict(dict1[key], dict2[key])
        elif isinstance(dict1[key], (int, float)) and isinstance(dict2[key], (int, float)):
            result[key] = min(dict1[key], dict2[key])
        elif isinstance(dict1[key], bool) and isinstance(dict2[key], bool):
            result[key] = dict1[key] and dict2[key]
        else:
            result[key] = dict1[key]

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

        if "PlaneCount" in fmt1:
            fmt_result["PlaneCount"] = min(fmt1["PlaneCount"], fmt2["PlaneCount"])

        # Intersect MultisampleQualityLevels
        if "MultisampleQualityLevels" in fmt1:
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

    # Intersect D3D12_FEATURE_DATA_D3D12_OPTIONS through OPTIONS21
    for i in range(22):  # OPTIONS through OPTIONS21
        if i == 0:
            feature_name = "D3D12_FEATURE_DATA_D3D12_OPTIONS"
        else:
            feature_name = f"D3D12_FEATURE_DATA_D3D12_OPTIONS{i}"

        intersection = intersect_feature_data(feature_name, adapter1, adapter2)
        if intersection:
            intersection_adapter[feature_name] = intersection

    # Intersect D3D12_FEATURE_DATA_SHADER_MODEL
    shader_model = intersect_feature_data(
        "D3D12_FEATURE_DATA_SHADER_MODEL", adapter1, adapter2)
    if shader_model:
        intersection_adapter["D3D12_FEATURE_DATA_SHADER_MODEL"] = shader_model

    # Intersect D3D12_FEATURE_DATA_ROOT_SIGNATURE
    root_sig = intersect_feature_data(
        "D3D12_FEATURE_DATA_ROOT_SIGNATURE", adapter1, adapter2)
    if root_sig:
        # Clamp HighestVersion to maximum 2
        if "HighestVersion" in root_sig:
            root_sig["HighestVersion"] = min(root_sig["HighestVersion"], 2)
        intersection_adapter["D3D12_FEATURE_DATA_ROOT_SIGNATURE"] = root_sig

    # Intersect D3D12_FEATURE_DATA_FEATURE_LEVELS
    feature_levels = intersect_feature_data(
        "D3D12_FEATURE_DATA_FEATURE_LEVELS", adapter1, adapter2)
    if feature_levels:
        intersection_adapter["D3D12_FEATURE_DATA_FEATURE_LEVELS"] = feature_levels

    # Intersect GetDescriptorHandleIncrementSize (use MAX)
    descriptor_sizes = intersect_descriptor_handle_increment_size(
        adapter1, adapter2)
    if descriptor_sizes:
        intersection_adapter["GetDescriptorHandleIncrementSize"] = descriptor_sizes

    # Intersect Formats
    formats = intersect_formats(adapter1, adapter2)
    if formats:
        intersection_adapter["Formats"] = formats

    # Copy other feature data that we're not explicitly intersecting
    # (keep from adapter1 as baseline)
    for key in adapter1:
        if (key.startswith("D3D12_FEATURE_DATA_") and
            key not in intersection_adapter and
            not key.startswith("D3D12_FEATURE_DATA_D3D12_OPTIONS") and
            key != "D3D12_FEATURE_DATA_SHADER_MODEL" and
            key != "D3D12_FEATURE_DATA_ROOT_SIGNATURE" and
                key != "D3D12_FEATURE_DATA_FEATURE_LEVELS"):
            intersection_adapter[key] = adapter1[key]

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
