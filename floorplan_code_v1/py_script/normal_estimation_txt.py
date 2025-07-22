import open3d as o3d
import numpy as np


def compute_and_save_normals(input_ply, output_ply, k_neighbors=30):

    data = np.loadtxt(input_ply)
    points = data[:, :3]  # 前3列是XYZ坐标
    pcd = o3d.geometry.PointCloud()
    pcd.points = o3d.utility.Vector3dVector(points)

    if not pcd.has_points():
        raise ValueError("无法读取点云数据或文件为空")

    print(f"已加载点云，包含 {len(pcd.points)} 个点")

    # 2. 计算法向量
    print("正在计算法向量...")
    pcd.estimate_normals(
        search_param=o3d.geometry.KDTreeSearchParamKNN(knn=k_neighbors))

    # 统一法向量方向(可选)
    pcd.orient_normals_towards_camera_location(
        camera_location=np.array([0, 0, 0]))  # 朝向原点

    # 3. 检查并保留原始颜色数据(如果存在)
    has_colors = pcd.has_colors()

    # 4. 保存带法向量的PLY文件
    print("正在保存结果...")

    normals = np.asarray(pcd.normals)
    data = np.column_stack((points, normals))
    header = "x y z nx ny nz"
    fmt = ['%.6f', '%.6f', '%.6f', '%.6f', '%.6f', '%.6f']
    np.savetxt(output_ply, data, fmt=fmt, comments='')

    print(f"结果已保存到 {output_ply}")

    # 返回点云对象供进一步使用
    return pcd


# 使用示例
if __name__ == "__main__":
    input_file = "C:/SLAM/floorplan_code_v1/data/wall.txt"  # 输入PLY文件
    output_file = "C:/SLAM/floorplan_code_v1/data/wall_normal.txt"  # 输出PLY文件

    # 计算法向量并保存
    point_cloud = compute_and_save_normals(
        input_file,
        output_file,
        k_neighbors=20)

    # 可选: 可视化结果
    # o3d.visualization.draw_geometries([point_cloud])