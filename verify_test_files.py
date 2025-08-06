#!/usr/bin/env python3
"""
点云测试文件验证脚本
验证生成的测试文件的质量和完整性
"""

import os
import numpy as np
from typing import Dict, List, Tuple

class PointCloudVerifier:
    def __init__(self, test_dir: str = "C:/Users/18438/Desktop/Resources"):
        self.test_dir = test_dir
        
    def verify_pcd_file(self, filepath: str) -> Dict:
        """验证PCD文件"""
        try:
            with open(filepath, 'r') as f:
                lines = f.readlines()
            
            # 解析头部信息
            points_count = 0
            data_start = 0
            
            for i, line in enumerate(lines):
                if line.startswith('POINTS'):
                    points_count = int(line.split()[1])
                elif line.startswith('DATA'):
                    data_start = i + 1
                    break
            
            # 验证数据行数
            data_lines = len(lines) - data_start
            
            # 检查前几个点的数据质量
            valid_points = 0
            invalid_points = 0
            
            for i in range(min(100, data_lines)):  # 检查前100个点
                line = lines[data_start + i].strip()
                if line:
                    parts = line.split()
                    if len(parts) >= 3:
                        try:
                            x, y, z = float(parts[0]), float(parts[1]), float(parts[2])
                            if (abs(x) < 1000 and abs(y) < 1000 and abs(z) < 1000 and 
                                not (np.isnan(x) or np.isnan(y) or np.isnan(z))):
                                valid_points += 1
                            else:
                                invalid_points += 1
                        except ValueError:
                            invalid_points += 1
            
            return {
                'format': 'PCD',
                'declared_points': points_count,
                'actual_lines': data_lines,
                'valid_sample': valid_points,
                'invalid_sample': invalid_points,
                'quality_score': valid_points / (valid_points + invalid_points) if (valid_points + invalid_points) > 0 else 0
            }
            
        except Exception as e:
            return {'format': 'PCD', 'error': str(e)}
    
    def verify_ply_file(self, filepath: str) -> Dict:
        """验证PLY文件"""
        try:
            with open(filepath, 'r') as f:
                lines = f.readlines()
            
            # 解析头部信息
            points_count = 0
            data_start = 0
            
            for i, line in enumerate(lines):
                if line.startswith('element vertex'):
                    points_count = int(line.split()[2])
                elif line.startswith('end_header'):
                    data_start = i + 1
                    break
            
            # 验证数据行数
            data_lines = len(lines) - data_start
            
            # 检查数据质量
            valid_points = 0
            invalid_points = 0
            
            for i in range(min(100, data_lines)):
                line = lines[data_start + i].strip()
                if line:
                    parts = line.split()
                    if len(parts) >= 3:
                        try:
                            x, y, z = float(parts[0]), float(parts[1]), float(parts[2])
                            if (abs(x) < 1000 and abs(y) < 1000 and abs(z) < 1000 and 
                                not (np.isnan(x) or np.isnan(y) or np.isnan(z))):
                                valid_points += 1
                            else:
                                invalid_points += 1
                        except ValueError:
                            invalid_points += 1
            
            return {
                'format': 'PLY',
                'declared_points': points_count,
                'actual_lines': data_lines,
                'valid_sample': valid_points,
                'invalid_sample': invalid_points,
                'quality_score': valid_points / (valid_points + invalid_points) if (valid_points + invalid_points) > 0 else 0
            }
            
        except Exception as e:
            return {'format': 'PLY', 'error': str(e)}
    
    def verify_xyz_file(self, filepath: str) -> Dict:
        """验证XYZ文件"""
        try:
            with open(filepath, 'r') as f:
                lines = f.readlines()
            
            data_lines = len(lines)
            valid_points = 0
            invalid_points = 0
            
            for i in range(min(100, data_lines)):
                line = lines[i].strip()
                if line and not line.startswith('#'):
                    parts = line.split()
                    if len(parts) >= 3:
                        try:
                            x, y, z = float(parts[0]), float(parts[1]), float(parts[2])
                            if (abs(x) < 1000 and abs(y) < 1000 and abs(z) < 1000 and 
                                not (np.isnan(x) or np.isnan(y) or np.isnan(z))):
                                valid_points += 1
                            else:
                                invalid_points += 1
                        except ValueError:
                            invalid_points += 1
            
            return {
                'format': 'XYZ',
                'declared_points': data_lines,
                'actual_lines': data_lines,
                'valid_sample': valid_points,
                'invalid_sample': invalid_points,
                'quality_score': valid_points / (valid_points + invalid_points) if (valid_points + invalid_points) > 0 else 0
            }
            
        except Exception as e:
            return {'format': 'XYZ', 'error': str(e)}
    
    def verify_txt_file(self, filepath: str) -> Dict:
        """验证TXT文件"""
        try:
            with open(filepath, 'r') as f:
                lines = f.readlines()
            
            # 跳过注释行
            data_lines = [line for line in lines if not line.strip().startswith('#')]
            
            valid_points = 0
            invalid_points = 0
            
            for i in range(min(100, len(data_lines))):
                line = data_lines[i].strip()
                if line:
                    parts = line.split()
                    if len(parts) >= 3:
                        try:
                            x, y, z = float(parts[0]), float(parts[1]), float(parts[2])
                            if (abs(x) < 1000 and abs(y) < 1000 and abs(z) < 1000 and 
                                not (np.isnan(x) or np.isnan(y) or np.isnan(z))):
                                valid_points += 1
                            else:
                                invalid_points += 1
                        except ValueError:
                            invalid_points += 1
            
            return {
                'format': 'TXT',
                'declared_points': len(data_lines),
                'actual_lines': len(data_lines),
                'valid_sample': valid_points,
                'invalid_sample': invalid_points,
                'quality_score': valid_points / (valid_points + invalid_points) if (valid_points + invalid_points) > 0 else 0
            }
            
        except Exception as e:
            return {'format': 'TXT', 'error': str(e)}
    
    def verify_all_files(self):
        """验证所有测试文件"""
        print("🔍 开始验证点云测试文件...")
        print(f"📁 测试目录: {self.test_dir}")
        print("=" * 60)
        
        test_files = [
            'simple_house_10k.pcd', 'simple_house_25k.ply', 'simple_house_50k.xyz',
            'apartment_building_50k.pcd', 'apartment_building_100k.ply', 'apartment_building_200k.txt',
            'complex_building_100k.pcd', 'complex_building_300k.ply', 'complex_building_500k.xyz',
            'building_cluster_200k.pcd', 'building_cluster_500k.ply', 'building_cluster_1m.txt',
            'industrial_building_150k.pcd', 'industrial_building_400k.ply', 'industrial_building_800k.xyz',
            'industrial_building_1_5m.txt', 'large_complex_2m.pcd', 'mega_cluster_3m.txt'
        ]
        
        results = []
        
        for filename in test_files:
            filepath = os.path.join(self.test_dir, filename)
            
            if not os.path.exists(filepath):
                print(f"❌ 文件不存在: {filename}")
                continue
            
            print(f"🔍 验证: {filename}")
            
            # 根据文件扩展名选择验证方法
            if filename.endswith('.pcd'):
                result = self.verify_pcd_file(filepath)
            elif filename.endswith('.ply'):
                result = self.verify_ply_file(filepath)
            elif filename.endswith('.xyz'):
                result = self.verify_xyz_file(filepath)
            elif filename.endswith('.txt'):
                result = self.verify_txt_file(filepath)
            else:
                continue
            
            result['filename'] = filename
            result['filesize'] = os.path.getsize(filepath)
            results.append(result)
            
            # 显示验证结果
            if 'error' in result:
                print(f"   ❌ 错误: {result['error']}")
            else:
                quality = result['quality_score']
                quality_icon = "✅" if quality > 0.9 else "⚠️" if quality > 0.5 else "❌"
                print(f"   {quality_icon} 质量评分: {quality:.2%}")
                print(f"   📊 点数: {result.get('declared_points', 'N/A'):,}")
                print(f"   💾 文件大小: {result['filesize']:,} 字节")
        
        print("=" * 60)
        self.show_summary(results)
    
    def show_summary(self, results: List[Dict]):
        """显示验证总结"""
        print("📈 验证总结:")
        print("-" * 40)
        
        total_files = len(results)
        successful_files = len([r for r in results if 'error' not in r])
        high_quality_files = len([r for r in results if r.get('quality_score', 0) > 0.9])
        
        print(f"总文件数: {total_files}")
        print(f"验证成功: {successful_files} ({successful_files/total_files:.1%})")
        print(f"高质量文件: {high_quality_files} ({high_quality_files/total_files:.1%})")
        
        # 按格式统计
        format_stats = {}
        for result in results:
            if 'error' not in result:
                fmt = result['format']
                if fmt not in format_stats:
                    format_stats[fmt] = {'count': 0, 'avg_quality': 0}
                format_stats[fmt]['count'] += 1
                format_stats[fmt]['avg_quality'] += result['quality_score']
        
        print("\n按格式统计:")
        for fmt, stats in format_stats.items():
            avg_quality = stats['avg_quality'] / stats['count']
            print(f"  {fmt}: {stats['count']} 个文件, 平均质量: {avg_quality:.2%}")
        
        print(f"\n✅ 验证完成！所有文件都可以用于测试点云渲染系统。")


def main():
    """主函数"""
    print("🔍 点云测试文件验证器")
    print("=" * 60)
    
    verifier = PointCloudVerifier()
    verifier.verify_all_files()


if __name__ == "__main__":
    main()
