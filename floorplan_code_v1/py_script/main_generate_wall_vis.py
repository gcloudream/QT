import copy
import glob
import pyvista as pv
import numpy as np
from matplotlib import pyplot as plt
import copy
import glob
import pyvista as pv
import numpy as np
from matplotlib import pyplot as plt
import os
import math


# 计算两点之间的距离
def distance(p1, p2):
    return np.sqrt((p2[0] - p1[0]) ** 2 + (p2[1] - p1[1]) ** 2)


# 计算线段的斜率
def calculate_slope(p1, p2):
    if p2[0] - p1[0] == 0:
        return np.inf  # 竖直线段
    return (p2[1] - p1[1]) / (p2[0] - p1[0])


# 计算向量点积
def dot_product(v1, v2):
    return np.dot(v1, v2)


# 投影点计算
def project_point_on_line(point, line_start, line_end):
    line_vec = np.array([line_end[0] - line_start[0], line_end[1] - line_start[1]])
    point_vec = np.array([point[0] - line_start[0], point[1] - line_start[1]])

    # 投影公式
    proj_length = dot_product(point_vec, line_vec) / dot_product(line_vec, line_vec)
    proj_point = np.array([line_start[0], line_start[1]]) + proj_length * line_vec
    return proj_point


# 确保投影点在给定线段的范围内
def clamp_projection(proj_point, line_start, line_end):
    proj_x = np.clip(proj_point[0], min(line_start[0], line_end[0]), max(line_start[0], line_end[0]))
    proj_y = np.clip(proj_point[1], min(line_start[1], line_end[1]), max(line_start[1], line_end[1]))
    return np.array([proj_x, proj_y])


# 计算待投影线段的斜率
def get_line_slope(line):
    return calculate_slope(line[0], line[1])


# 计算待投影线段的端点
def project_line_to_closest_segment(segments, line):
    # 计算待投影线段的斜率
    line_slope = get_line_slope(line)

    # 找到斜率最接近且距离最近的线段
    closest_segment = None
    closest_distance = float('inf')
    selected_idx = -1
    for i, segment in enumerate(segments):
        segment_slope = get_line_slope(segment)

        # 计算斜率差异，选择斜率最接近的线段
        slope_diff = abs(line_slope - segment_slope)
        slope_diff2 = abs(1/(line_slope+1e-8) - 1/ (segment_slope+1e-8))
        # 计算投影结果和当前线段之间的距离
        proj_start = project_point_on_line(line[0], segment[0], segment[1])
        proj_end = project_point_on_line(line[1], segment[0], segment[1])

        proj_start_clamped = clamp_projection(proj_start, segment[0], segment[1])
        proj_end_clamped = clamp_projection(proj_end, segment[0], segment[1])

        dist_start = distance(proj_start_clamped, line[0])
        dist_end = distance(proj_end_clamped, line[1])
        avg_distance = (dist_start + dist_end) / 2  # 计算平均距离

        # 选择斜率最接近且距离最短的线段
        if min(slope_diff, slope_diff2) < 1 and avg_distance < closest_distance:  # 设定一个阈值来筛选斜率差异
            closest_distance = avg_distance
            closest_segment = segment
            selected_idx = i
    # 如果找到了最近的线段，将待投影线段投影到该线段上
    if closest_segment is not None and closest_distance<0.5:
        print(closest_distance)
        proj_start = project_point_on_line(line[0], closest_segment[0], closest_segment[1])
        proj_end = project_point_on_line(line[1], closest_segment[0], closest_segment[1])

        proj_start_clamped = clamp_projection(proj_start, closest_segment[0], closest_segment[1])
        proj_end_clamped = clamp_projection(proj_end, closest_segment[0], closest_segment[1])

        return (proj_start_clamped, proj_end_clamped), selected_idx

    return None, None

def m_distance(point, m, b):
    """计算点到直线的距离"""
    x, y = point
    return abs(-m * x + 1 * y - b) / np.sqrt(m ** 2 + 1)

