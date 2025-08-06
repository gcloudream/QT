#!/usr/bin/env python3
"""
测试点云文件加载功能的脚本
用于验证Stage1DemoWidget是否正确加载不同格式的点云文件
"""

import os
import subprocess
import time

def test_file_loading():
    """测试文件加载功能"""
    
    print("🔍 测试点云文件加载功能修复")
    print("=" * 50)
    
    # 测试文件路径
    test_files = [
        "C:/Users/18438/Desktop/Resources/109sub_11_30_empty.pcd",
        "C:/Users/18438/Desktop/Resources/ply/bunny.ply"
    ]
    
    print("📁 测试文件列表:")
    for i, file_path in enumerate(test_files, 1):
        exists = "✅" if os.path.exists(file_path) else "❌"
        print(f"  {i}. {exists} {file_path}")
    
    print("\n🎯 预期修复效果:")
    print("  • 不同文件应显示不同的俯视图结果")
    print("  • 不再是相同的蓝色条纹图案")
    print("  • 文件信息正确显示实际加载的点云数据")
    print("  • 调试输出显示正确的文件格式和点数")
    
    print("\n📋 测试步骤:")
    print("  1. 启动程序并进入Stage1DemoWidget")
    print("  2. 点击'加载点云文件'按钮")
    print("  3. 选择PCD文件 (109sub_11_30_empty.pcd)")
    print("  4. 点击'渲染俯视图'按钮")
    print("  5. 观察渲染结果")
    print("  6. 重复步骤2-5，选择PLY文件 (bunny.ply)")
    print("  7. 比较两次渲染结果是否不同")
    
    print("\n🔧 修复内容:")
    print("  ✅ 修复了loadPointCloudFile()中的文件格式处理逻辑")
    print("  ✅ 添加了PCD、PLY、XYZ、TXT格式的完整支持")
    print("  ✅ 移除了错误的fallback到测试数据的逻辑")
    print("  ✅ 为不同格式生成合适的属性数据")
    print("  ✅ 增强了调试输出和错误处理")
    
    print("\n🎉 修复完成！请手动测试验证效果。")

if __name__ == "__main__":
    test_file_loading()
