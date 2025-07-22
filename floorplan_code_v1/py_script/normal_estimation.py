import numpy as np
from scipy.spatial import KDTree
from sklearn.decomposition import PCA
import open3d as o3d


def load_xyzc_point_cloud(filename):
    """
    加载包含XYZC数据的TXT点云文件
    文件格式：每行 x y z c (空格或逗号分隔)
    """
    data = np.loadtxt(filename)
    points = data[:, :3]  # 前3列是XYZ坐标
    c_values = data[:, 3]  # 第4列是C值(分类/颜色)
    return points, c_values


def save_with_normals(points, c_values, normals, output_file):
    """
    保存带法向量和原始C值的点云
    :param points: XYZ坐标
    :param c_values: 原始C值
    :param normals: 法向量
    :param output_file: 输出文件名
    """
    data = np.column_stack((points, c_values, normals))
    header = "x y z c nx ny nz"
    fmt = ['%.6f', '%.6f', '%.6f', '%d', '%.6f', '%.6f', '%.6f']
    np.savetxt(output_file, data, fmt=fmt, comments='')


# 使用Open3D加速版本(推荐)
def compute_normals_open3d(points, k=10):
    pcd = o3d.geometry.PointCloud()
    pcd.points = o3d.utility.Vector3dVector(points)
    pcd.estimate_normals(search_param=o3d.geometry.KDTreeSearchParamKNN(knn=k))
    # 统一法向量方向(朝向原点)
    pcd.orient_normals_towards_camera_location()
    return np.asarray(pcd.normals)


def main():
    # 输入输出文件
    input_file = "C:/SLAM/floorplan_code_v1/data/xyzc.txt"
    output_file = 'C:/SLAM/floorplan_code_v1/data/xyzc_normal.txt'

    # 1. 加载点云(保留C值)
    points, c_values = load_xyzc_point_cloud(input_file)
    print(f"Loaded {len(points)} points")

    # 2. 计算法向量(选择其中一种方法)
    # normals = compute_normals(points, k=20)  # 纯Python方法
    normals = compute_normals_open3d(points, k=20)  # Open3D加速方法

    # 3. 保存结果(包含原始XYZC和法向量)
    save_with_normals(points, c_values, normals, output_file)
    print(f"Saved results to {output_file}")


if __name__ == "__main__":
    main()