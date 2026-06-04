#!/usr/bin/env python3
"""
版本号自动管理脚本
用于自动增加版本号
"""

import re
import sys
import os


def bump_version(version_file_path, part='patch'):
    """
    增加版本号
    :param version_file_path: version.h 文件路径
    :param part: 要增加的版本部分 (major, minor, patch)
    """
    with open(version_file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 找到当前版本号
    major_match = re.search(r'#define VERSION_MAJOR\s+(\d+)', content)
    minor_match = re.search(r'#define VERSION_MINOR\s+(\d+)', content)
    patch_match = re.search(r'#define VERSION_PATCH\s+(\d+)', content)
    
    if not major_match or not minor_match or not patch_match:
        print("错误：无法在文件中找到版本号！")
        return False
    
    major = int(major_match.group(1))
    minor = int(minor_match.group(1))
    patch = int(patch_match.group(1))
    
    old_version = f"v{major}.{minor}.{patch}"
    
    # 增加版本号
    if part == 'major':
        major += 1
        minor = 0
        patch = 0
    elif part == 'minor':
        minor += 1
        patch = 0
    else:  # patch
        patch += 1
    
    new_version = f"v{major}.{minor}.{patch}"
    
    # 替换版本号
    content = re.sub(r'#define VERSION_MAJOR\s+\d+', f'#define VERSION_MAJOR {major}', content)
    content = re.sub(r'#define VERSION_MINOR\s+\d+', f'#define VERSION_MINOR {minor}', content)
    content = re.sub(r'#define VERSION_PATCH\s+\d+', f'#define VERSION_PATCH {patch}', content)
    
    with open(version_file_path, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"版本号已更新：{old_version} -> {new_version}")
    return True


def update_cmake(cmake_file_path, version_str):
    """
    更新 CMakeLists.txt 中的版本号
    """
    with open(cmake_file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 去掉 'v' 前缀
    cmake_version = version_str.lstrip('v')
    
    # 替换版本号
    content = re.sub(r'project\(DrawingHelper VERSION [\d\.]+\s+', f'project(DrawingHelper VERSION {cmake_version} ', content)
    
    with open(cmake_file_path, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"CMakeLists.txt 已更新到版本 {cmake_version}")


if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser(description='自动增加版本号')
    parser.add_argument('part', nargs='?', default='patch',
                        choices=['major', 'minor', 'patch'],
                        help='要增加的版本部分 (默认: patch)')
    parser.add_argument('--version-file', default='src/version.h',
                        help='版本号文件路径 (默认: src/version.h)')
    parser.add_argument('--cmake-file', default='CMakeLists.txt',
                        help='CMakeLists.txt 文件路径 (默认: CMakeLists.txt)')
    parser.add_argument('--no-cmake', action='store_true',
                        help='不更新 CMakeLists.txt')
    
    args = parser.parse_args()
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    version_file = os.path.join(script_dir, args.version_file)
    cmake_file = os.path.join(script_dir, args.cmake_file)
    
    if not os.path.exists(version_file):
        print(f"错误：找不到版本文件 {version_file}")
        sys.exit(1)
    
    # 先保存当前版本号
    with open(version_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    major_match = re.search(r'#define VERSION_MAJOR\s+(\d+)', content)
    minor_match = re.search(r'#define VERSION_MINOR\s+(\d+)', content)
    patch_match = re.search(r'#define VERSION_PATCH\s+(\d+)', content)
    
    old_version = f"v{major_match.group(1)}.{minor_match.group(1)}.{patch_match.group(1)}"
    
    # 增加版本号
    if bump_version(version_file, args.part):
        # 更新 CMakeLists.txt（如果需要）
        if not args.no_cmake:
            # 重新读取新的版本号
            with open(version_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            major_match = re.search(r'#define VERSION_MAJOR\s+(\d+)', content)
            minor_match = re.search(r'#define VERSION_MINOR\s+(\d+)', content)
            patch_match = re.search(r'#define VERSION_PATCH\s+(\d+)', content)
            
            new_version = f"v{major_match.group(1)}.{minor_match.group(1)}.{patch_match.group(1)}"
            update_cmake(cmake_file, new_version)
