#!/usr/bin/env python3
"""
点云测试文件生成器
生成多种格式的建筑物点云文件用于测试渲染系统
"""

import numpy as np
import os
import random
from typing import List, Tuple
import struct

class PointCloudGenerator:
    def __init__(self, output_dir: str = "C:/Users/18438/Desktop/Resources"):
        self.output_dir = output_dir
        os.makedirs(output_dir, exist_ok=True)
        
    def add_noise(self, points: np.ndarray, noise_level: float = 0.01) -> np.ndarray:
        """添加适量噪声使点云更真实"""
        noise = np.random.normal(0, noise_level, points.shape)
        return points + noise
    
    def generate_simple_house(self, num_points: int) -> np.ndarray:
        """生成简单房屋点云"""
        points = []
        
        # 房屋尺寸
        width, length, height = 8.0, 10.0, 3.0
        wall_thickness = 0.2
        
        # 地面 (20%)
        ground_points = int(num_points * 0.2)
        for _ in range(ground_points):
            x = np.random.uniform(-width/2, width/2)
            y = np.random.uniform(-length/2, length/2)
            z = np.random.uniform(0, 0.1)
            points.append([x, y, z])
        
        # 墙面 (50%)
        wall_points = int(num_points * 0.5)
        for _ in range(wall_points):
            wall = random.choice(['front', 'back', 'left', 'right'])
            if wall == 'front':
                x = np.random.uniform(-width/2, width/2)
                y = length/2
                z = np.random.uniform(0, height)
            elif wall == 'back':
                x = np.random.uniform(-width/2, width/2)
                y = -length/2
                z = np.random.uniform(0, height)
            elif wall == 'left':
                x = -width/2
                y = np.random.uniform(-length/2, length/2)
                z = np.random.uniform(0, height)
            else:  # right
                x = width/2
                y = np.random.uniform(-length/2, length/2)
                z = np.random.uniform(0, height)
            points.append([x, y, z])
        
        # 屋顶 (25%)
        roof_points = int(num_points * 0.25)
        for _ in range(roof_points):
            x = np.random.uniform(-width/2, width/2)
            y = np.random.uniform(-length/2, length/2)
            z = height + np.random.uniform(0, 0.5)
            points.append([x, y, z])
        
        # 填充剩余点
        remaining = num_points - len(points)
        for _ in range(remaining):
            x = np.random.uniform(-width/2-1, width/2+1)
            y = np.random.uniform(-length/2-1, length/2+1)
            z = np.random.uniform(0, height+1)
            points.append([x, y, z])
        
        points = np.array(points)
        return self.add_noise(points, 0.02)
    
    def generate_apartment_building(self, num_points: int) -> np.ndarray:
        """生成多层公寓楼点云"""
        points = []
        
        # 建筑尺寸
        width, length = 12.0, 15.0
        floors = 4
        floor_height = 3.0
        total_height = floors * floor_height
        
        # 地面 (10%)
        ground_points = int(num_points * 0.1)
        for _ in range(ground_points):
            x = np.random.uniform(-width/2-2, width/2+2)
            y = np.random.uniform(-length/2-2, length/2+2)
            z = np.random.uniform(0, 0.1)
            points.append([x, y, z])
        
        # 每层楼的墙面和内部结构 (70%)
        floor_points = int(num_points * 0.7)
        for _ in range(floor_points):
            floor = random.randint(0, floors-1)
            z_base = floor * floor_height
            
            # 外墙
            if random.random() < 0.6:  # 60%是外墙
                wall = random.choice(['front', 'back', 'left', 'right'])
                if wall == 'front':
                    x = np.random.uniform(-width/2, width/2)
                    y = length/2
                elif wall == 'back':
                    x = np.random.uniform(-width/2, width/2)
                    y = -length/2
                elif wall == 'left':
                    x = -width/2
                    y = np.random.uniform(-length/2, length/2)
                else:  # right
                    x = width/2
                    y = np.random.uniform(-length/2, length/2)
                z = z_base + np.random.uniform(0, floor_height)
            else:  # 内部结构
                x = np.random.uniform(-width/2, width/2)
                y = np.random.uniform(-length/2, length/2)
                z = z_base + np.random.uniform(0, floor_height)
            
            points.append([x, y, z])
        
        # 屋顶 (15%)
        roof_points = int(num_points * 0.15)
        for _ in range(roof_points):
            x = np.random.uniform(-width/2, width/2)
            y = np.random.uniform(-length/2, length/2)
            z = total_height + np.random.uniform(0, 0.5)
            points.append([x, y, z])
        
        # 填充剩余点
        remaining = num_points - len(points)
        for _ in range(remaining):
            x = np.random.uniform(-width/2-3, width/2+3)
            y = np.random.uniform(-length/2-3, length/2+3)
            z = np.random.uniform(0, total_height+2)
            points.append([x, y, z])
        
        points = np.array(points)
        return self.add_noise(points, 0.03)
    
    def generate_complex_building(self, num_points: int) -> np.ndarray:
        """生成复杂建筑（带阳台、楼梯）"""
        points = []
        
        # 主建筑
        main_width, main_length = 15.0, 20.0
        floors = 5
        floor_height = 3.2
        total_height = floors * floor_height
        
        # 主建筑结构 (60%)
        main_points = int(num_points * 0.6)
        for _ in range(main_points):
            floor = random.randint(0, floors-1)
            z_base = floor * floor_height
            
            x = np.random.uniform(-main_width/2, main_width/2)
            y = np.random.uniform(-main_length/2, main_length/2)
            z = z_base + np.random.uniform(0, floor_height)
            points.append([x, y, z])
        
        # 阳台结构 (20%)
        balcony_points = int(num_points * 0.2)
        for _ in range(balcony_points):
            floor = random.randint(1, floors-1)  # 阳台从2楼开始
            z_base = floor * floor_height
            
            # 阳台位置（建筑前方）
            x = np.random.uniform(-main_width/2+2, main_width/2-2)
            y = main_length/2 + np.random.uniform(0, 2.0)  # 向外延伸
            z = z_base + np.random.uniform(0, floor_height)
            points.append([x, y, z])
        
        # 楼梯结构 (15%)
        stair_points = int(num_points * 0.15)
        stair_width = 2.0
        for _ in range(stair_points):
            # 楼梯位置（建筑侧面）
            x = main_width/2 + np.random.uniform(0, stair_width)
            y = np.random.uniform(-main_length/4, main_length/4)
            z = np.random.uniform(0, total_height)
            points.append([x, y, z])
        
        # 填充剩余点
        remaining = num_points - len(points)
        for _ in range(remaining):
            x = np.random.uniform(-main_width/2-5, main_width/2+5)
            y = np.random.uniform(-main_length/2-5, main_length/2+5)
            z = np.random.uniform(0, total_height+3)
            points.append([x, y, z])
        
        points = np.array(points)
        return self.add_noise(points, 0.04)

    def generate_building_cluster(self, num_points: int) -> np.ndarray:
        """生成建筑群点云"""
        points = []

        # 生成3-5个建筑物
        num_buildings = random.randint(3, 5)
        points_per_building = num_points // num_buildings

        building_positions = [
            (-20, -15), (20, -15), (-20, 15), (20, 15), (0, 0)
        ][:num_buildings]

        for i, (offset_x, offset_y) in enumerate(building_positions):
            building_points = points_per_building
            if i == num_buildings - 1:  # 最后一个建筑获得剩余点数
                building_points = num_points - len(points)

            # 随机选择建筑类型
            building_type = random.choice(['simple', 'apartment', 'complex'])

            if building_type == 'simple':
                building_cloud = self.generate_simple_house(building_points)
            elif building_type == 'apartment':
                building_cloud = self.generate_apartment_building(building_points)
            else:
                building_cloud = self.generate_complex_building(building_points)

            # 应用偏移
            building_cloud[:, 0] += offset_x
            building_cloud[:, 1] += offset_y

            points.extend(building_cloud.tolist())

        return np.array(points)

    def generate_industrial_building(self, num_points: int) -> np.ndarray:
        """生成工业建筑（厂房、仓库）"""
        points = []

        # 大型厂房尺寸
        width, length, height = 30.0, 50.0, 8.0

        # 地面 (15%)
        ground_points = int(num_points * 0.15)
        for _ in range(ground_points):
            x = np.random.uniform(-width/2-5, width/2+5)
            y = np.random.uniform(-length/2-5, length/2+5)
            z = np.random.uniform(0, 0.2)
            points.append([x, y, z])

        # 外墙 (30%)
        wall_points = int(num_points * 0.3)
        for _ in range(wall_points):
            wall = random.choice(['front', 'back', 'left', 'right'])
            if wall == 'front':
                x = np.random.uniform(-width/2, width/2)
                y = length/2
            elif wall == 'back':
                x = np.random.uniform(-width/2, width/2)
                y = -length/2
            elif wall == 'left':
                x = -width/2
                y = np.random.uniform(-length/2, length/2)
            else:  # right
                x = width/2
                y = np.random.uniform(-length/2, length/2)
            z = np.random.uniform(0, height)
            points.append([x, y, z])

        # 屋顶结构 (25%)
        roof_points = int(num_points * 0.25)
        for _ in range(roof_points):
            x = np.random.uniform(-width/2, width/2)
            y = np.random.uniform(-length/2, length/2)
            z = height + np.random.uniform(0, 1.0)
            points.append([x, y, z])

        # 内部设备和结构 (25%)
        equipment_points = int(num_points * 0.25)
        for _ in range(equipment_points):
            x = np.random.uniform(-width/2+2, width/2-2)
            y = np.random.uniform(-length/2+2, length/2-2)
            z = np.random.uniform(0.5, height-0.5)
            points.append([x, y, z])

        # 填充剩余点
        remaining = num_points - len(points)
        for _ in range(remaining):
            x = np.random.uniform(-width/2-8, width/2+8)
            y = np.random.uniform(-length/2-8, length/2+8)
            z = np.random.uniform(0, height+2)
            points.append([x, y, z])

        points = np.array(points)
        return self.add_noise(points, 0.05)

    def generate_attributes(self, points: np.ndarray) -> dict:
        """为点云生成属性信息"""
        num_points = len(points)

        # 基于高度的强度值
        z_values = points[:, 2]
        z_min, z_max = z_values.min(), z_values.max()
        z_range = z_max - z_min if z_max > z_min else 1.0

        intensity = ((z_values - z_min) / z_range * 65535).astype(np.int32)

        # 分类：地面(2)、建筑物(6)、其他(1)
        classification = np.where(z_values < 0.5, 2,  # 地面
                                np.where(z_values > z_max * 0.8, 1, 6))  # 高处为其他，中间为建筑

        # RGB颜色（基于高度和位置）
        red = ((z_values / z_max) * 255).astype(np.int32)
        green = (255 - red).astype(np.int32)
        blue = ((np.abs(points[:, 0]) + np.abs(points[:, 1])) /
                (np.abs(points[:, 0]).max() + np.abs(points[:, 1]).max()) * 255).astype(np.int32)

        return {
            'intensity': intensity,
            'classification': classification,
            'red': red,
            'green': green,
            'blue': blue
        }

    def save_pcd(self, points: np.ndarray, filename: str):
        """保存为PCD格式"""
        attributes = self.generate_attributes(points)

        filepath = os.path.join(self.output_dir, filename)
        with open(filepath, 'w') as f:
            # PCD头部
            f.write("# .PCD v0.7 - Point Cloud Data file format\n")
            f.write("VERSION 0.7\n")
            f.write("FIELDS x y z intensity classification red green blue\n")
            f.write("SIZE 4 4 4 4 4 4 4 4\n")
            f.write("TYPE F F F I I I I I\n")
            f.write("COUNT 1 1 1 1 1 1 1 1\n")
            f.write(f"WIDTH {len(points)}\n")
            f.write("HEIGHT 1\n")
            f.write("VIEWPOINT 0 0 0 1 0 0 0\n")
            f.write(f"POINTS {len(points)}\n")
            f.write("DATA ascii\n")

            # 点数据
            for i, (x, y, z) in enumerate(points):
                f.write(f"{x:.6f} {y:.6f} {z:.6f} {attributes['intensity'][i]} "
                       f"{attributes['classification'][i]} {attributes['red'][i]} "
                       f"{attributes['green'][i]} {attributes['blue'][i]}\n")

        print(f"✅ Generated PCD file: {filename} ({len(points)} points)")

    def save_ply(self, points: np.ndarray, filename: str):
        """保存为PLY格式"""
        attributes = self.generate_attributes(points)

        filepath = os.path.join(self.output_dir, filename)
        with open(filepath, 'w') as f:
            # PLY头部
            f.write("ply\n")
            f.write("format ascii 1.0\n")
            f.write(f"element vertex {len(points)}\n")
            f.write("property float x\n")
            f.write("property float y\n")
            f.write("property float z\n")
            f.write("property int intensity\n")
            f.write("property uchar red\n")
            f.write("property uchar green\n")
            f.write("property uchar blue\n")
            f.write("end_header\n")

            # 点数据
            for i, (x, y, z) in enumerate(points):
                f.write(f"{x:.6f} {y:.6f} {z:.6f} {attributes['intensity'][i]} "
                       f"{attributes['red'][i]} {attributes['green'][i]} {attributes['blue'][i]}\n")

        print(f"✅ Generated PLY file: {filename} ({len(points)} points)")

    def save_xyz(self, points: np.ndarray, filename: str):
        """保存为XYZ格式"""
        attributes = self.generate_attributes(points)

        filepath = os.path.join(self.output_dir, filename)
        with open(filepath, 'w') as f:
            # XYZ格式：x y z intensity red green blue
            for i, (x, y, z) in enumerate(points):
                f.write(f"{x:.6f} {y:.6f} {z:.6f} {attributes['intensity'][i]} "
                       f"{attributes['red'][i]} {attributes['green'][i]} {attributes['blue'][i]}\n")

        print(f"✅ Generated XYZ file: {filename} ({len(points)} points)")

    def save_txt(self, points: np.ndarray, filename: str):
        """保存为TXT格式"""
        attributes = self.generate_attributes(points)

        filepath = os.path.join(self.output_dir, filename)
        with open(filepath, 'w') as f:
            # TXT格式头部注释
            f.write("# Point Cloud Data - TXT Format\n")
            f.write("# Format: X Y Z Intensity Classification Red Green Blue\n")
            f.write(f"# Points: {len(points)}\n")
            f.write("# Generated by Point Cloud Test Generator\n")

            # 点数据
            for i, (x, y, z) in enumerate(points):
                f.write(f"{x:.6f} {y:.6f} {z:.6f} {attributes['intensity'][i]} "
                       f"{attributes['classification'][i]} {attributes['red'][i]} "
                       f"{attributes['green'][i]} {attributes['blue'][i]}\n")

        print(f"✅ Generated TXT file: {filename} ({len(points)} points)")

    def generate_all_test_files(self):
        """生成所有测试文件"""
        print("🚀 开始生成点云测试文件...")
        print(f"📁 输出目录: {self.output_dir}")
        print("=" * 60)

        # 定义测试文件配置
        test_configs = [
            # 简单房屋
            ("simple_house_10k", "simple_house", 10000, "pcd"),
            ("simple_house_25k", "simple_house", 25000, "ply"),
            ("simple_house_50k", "simple_house", 50000, "xyz"),

            # 多层建筑
            ("apartment_building_50k", "apartment", 50000, "pcd"),
            ("apartment_building_100k", "apartment", 100000, "ply"),
            ("apartment_building_200k", "apartment", 200000, "txt"),

            # 复杂建筑
            ("complex_building_100k", "complex", 100000, "pcd"),
            ("complex_building_300k", "complex", 300000, "ply"),
            ("complex_building_500k", "complex", 500000, "xyz"),

            # 建筑群
            ("building_cluster_200k", "cluster", 200000, "pcd"),
            ("building_cluster_500k", "cluster", 500000, "ply"),
            ("building_cluster_1m", "cluster", 1000000, "txt"),

            # 工业建筑
            ("industrial_building_150k", "industrial", 150000, "pcd"),
            ("industrial_building_400k", "industrial", 400000, "ply"),
            ("industrial_building_800k", "industrial", 800000, "xyz"),
            ("industrial_building_1_5m", "industrial", 1500000, "txt"),

            # 大型测试文件
            ("large_complex_2m", "complex", 2000000, "pcd"),
            ("mega_cluster_3m", "cluster", 3000000, "txt"),
        ]

        total_files = len(test_configs)
        generated_files = 0

        for config in test_configs:
            filename_base, building_type, num_points, file_format = config
            filename = f"{filename_base}.{file_format}"

            try:
                print(f"📦 生成 {filename} ({num_points:,} 点, {building_type} 类型)...")

                # 生成点云数据
                if building_type == "simple_house":
                    points = self.generate_simple_house(num_points)
                elif building_type == "apartment":
                    points = self.generate_apartment_building(num_points)
                elif building_type == "complex":
                    points = self.generate_complex_building(num_points)
                elif building_type == "cluster":
                    points = self.generate_building_cluster(num_points)
                elif building_type == "industrial":
                    points = self.generate_industrial_building(num_points)
                else:
                    raise ValueError(f"Unknown building type: {building_type}")

                # 保存文件
                if file_format == "pcd":
                    self.save_pcd(points, filename)
                elif file_format == "ply":
                    self.save_ply(points, filename)
                elif file_format == "xyz":
                    self.save_xyz(points, filename)
                elif file_format == "txt":
                    self.save_txt(points, filename)
                else:
                    raise ValueError(f"Unknown file format: {file_format}")

                generated_files += 1

            except Exception as e:
                print(f"❌ 生成 {filename} 失败: {str(e)}")

        print("=" * 60)
        print(f"🎉 文件生成完成！")
        print(f"📊 成功生成: {generated_files}/{total_files} 个文件")
        print(f"📁 文件位置: {self.output_dir}")

        # 显示文件统计
        self.show_file_statistics()

    def show_file_statistics(self):
        """显示生成的文件统计信息"""
        print("\n📈 文件统计信息:")
        print("-" * 40)

        formats = {"pcd": 0, "ply": 0, "xyz": 0, "txt": 0}
        sizes = {"small": 0, "medium": 0, "large": 0, "mega": 0}

        for filename in os.listdir(self.output_dir):
            if any(filename.endswith(f".{fmt}") for fmt in formats.keys()):
                # 统计格式
                for fmt in formats.keys():
                    if filename.endswith(f".{fmt}"):
                        formats[fmt] += 1
                        break

                # 统计大小类别
                if "10k" in filename or "25k" in filename or "50k" in filename:
                    sizes["small"] += 1
                elif "100k" in filename or "150k" in filename or "200k" in filename or "300k" in filename:
                    sizes["medium"] += 1
                elif "500k" in filename or "800k" in filename or "1m" in filename or "1_5m" in filename:
                    sizes["large"] += 1
                else:
                    sizes["mega"] += 1

        print("按格式分类:")
        for fmt, count in formats.items():
            print(f"  {fmt.upper()}: {count} 个文件")

        print("\n按大小分类:")
        print(f"  小型 (1万-5万点): {sizes['small']} 个文件")
        print(f"  中型 (10万-50万点): {sizes['medium']} 个文件")
        print(f"  大型 (50万-200万点): {sizes['large']} 个文件")
        print(f"  超大型 (200万+点): {sizes['mega']} 个文件")


def main():
    """主函数"""
    print("🏗️  建筑物点云测试文件生成器")
    print("=" * 60)

    # 创建生成器
    generator = PointCloudGenerator()

    # 生成所有测试文件
    generator.generate_all_test_files()

    print("\n🎯 测试建议:")
    print("1. 先测试小型文件验证基本功能")
    print("2. 然后测试中型文件验证性能")
    print("3. 最后测试大型文件验证智能采样")
    print("4. 验证不同格式的兼容性")
    print("\n✨ 生成完成！可以开始测试点云渲染系统了。")


if __name__ == "__main__":
    main()