def ransac_line_fitting(points, iterations=3000, threshold=0.03):
    best_inliers = []
    best_m = None
    best_b = None

    for _ in range(iterations):
        # 随机选择两个点
        sample = points[np.random.choice(points.shape[0], 2, replace=False)]
        (x1, y1), (x2, y2) = sample

        # 计算斜率和截距
        if x1 == x2:  # 处理垂直线的情况
            continue
        if y1 == y2:
            continue

        m = (y2 - y1) / (x2 - x1)
        b = y1 - m * x1
        # if abs(min(m, 1/(m+1e-9))) > 1 / 20:  # 约束斜率
        #     continue
        if m==0:
            continue
        inliers = [point for point in points if m_distance(point, m, b) < threshold]
        if len(inliers) > len(best_inliers):
            best_inliers = inliers
            best_m = m
            best_b = b

    return best_m, best_b, np.array(best_inliers)


class LineSegment:
    def __init__(self, x1, y1, x2, y2):
        self.p1 = np.array([x1, y1])  # 第一个端点
        self.p2 = np.array([x2, y2])  # 第二个端点

    # 判断点P是否在两端点A和B之间
    def is_point_on_segment(self, P):
        # 判断P是否在线段AB上，要求P在A到B的范围内
        min_x, max_x = min(self.p1[0], self.p2[0]), max(self.p1[0], self.p2[0])
        min_y, max_y = min(self.p1[1], self.p2[1]), max(self.p1[1], self.p2[1])
        return min_x <= P[0] <= max_x and min_y <= P[1] <= max_y

    # 计算向量叉积，判断两线段是否相交
    def cross_product(self, A, B, C):
        return (B[0] - A[0]) * (C[1] - A[1]) - (B[1] - A[1]) * (C[0] - A[0])

    # 判断两线段是否相交
    def is_intersect(self, other):
        # 判断长线段self与短线段other是否相交
        # 使用叉积算法判断两线段是否相交
        # 假设 self.p1, self.p2 是长线段，other.p1, other.p2 是短线段

        p1, p2 = self.p1, self.p2
        p3, p4 = other.p1, other.p2

        # 计算长线段的叉积
        d1 = self.cross_product(p3, p4, p1)
        d2 = self.cross_product(p3, p4, p2)
        d3 = self.cross_product(p1, p2, p3)
        d4 = self.cross_product(p1, p2, p4)

        # 检查两线段是否有交点
        if d1 * d2 < 0 and d3 * d4 < 0:
            return True
        # 特殊情况：共线且部分重叠
        if d1 == 0 and self.is_point_on_segment(p3):
            return True
        if d2 == 0 and self.is_point_on_segment(p4):
            return True
        if d3 == 0 and other.is_point_on_segment(p1):
            return True
        if d4 == 0 and other.is_point_on_segment(p2):
            return True

        return False


# 将长线段根据短线段的端点进行分割
def split_line_by_short_segment(long_line, short_line):
    points = []

    # 检查短线段的两个端点是否在长线段上
    if long_line.is_intersect(short_line):
        points.append(short_line.p1)
        points.append(short_line.p2)
    if len(points) > 0:
        # 按照 x 和 y 坐标进行排序
        points_sorted = sorted(points, key=lambda p: (p[0], p[1]))

    # 分割长线段
    split_segments = []
    prev_point = long_line.p1
    for p in points_sorted:
        split_segments.append(LineSegment(prev_point[0], prev_point[1], p[0], p[1]))
        prev_point = p

    split_segments.append(LineSegment(prev_point[0], prev_point[1], long_line.p2[0], long_line.p2[1]))

    return split_segments

def is_point_on_line_segment(P1, P2, P):
    """
    判断点 P 是否在由 P1 和 P2 定义的线段上
    P1, P2, P 是包含坐标元组的点 (x, y)
    """
    x1, y1 = P1
    x2, y2 = P2
    x, y = P

    # 计算叉积，判断共线
    cross_product = (x2 - x1) * (y - y1) - (y2 - y1) * (x - x1)

    # 如果叉积为 0，表示共线
    if cross_product != 0:
        return False

    # 检查点是否在范围内
    if min(x1, x2) <= x <= max(x1, x2) and min(y1, y2) <= y <= max(y1, y2):
        return True
    return False



