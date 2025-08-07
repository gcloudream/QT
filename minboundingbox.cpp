// MinBoundingBox.cpp
#include "MinBoundingBox.h"
#include <cmath>  // 添加数学函数支持


//没有使用过

/* 构造函数：初始化无效距离阈值并重置成员变量 */
MinBoundingBox::MinBoundingBox(void)
    : m_finvalidDis(1e10f)  // 🔧 修复：设置更大的无效距离阈值(100亿米)，支持大坐标系
{
    zerolize(); // 初始化成员变量
}

/* 析构函数：空实现（无动态资源需要释放） */
MinBoundingBox::~MinBoundingBox(void)
{
}

/**
 * @brief 计算点云的最小轴对齐包围盒
 * @param cloud 输入点云数据
 * @return bool 计算是否成功（至少包含2个有效点时返回true）
 *
 * 算法流程：
 * 1. 重置所有成员变量
 * 2. 处理空点云和单点特殊情况
 * 3. 遍历点云找到有效点范围
 * 4. 计算包围盒中心点
 */
bool MinBoundingBox::calculateMinBoundingBox(const std::vector<QVector3D>& cloud)
{
    zerolize();  // 重置计算结果
    int size = (int)cloud.size();

    // 空点云处理
    if (size == 0) {
        return false;
    }
    // 单点特殊情况处理
    else if (size == 1) {
        firstPoint(cloud[0]);  // 记录第一个有效点
        return false;          // 单点不算有效包围盒
    }
    // 多点处理
    else {
        bool bfirst = false;   // 首个有效点标记
        for (int i = 0; i < size; i++) {
            if (!bfirst) {
                if (isValid(cloud[i])) {  // 过滤无效点
                    firstPoint(cloud[i]); // 初始化包围盒
                    bfirst = true;
                }
            } else {
                nextPoint(cloud[i]);  // 扩展包围盒范围
            }
        }
        m_center = QVector3D(midX(), midY(), midZ());  // 计算包围盒中心
    }
    return true;
}

/**
 * @brief 更新最小/最大坐标值
 * @param point 输入点坐标
 * @param min [输入输出]当前最小坐标
 * @param max [输入输出]当前最大坐标
 *
 * 逐个分量比较并更新极值：
 * - X/Y/Z分量分别比较
 * - 使用std::min/max保证正确性
 */
void MinBoundingBox::setMinMax(QVector3D point, QVector3D &min, QVector3D &max)
{
    min.setX(std::min(point.x(), min.x()));
    min.setY(std::min(point.y(), min.y()));
    min.setZ(std::min(point.z(), min.z()));

    max.setX(std::max(point.x(), max.x()));
    max.setY(std::max(point.y(), max.y()));
    max.setZ(std::max(point.z(), max.z()));
}

/* 赋值运算符重载（实现深拷贝） */
MinBoundingBox& MinBoundingBox::operator=(MinBoundingBox &box)
{
    // 拷贝所有成员变量
    this->m_min = box.m_min;
    this->m_max = box.m_max;
    this->m_mean = box.m_mean;
    this->m_center = box.m_center;
    this->index = box.index;
    return *this;  // 支持链式赋值
}

/**
 * @brief 处理后续点数据
 * @param p 输入点坐标
 *
 * 流程：
 * 1. 有效性检查（过滤异常点）
 * 2. 更新包围盒范围
 * 3. 增加处理点计数器
 */
void MinBoundingBox::nextPoint(QVector3D p)
{
    if (!isValid(p)) return;  // 跳过无效点

    setMinMax(p, m_min, m_max);  // 更新极值
    index++;  // 增加有效点计数
}

/* 判断点是否在包围盒内部 */
bool MinBoundingBox::isContain(QVector3D point)
{
    return (point.x() <= m_max.x() && point.x() >= m_min.x() &&
            point.y() <= m_max.y() && point.y() >= m_min.y() &&
            point.z() <= m_max.z() && point.z() >= m_min.z());
}

/* 点有效性检查（过滤异常值） - 改进版本 */
bool MinBoundingBox::isValid(QVector3D point)
{
    // 检查是否为有限数值（排除NaN和无穷大）
    if (!std::isfinite(point.x()) || !std::isfinite(point.y()) || !std::isfinite(point.z())) {
        return false;
    }

    // 检查坐标绝对值是否在合理范围内
    // 使用更大的阈值以支持大坐标系统（如UTM坐标系）
    return !(std::abs(point.x()) > m_finvalidDis ||
             std::abs(point.y()) > m_finvalidDis ||
             std::abs(point.z()) > m_finvalidDis);
}

// // 其他成员函数实现（示例，需根据实际内容补充）
// void MinBoundingBox::zerolize()
// {
//     /* 重置所有极值为极大/极小值：
//      * - m_min初始化为最大浮点数
//      * - m_max初始化为最小浮点数
//      * - 其他成员重置为0
//      */
//     m_min = QVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
//     m_max = QVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX);
//     m_mean = QVector3D(0,0,0);
//     m_center = QVector3D(0,0,0);
//     index = 0;
// }

// void MinBoundingBox::firstPoint(QVector3D p)
// {
//     /* 初始化第一个有效点：
//      * - 将第一个点同时设为min和max
//      * - 重置计数器
//      */
//     m_min = p;
//     m_max = p;
//     index = 1;
// }

// // 中心点坐标计算（需根据实际实现补充）
// float MinBoundingBox::midX() const { return (m_min.x() + m_max.x())*0.5f; }
// float MinBoundingBox::midY() const { return (m_min.y() + m_max.y())*0.5f; }
// float MinBoundingBox::midZ() const { return (m_min.z() + m_max.z())*0.5f; }
