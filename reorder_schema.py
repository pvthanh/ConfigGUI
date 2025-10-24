#!/usr/bin/env python3
"""
Reorganize schema properties in the correct order matching config_new.json
"""
import json

# Load the schema
with open('resources/schemas/config.schema.json', 'r') as f:
    schema = json.load(f)

# Define the desired order for configureRules properties
desired_order = [
    "3DFileCompletenessRule",
    "BuildingTableRule",
    "GeometryEquivalenceRule",
    "LinksGeometryRule",
    "LinksTableRule",
    "ObjMtlReferenceRule",
    "PoiTableRule",
    "RdfIntegrityRule",
    "RootStructureRule",
    "ShapefileCompletenessRule",
    "ShapefilePrjRule",
    "dataTypeValidationRule"
]

# Reorder configureRules properties
if "configureRules" in schema["properties"]:
    old_props = schema["properties"]["configureRules"]["properties"]
    new_props = {}
    
    for rule_name in desired_order:
        if rule_name in old_props:
            new_props[rule_name] = old_props[rule_name]
        else:
            print(f"Warning: {rule_name} not found in schema properties")
    
    # Check if there are any properties not in desired_order
    for rule_name in old_props:
        if rule_name not in desired_order:
            print(f"Warning: {rule_name} not in desired order list, adding at end")
            new_props[rule_name] = old_props[rule_name]
    
    schema["properties"]["configureRules"]["properties"] = new_props
    print(f"✓ Reordered configureRules properties from {len(old_props)} items")
    print(f"  New order: {list(new_props.keys())}")

# Save the reordered schema
with open('resources/schemas/config.schema.json', 'w') as f:
    json.dump(schema, f, indent=2)

print("✓ Schema saved with reordered properties")