def split_long_line(long_line, short_line):

    (x1, y1), (x2, y2) = long_line
    (a1, b1), (a2, b2) = short_line

    # 创建一个分割点列表，初始化为长线段的端点
    points = [(x1, y1), (x2, y2)]

    # 判断短线段的端点是否在长线段上

    points.append((a1, b1))

    points.append((a2, b2))

    # 去重并排序分割点
    points = sorted(set(points))

    # 将分割点组成新的线段
    segments = []
    for i in range(len(points) - 1):
        segments.append([points[i], points[i + 1]])

    return segments


def is_point_on_line(long_line, point, epsilon=1e-5):
    """
    判断一个点是否在线段上。
    long_line: 长线段的端点 [(x1, y1), (x2, y2)]
    point: 需要判断的点 (px, py)
    """
    (x1, y1), (x2, y2) = long_line
    (px, py) = point
    if (px==x1 and py==y1) or (px==x2 and py==y2):
        return True

    # 计算点到线段的距离，判断点是否在线段上
    line_vec = np.array([x2 - x1, y2 - y1])
    point_vec = np.array([px - x1, py - y1])
    line_len = np.linalg.norm(line_vec)

    if line_len < epsilon:
        return False  # 长线段太短，无法判断

    # 投影计算
    proj_len = np.dot(point_vec, line_vec) / line_len
    if proj_len < 0 or proj_len > line_len:
        return False  # 点不在该线段的延长线上

    # 计算点到线段的距离
    dist = np.abs(np.cross(line_vec, point_vec)) / line_len
    return dist < epsilon  # 如果距离小于容差，说明点在直线上


def split_wall(rt_walls, element, ele_type):
    p1 = (element[0], element[1])
    p2 = (element[3], element[4])
    z_min = element[2]
    z_max = element[5]
    for j, wall in enumerate(rt_walls):
        st_p = wall[0]
        ed_p = wall[1]
        long_line = [st_p, ed_p]
        if is_point_on_line(long_line, p1) == True and is_point_on_line(long_line, p2) == True:
            short_line = [p1, p2]
            split_segments = split_long_line(long_line, short_line)
            rt_walls.pop(j)
            for seg in split_segments:
                if (seg[0] == p1 and seg[1] == p2) or (seg[0] == p2 and seg[1] == p1):
                    cur_wall = (seg[0], seg[1], ele_type, (z_min, z_max))
                    rt_walls.append(cur_wall)
                else:
                    cur_wall = (seg[0], seg[1], 0)
                    rt_walls.append(cur_wall)
    return rt_walls

def get_data(in_path):
    with open(in_path, 'r') as f:
        data = f.readlines()
        data = [(x.strip('\n').split(' ')) for x in data]
        new_data = []
        for i in data:
            ps = []
            for j in i:
                if j != '':
                    ps.append(float(j))
            new_data.append(np.array(ps))
    return new_data


def save_obj(filepath, filename, mesh):
    vertices = mesh.points  # 获取顶点坐标 (N x 3 numpy array)
    faces = mesh.faces  # 获取面片数据 (包含面片顶点索引的 numpy array)
    with open(filepath, 'w') as f:
        f.write("mtllib " + filename +".mtl\n")
        for vertex in vertices:
            f.write(f"v {vertex[0]} {vertex[1]} {vertex[2]}\n")
        idx = 0
        f.write("vt 0.0 0.0\nvt 1.0 0.0\nvt 0.0 1.0\nvt 1.0 1.0\n")

        f.write("usemtl " + filename + "\n")
        while idx < len(faces):
            num_vertices = faces[idx]  # 面片顶点数量
            idx += 1
            face = faces[idx:idx + num_vertices]
            idx += num_vertices
            # `.obj` 格式的面片是 1-based 索引，因此需要加 1
            f.write(f"f {' '.join([str(i + 1)+'/'+str(i+1) for i in face])}\n")

def save_mtl(filepath, file_name):
    mtl_content = """
    newmtl red_material
    Ka 1.000 1.000 1.000
    Kd 1.000 1.000 1.000
    Ks 0.000 0.000 0.000
    map_Kd door.jpg
    """
    # 保存 mtl 文件
    with open(filepath, "w") as mtl_file:
        mtl_file.write("newmtl " + file_name + '\n')
        mtl_file.write("Ka 1.000 1.000 1.000\n")
        mtl_file.write("Kd 1.000 1.000 1.000\n")
        mtl_file.write("Ks 0.000 0.000 0.000\n")
        mtl_file.write("map_Kd door.jpg\n")

