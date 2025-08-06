#!/usr/bin/env python3
"""
颜色映射修复验证脚本

此脚本用于验证颜色映射功能的修复效果
"""

import os
import sys
import time
import subprocess
from pathlib import Path

def check_test_files():
    """检查测试文件是否存在"""
    test_files = [
        "simple_house_10k.pcd",
        "complex_building_50k.pcd", 
        "terrain_with_buildings_100k.pcd"
    ]
    
    existing_files = []
    for file in test_files:
        if os.path.exists(file):
            existing_files.append(file)
            print(f"✅ 找到测试文件: {file}")
        else:
            print(f"❌ 缺少测试文件: {file}")
    
    return existing_files

def check_executable():
    """检查可执行文件是否存在"""
    exe_path = Path("release/simple_demo.exe")
    if exe_path.exists():
        print(f"✅ 找到可执行文件: {exe_path}")
        return True
    else:
        print(f"❌ 未找到可执行文件: {exe_path}")
        return False

def print_test_instructions():
    """打印测试说明"""
    print("\n" + "="*60)
    print("颜色映射功能测试说明")
    print("="*60)
    
    print("\n📋 测试步骤：")
    print("1. 启动程序后，点击'生成测试数据'按钮")
    print("2. 或者点击'加载点云文件'加载现有的PCD文件")
    print("3. 在颜色映射面板中测试以下功能：")
    
    print("\n🎨 颜色方案测试：")
    print("   - 选择'高度'：应显示蓝色到红色的高度渐变")
    print("   - 选择'强度'：应根据强度值显示不同颜色")
    print("   - 选择'分类'：应显示离散的分类颜色")
    print("   - 选择'RGB'：应显示原始RGB颜色")
    
    print("\n📊 数值范围测试：")
    print("   - 调整最小值和最大值")
    print("   - 观察点云颜色是否实时更新")
    print("   - 颜色条应该同步更新")
    
    print("\n🔍 预期结果：")
    print("   ✅ 切换颜色方案时，点云颜色立即改变")
    print("   ✅ 调整数值范围时，颜色映射实时更新")
    print("   ✅ 颜色条正确显示当前映射")
    print("   ✅ 控制台输出相关调试信息")
    
    print("\n⚠️  已知限制：")
    print("   - LOD模式下只支持高度映射")
    print("   - 非LOD模式支持所有颜色方案")
    
    print("\n🐛 如果发现问题：")
    print("   - 检查控制台输出的调试信息")
    print("   - 确认点云数据包含相应的属性信息")
    print("   - 尝试不同的测试文件")

def main():
    """主函数"""
    print("颜色映射修复验证工具")
    print("="*40)
    
    # 检查可执行文件
    if not check_executable():
        print("\n❌ 无法找到可执行文件，请先编译项目")
        return
    
    # 检查测试文件
    print("\n📁 检查测试文件...")
    test_files = check_test_files()
    
    if not test_files:
        print("\n⚠️  没有找到预生成的测试文件")
        print("   可以使用程序内的'生成测试数据'功能")
    
    # 打印测试说明
    print_test_instructions()
    
    # 询问是否启动程序
    print("\n" + "="*60)
    response = input("是否现在启动程序进行测试？(y/n): ").lower().strip()
    
    if response in ['y', 'yes', '是']:
        print("\n🚀 启动程序...")
        try:
            # 启动程序
            subprocess.Popen(["release/simple_demo.exe"], 
                           cwd=os.getcwd(),
                           creationflags=subprocess.CREATE_NEW_CONSOLE if os.name == 'nt' else 0)
            print("✅ 程序已启动，请按照上述说明进行测试")
        except Exception as e:
            print(f"❌ 启动程序失败: {e}")
    else:
        print("📝 请手动启动程序并按照上述说明进行测试")

if __name__ == "__main__":
    main()
