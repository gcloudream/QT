import open3d as o3d
import numpy as np
# 读取PCD文件
# pcd = o3d.io.read_point_cloud("C:/SLAM/floorplan_code_v1/data/points.pcd")
pcd = o3d.io.read_point_cloud("C:/Users/3DSMART/Desktop/Resources/2.pcd")

down_pcd = pcd.voxel_down_sample(voxel_size=0.02)

# 获取点和颜色数据
points = np.asarray(down_pcd.points)
colors = np.asarray(down_pcd.colors) * 255 if down_pcd.has_colors() else None

# 保存为TXT
if colors is not None:
    data = np.hstack((points, colors))
    print('Saving to xyzrbg.txt')
    np.savetxt("C:/SLAM/floorplan_code_v1/data/xyzrgb.txt", data, comments='', fmt='%.6f %.6f %.6f %d %d %d')
else:
    np.savetxt("C:/SLAM/floorplan_code_v1/data/xyzrgb.txt", points, comments='', fmt='%.6f %.6f %.6f %d %d %d')