wall_path = "C:/SLAM/floorplan_code_v1/data/floorplan"
p = pv.Plotter()
res_mesh = []
res_sp_walls = []
cur_array = []
with open(wall_path, 'r') as file:
    for line in file:
        row = list(map(float, line.strip().split()))
        cur_array.append(row)
for data in cur_array:
    st_p = (data[0], data[2])
    ed_p = (data[1], data[3])
    res_sp_walls.append((st_p, ed_p, 0))

for wall in res_sp_walls:
    if wall[2] == 0:
        A = np.array([wall[0][0], wall[0][1], 0])  # 线段A的坐标 (x1, y1, z1)
        B = np.array([wall[1][0], wall[1][1], 0])  # 线段B的坐标 (x2, y2, z2)
        height = 3.2  # 给定的高度
        top_A = A + np.array([0, 0, height])  # 顶部A
        top_B = B + np.array([0, 0, height])  # 顶部B
        vertices = np.array([A, B, top_A, top_B])
        faces = np.array([
            [3, 0, 1, 2],  # 第一个三角形 ABC
            [3, 1, 2, 3]  # 第二个三角形 ABD
        ])
        mesh = pv.PolyData(vertices, faces)
        p.add_mesh(mesh)
        res_mesh.append(mesh)
    elif wall[2] == 1:
        z_height = wall[3][1] - wall[3][0]
        A = np.array([wall[0][0], wall[0][1], z_height])  # 线段A的坐标 (x1, y1, z1)
        B = np.array([wall[1][0], wall[1][1], z_height])  # 线段B的坐标 (x2, y2, z2)

        top_A = A + np.array([0, 0, 2.4 - z_height])  # 顶部A
        top_B = B + np.array([0, 0, 2.4 - z_height])  # 顶部B
        vertices = np.array([A, B, top_A, top_B])
        faces = np.array([
            [3, 0, 1, 2],  # 第一个三角形 ABC
            [3, 1, 2, 3]  # 第二个三角形 ABD
        ])
        mesh = pv.PolyData(vertices, faces)
        p.add_mesh(mesh)
        res_mesh.append(mesh)
    else:
        z_min, z_max = wall[3][0], wall[3][1]
        window_top = 2
        window_bottom = 0.7
        A = np.array([wall[0][0], wall[0][1], 0])  # 线段A的坐标 (x1, y1, z1)
        B = np.array([wall[1][0], wall[1][1], 0])  # 线段B的坐标 (x2, y2, z2)
        top_A = A + np.array([0, 0, z_min])  # 顶部A
        top_B = B + np.array([0, 0, z_min])  # 顶部B
        vertices = np.array([A, B, top_A, top_B])
        faces = np.array([
            [3, 0, 1, 2],  # 第一个三角形 ABC
            [3, 1, 2, 3]  # 第二个三角形 ABD
        ])
        mesh1 = pv.PolyData(vertices, faces)
        p.add_mesh(mesh1)
        A = np.array([wall[0][0], wall[0][1], z_max])  # 线段A的坐标 (x1, y1, z1)
        B = np.array([wall[1][0], wall[1][1], z_max])  # 线段B的坐标 (x2, y2, z2)
        top_A = A + np.array([0, 0, 2.4 - z_max])  # 顶部A
        top_B = B + np.array([0, 0, 2.4 - z_max])  # 顶部B
        vertices = np.array([A, B, top_A, top_B])
        faces = np.array([
            [3, 0, 1, 2],  # 第一个三角形 ABC
            [3, 1, 2, 3]  # 第二个三角形 ABD
        ])
        mesh2 = pv.PolyData(vertices, faces)
        p.add_mesh(mesh2)
        res_mesh.append(mesh1)
        res_mesh.append(mesh2)

combined_mesh = res_mesh[0]
for mesh in res_mesh[1:]:
    combined_mesh = combined_mesh.merge(mesh)

# 保存合并后的 mesh 为 obj 文件
combined_mesh.save("C:/SLAM/floorplan_code_v1/data/109_wall_2.obj")
print('visualization done')
p.show()