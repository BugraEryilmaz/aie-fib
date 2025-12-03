#!/usr/bin/env python3
"""
Script to rename Verilog/SystemVerilog module declarations by adding folder name as suffix.

This script:
1. Iterates over all folders in src/rtl
2. Finds all .v and .sv files in each folder
3. Renames all module declarations by adding the folder name as a suffix
4. Also updates module instantiations to use the new module names
5. Saves the modified files to a specified output directory
"""

import os
import re
import argparse
from pathlib import Path
from typing import Dict, Set


def find_module_declarations(content: str) -> Set[str]:
    """
    Find all module declarations in the content.
    Returns a set of module names.
    """
    # Match module declarations: module <name> ( or module <name> #(
    pattern = r'^\s*module\s+(\w+)\s*(?:#?\s*\(|;)'
    matches = re.findall(pattern, content, re.MULTILINE)
    return set(matches)


def rename_modules_in_content(content: str, module_names: Set[str], suffix: str, top_module: str) -> str:
    """
    Rename all module declarations and instantiations in the content.
    Adds the suffix to each module name, except for the top module.
    
    Args:
        content: The file content to process
        module_names: Set of module names to rename
        suffix: The suffix to add (folder name)
        top_module: The top module name (same as folder name) - will not be renamed
    """
    result = content
    
    # Sort by length (longest first) to avoid partial replacements
    sorted_names = sorted(module_names, key=len, reverse=True)
    
    for module_name in sorted_names:
        # Skip the top module - it should already have the correct name
        if module_name == top_module:
            continue
        new_name = f"{module_name}_{suffix}"
        
        # Replace module declarations: module <name>
        # This pattern matches: module name ( or module name #( or module name ;
        result = re.sub(
            rf'(\bmodule\s+){module_name}(\s*(?:#?\s*\(|;))',
            rf'\g<1>{new_name}\g<2>',
            result
        )
        
        # Replace module instantiations
        # Pattern: <module_name> <instance_name> ( or <module_name> #(
        # Be careful not to replace inside strings or comments
        result = re.sub(
            rf'(\s+){module_name}(\s+\w+\s*\()',
            rf'\g<1>{new_name}\g<2>',
            result
        )
        
        # Also handle instantiation with parameters: ModuleName #(
        result = re.sub(
            rf'(\s+){module_name}(\s*#\s*\()',
            rf'\g<1>{new_name}\g<2>',
            result
        )
        
    return result


def process_folder(src_folder: Path, output_dir: Path, suffix: str) -> None:
    """
    Process all Verilog/SystemVerilog files in a folder.
    """
    # Find all .v and .sv files
    verilog_files = list(src_folder.glob("*.v")) + list(src_folder.glob("*.sv"))
    
    if not verilog_files:
        print(f"  No Verilog files found in {src_folder}")
        return
    
    # First pass: collect all module names from all files in this folder
    all_modules: Set[str] = set()
    file_contents: Dict[Path, str] = {}
    
    for file_path in verilog_files:
        with open(file_path, 'r') as f:
            content = f.read()
            file_contents[file_path] = content
            modules = find_module_declarations(content)
            all_modules.update(modules)
            print(f"  Found modules in {file_path.name}: {modules}")
    
    # Second pass: rename modules in all files (except the top module)
    output_folder = output_dir / suffix
    output_folder.mkdir(parents=True, exist_ok=True)
    
    # The top module has the same name as the folder
    top_module = suffix
    
    for file_path, content in file_contents.items():
        new_content = rename_modules_in_content(content, all_modules, suffix, top_module)
        
        output_file = output_folder / file_path.name
        with open(output_file, 'w') as f:
            f.write(new_content)
        print(f"  Saved: {output_file}")


def main():
    parser = argparse.ArgumentParser(
        description="Rename Verilog modules by adding folder name as suffix"
    )
    parser.add_argument(
        "--src", "-s",
        default="src/rtl",
        help="Source directory containing RTL folders (default: src/rtl)"
    )
    parser.add_argument(
        "--output", "-o",
        default="output_rtl",
        help="Output directory for modified files (default: output_rtl)"
    )
    
    args = parser.parse_args()
    
    # Get absolute paths
    script_dir = Path(__file__).parent
    base_dir = script_dir.parent  # aie-fib directory
    
    src_dir = Path(args.src)
    if not src_dir.is_absolute():
        src_dir = base_dir / src_dir
    
    output_dir = Path(args.output)
    if not output_dir.is_absolute():
        output_dir = base_dir / output_dir
    
    if not src_dir.exists():
        print(f"Error: Source directory '{src_dir}' does not exist")
        return 1
    
    # Create output directory
    output_dir.mkdir(parents=True, exist_ok=True)
    
    print(f"Source directory: {src_dir}")
    print(f"Output directory: {output_dir}")
    print()
    
    # Process each folder in src/rtl
    folders = [f for f in src_dir.iterdir() if f.is_dir()]
    
    if not folders:
        print(f"No folders found in {src_dir}")
        return 1
    
    for folder in sorted(folders):
        suffix = folder.name
        print(f"Processing folder: {folder.name}")
        process_folder(folder, output_dir, suffix)
        print()
    
    print("Done!")
    return 0


if __name__ == "__main__":
    exit(main())
