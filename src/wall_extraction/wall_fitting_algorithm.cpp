#include "wall_fitting_algorithm.h"
#include "line_drawing_tool.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>
#include <cmath>

namespace WallExtraction {

// Plane3D 方法实现
float Plane3D::distanceToPoint(const QVector3D& point) const
{
    return qAbs(QVector3D::dotProduct(normal, point - this->point));
}

bool Plane3D::containsPoint(const QVector3D& point, float tolerance) const
{
    return distanceToPoint(point) <= tolerance;
}

// WallSegment 方法实现
float WallSegment::length() const
{
    return startPoint.distanceToPoint(endPoint);
}

float WallSegment::area() const
{
    return length() * height;
}

// WallFittingAlgorithm 构造函数和基本方法
WallFittingAlgorithm::WallFittingAlgorithm(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_isProcessing(false)
    , m_totalIterations(0)
    , m_successfulFits(0)
{
    qDebug() << "WallFittingAlgorithm created";
}

WallFittingAlgorithm::~WallFittingAlgorithm()
{
    qDebug() << "WallFittingAlgorithm destroyed";
}

bool WallFittingAlgorithm::initialize()
{
    if (m_initialized) {
        return true;
    }

    try {
        initializeParameters();
        setupCallbacks();

        m_initialized = true;
        qDebug() << "WallFittingAlgorithm initialized successfully";

        return true;
    } catch (const std::exception& e) {
        qDebug() << "WallFittingAlgorithm initialization failed:" << e.what();
        emit errorOccurred(QString("初始化失败: %1").arg(e.what()));
        return false;
    }
}

bool WallFittingAlgorithm::isInitialized() const
{
    return m_initialized;
}

void WallFittingAlgorithm::reset()
{
    m_isProcessing = false;
    m_totalIterations = 0;
    m_successfulFits = 0;
    m_lastDetectedPlanes.clear();
    m_lastExtractedWalls.clear();

    qDebug() << "WallFittingAlgorithm reset";
}

void WallFittingAlgorithm::initializeParameters()
{
    // 使用默认参数
    m_parameters = RANSACParameters();
    qDebug() << "RANSAC parameters initialized";
}

void WallFittingAlgorithm::setupCallbacks()
{
    // 连接内部信号
    connect(this, &WallFittingAlgorithm::progressChanged,
            this, &WallFittingAlgorithm::onInternalProgress);
}

// 参数设置
void WallFittingAlgorithm::setRANSACParameters(const RANSACParameters& params)
{
    m_parameters = params;
    qDebug() << "RANSAC parameters updated";
}

RANSACParameters WallFittingAlgorithm::getRANSACParameters() const
{
    return m_parameters;
}

void WallFittingAlgorithm::setProgressCallback(std::function<void(int, const QString&)> callback)
{
    m_progressCallback = callback;
}

// 主要算法接口
WallFittingResult WallFittingAlgorithm::fitWallsFromPointCloud(const std::vector<QVector3D>& points)
{
    WallFittingResult result;

    if (!m_initialized) {
        result.errorMessage = "算法未初始化";
        emit processingFailed(result.errorMessage);
        return result;
    }

    if (!validatePointCloud(points)) {
        result.errorMessage = "点云数据无效";
        emit processingFailed(result.errorMessage);
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    m_isProcessing = true;
    m_processingStartTime = QDateTime::currentDateTime();
    emit processingStarted();

    try {
        reportProgress(0, "开始处理点云数据");

        // 步骤1：检测平面
        reportProgress(10, "检测垂直平面");
        std::vector<Plane3D> planes = detectPlanes(points);
        result.planes = planes;

        if (planes.empty()) {
            result.errorMessage = "未检测到垂直平面";
            emit processingFailed(result.errorMessage);
            return result;
        }

        emit planesDetected(planes);
        reportProgress(50, QString("检测到 %1 个平面").arg(planes.size()));

        // 步骤2：从平面提取墙面
        reportProgress(60, "提取墙面段");
        std::vector<WallSegment> walls = extractWallsFromPlanes(planes, points);
        result.walls = walls;

        if (walls.empty()) {
            result.errorMessage = "未能提取到墙面";
            emit processingFailed(result.errorMessage);
            return result;
        }

        // 步骤3：几何优化
        reportProgress(80, "优化墙面几何");
        optimizeWallGeometry(result.walls);

        // 完成处理
        result.totalPoints = static_cast<int>(points.size());
        result.processedPoints = result.totalPoints;
        result.processingTime = timer.elapsed() / 1000.0f;
        result.success = true;

        m_lastDetectedPlanes = result.planes;
        m_lastExtractedWalls = result.walls;

        emit wallsDetected(result.walls);
        emit processingCompleted(result);
        reportProgress(100, QString("完成：提取到 %1 个墙面").arg(result.walls.size()));

        qDebug() << "Wall fitting completed:" << result.walls.size() << "walls extracted";

    } catch (const std::exception& e) {
        result.errorMessage = QString("处理过程中发生错误: %1").arg(e.what());
        emit processingFailed(result.errorMessage);
        qDebug() << "Wall fitting failed:" << result.errorMessage;
    }

    m_isProcessing = false;
    return result;
}

WallFittingResult WallFittingAlgorithm::fitWallsFromLines(const std::vector<QVector3D>& points,
                                                         const std::vector<LineSegment>& userLines)
{
    WallFittingResult result;

    if (!m_initialized) {
        result.errorMessage = "算法未初始化";
        emit processingFailed(result.errorMessage);
        return result;
    }

    if (!validatePointCloud(points) || !validateUserLines(userLines)) {
        result.errorMessage = "输入数据无效";
        emit processingFailed(result.errorMessage);
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    m_isProcessing = true;
    emit processingStarted();

    try {
        reportProgress(0, "开始基于线段的墙面拟合");

        // 基于用户线段拟合墙面
        std::vector<WallSegment> walls = fitWallsAlongLines(points, userLines);
        result.walls = walls;

        if (walls.empty()) {
            result.errorMessage = "未能基于用户线段提取到墙面";
            emit processingFailed(result.errorMessage);
            return result;
        }

        reportProgress(80, "优化墙面几何");
        optimizeWallGeometry(result.walls);

        // 完成处理
        result.totalPoints = static_cast<int>(points.size());
        result.processedPoints = result.totalPoints;
        result.processingTime = timer.elapsed() / 1000.0f;
        result.success = true;

        m_lastExtractedWalls = result.walls;

        emit wallsDetected(result.walls);
        emit processingCompleted(result);
        reportProgress(100, QString("完成：基于 %1 条线段提取到 %2 个墙面")
                      .arg(userLines.size()).arg(result.walls.size()));

        qDebug() << "Line-based wall fitting completed:" << result.walls.size() << "walls extracted";

    } catch (const std::exception& e) {
        result.errorMessage = QString("处理过程中发生错误: %1").arg(e.what());
        emit processingFailed(result.errorMessage);
        qDebug() << "Line-based wall fitting failed:" << result.errorMessage;
    }

    m_isProcessing = false;
    return result;
}

// 平面检测实现
std::vector<Plane3D> WallFittingAlgorithm::detectPlanes(const std::vector<QVector3D>& points)
{
    std::vector<Plane3D> planes;

    if (points.size() < m_parameters.minPoints) {
        qDebug() << "点数不足，无法进行平面检测";
        return planes;
    }

    std::vector<bool> used(points.size(), false);
    std::random_device rd;
    std::mt19937 gen(rd());

    reportProgress(15, "执行RANSAC平面检测");

    // 多次RANSAC检测
    for (int planeCount = 0; planeCount < 10; ++planeCount) {
        // 收集未使用的点
        std::vector<int> availableIndices;
        for (size_t i = 0; i < points.size(); ++i) {
            if (!used[i]) {
                availableIndices.push_back(static_cast<int>(i));
            }
        }

        if (availableIndices.size() < m_parameters.minPoints) {
            break;
        }

        // 执行RANSAC
        Plane3D plane = fitPlaneRANSAC(points, availableIndices);

        if (plane.inlierIndices.size() < m_parameters.minPoints) {
            break;
        }

        // 检查是否为垂直平面
        if (isVerticalPlane(plane)) {
            planes.push_back(plane);

            // 标记已使用的点
            for (int idx : plane.inlierIndices) {
                used[idx] = true;
            }

            qDebug() << "检测到垂直平面，内点数:" << plane.inlierIndices.size();
        }

        updateProgress(planeCount + 1, 10, "平面检测");
    }

    // 过滤和聚类平面
    filterVerticalPlanes(planes);
    clusterPlanes(planes);

    qDebug() << "平面检测完成，共检测到" << planes.size() << "个垂直平面";
    return planes;
}

// RANSAC平面拟合核心算法
Plane3D WallFittingAlgorithm::fitPlaneRANSAC(const std::vector<QVector3D>& points,
                                             const std::vector<int>& indices)
{
    Plane3D bestPlane;
    int bestInlierCount = 0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(indices.size()) - 1);

    for (int iter = 0; iter < m_parameters.maxIterations; ++iter) {
        // 随机选择3个点
        if (indices.size() < 3) break;

        int idx1 = indices[dis(gen)];
        int idx2 = indices[dis(gen)];
        int idx3 = indices[dis(gen)];

        if (idx1 == idx2 || idx1 == idx3 || idx2 == idx3) {
            continue;
        }

        QVector3D p1 = points[idx1];
        QVector3D p2 = points[idx2];
        QVector3D p3 = points[idx3];

        // 计算平面法向量
        QVector3D v1 = p2 - p1;
        QVector3D v2 = p3 - p1;
        QVector3D normal = QVector3D::crossProduct(v1, v2).normalized();

        if (normal.length() < 0.1f) {
            continue; // 三点共线
        }

        // 创建候选平面
        Plane3D candidatePlane;
        candidatePlane.point = p1;
        candidatePlane.normal = normal;
        candidatePlane.distance = QVector3D::dotProduct(normal, p1);

        // 查找内点
        std::vector<int> inliers = findPlaneInliers(points, candidatePlane, m_parameters.epsilon);

        if (inliers.size() > bestInlierCount) {
            bestInlierCount = static_cast<int>(inliers.size());
            bestPlane = candidatePlane;
            bestPlane.inlierIndices = inliers;
        }

        // 早期终止条件
        if (bestInlierCount > points.size() * 0.8) {
            break;
        }
    }

    // 精化最佳平面
    if (bestInlierCount >= m_parameters.minPoints) {
        bestPlane = refinePlane(points, bestPlane.inlierIndices);
        bestPlane.confidence = static_cast<float>(bestInlierCount) / points.size();
    }

    return bestPlane;
}

// 查找平面内点
std::vector<int> WallFittingAlgorithm::findPlaneInliers(const std::vector<QVector3D>& points,
                                                       const Plane3D& plane, float threshold)
{
    std::vector<int> inliers;

    for (size_t i = 0; i < points.size(); ++i) {
        if (plane.containsPoint(points[i], threshold)) {
            inliers.push_back(static_cast<int>(i));
        }
    }

    return inliers;
}

// 精化平面参数
Plane3D WallFittingAlgorithm::refinePlane(const std::vector<QVector3D>& points,
                                          const std::vector<int>& inliers)
{
    if (inliers.size() < 3) {
        return Plane3D();
    }

    // 计算质心
    QVector3D centroid = QVector3D(0, 0, 0);
    for (int idx : inliers) {
        centroid += points[idx];
    }
    centroid /= static_cast<float>(inliers.size());

    // 计算协方差矩阵的特征向量（简化实现）
    QVector3D normal = calculateNormal(points);

    Plane3D refinedPlane;
    refinedPlane.point = centroid;
    refinedPlane.normal = normal.normalized();
    refinedPlane.distance = QVector3D::dotProduct(refinedPlane.normal, centroid);
    refinedPlane.inlierIndices = inliers;

    return refinedPlane;
}

// 墙面提取
std::vector<WallSegment> WallFittingAlgorithm::extractWallsFromPlanes(const std::vector<Plane3D>& planes,
                                                                      const std::vector<QVector3D>& points)
{
    std::vector<WallSegment> walls;

    reportProgress(65, "从平面构建墙面");

    for (size_t i = 0; i < planes.size(); ++i) {
        const Plane3D& plane = planes[i];

        WallSegment wall = buildWallFromPlane(plane, points);

        if (wall.length() >= m_parameters.minWallLength) {
            wall.id = static_cast<int>(walls.size());
            walls.push_back(wall);
        }

        updateProgress(static_cast<int>(i + 1), static_cast<int>(planes.size()), "构建墙面");
    }

    return walls;
}

// 从平面构建墙面
WallSegment WallFittingAlgorithm::buildWallFromPlane(const Plane3D& plane,
                                                    const std::vector<QVector3D>& points)
{
    WallSegment wall;

    // 收集平面上的点
    std::vector<QVector3D> planePoints;
    for (int idx : plane.inlierIndices) {
        planePoints.push_back(points[idx]);
        wall.supportingPoints.push_back(points[idx]);
    }

    if (planePoints.empty()) {
        return wall;
    }

    // 计算墙面边界
    calculateWallBoundaries(wall, planePoints);

    // 设置墙面属性
    wall.normal = plane.normal;
    wall.confidence = plane.confidence;

    // 估算墙面厚度
    estimateWallThickness(wall, points);

    return wall;
}

// 计算墙面边界
void WallFittingAlgorithm::calculateWallBoundaries(WallSegment& wall,
                                                  const std::vector<QVector3D>& points)
{
    if (points.empty()) {
        return;
    }

    // 简化实现：找到X-Y平面上的边界点
    float minX = points[0].x(), maxX = points[0].x();
    float minY = points[0].y(), maxY = points[0].y();
    float minZ = points[0].z(), maxZ = points[0].z();

    for (const QVector3D& point : points) {
        minX = qMin(minX, point.x());
        maxX = qMax(maxX, point.x());
        minY = qMin(minY, point.y());
        maxY = qMax(maxY, point.y());
        minZ = qMin(minZ, point.z());
        maxZ = qMax(maxZ, point.z());
    }

    // 确定主要方向
    float xRange = maxX - minX;
    float yRange = maxY - minY;

    if (xRange > yRange) {
        // 主要沿X方向
        wall.startPoint = QVector3D(minX, (minY + maxY) / 2, minZ);
        wall.endPoint = QVector3D(maxX, (minY + maxY) / 2, minZ);
    } else {
        // 主要沿Y方向
        wall.startPoint = QVector3D((minX + maxX) / 2, minY, minZ);
        wall.endPoint = QVector3D((minX + maxX) / 2, maxY, minZ);
    }

    wall.height = maxZ - minZ;
}

// 估算墙面厚度
void WallFittingAlgorithm::estimateWallThickness(WallSegment& wall,
                                                const std::vector<QVector3D>& points)
{
    // 简化实现：使用固定厚度
    wall.thickness = 0.2f; // 20cm

    // 在实际应用中，可以通过分析点云密度和分布来估算厚度
    Q_UNUSED(points)
}

// 基于用户线段的墙面拟合
std::vector<WallSegment> WallFittingAlgorithm::fitWallsAlongLines(const std::vector<QVector3D>& points,
                                                                 const std::vector<LineSegment>& userLines)
{
    std::vector<WallSegment> walls;

    reportProgress(20, "基于用户线段拟合墙面");

    for (size_t i = 0; i < userLines.size(); ++i) {
        const LineSegment& line = userLines[i];

        // 查找线段附近的点
        std::vector<QVector3D> nearbyPoints = findPointsNearLine(points, line, 2.0f);

        if (nearbyPoints.size() < m_parameters.minPoints) {
            continue;
        }

        // 拟合平面
        Plane3D plane = fitPlaneToLineAndPoints(line, nearbyPoints);

        if (plane.inlierIndices.size() >= m_parameters.minPoints && isVerticalPlane(plane)) {
            WallSegment wall = buildWallFromPlane(plane, points);
            wall.sourceLineIds.push_back(line.id);
            wall.id = static_cast<int>(walls.size());
            walls.push_back(wall);
        }

        updateProgress(static_cast<int>(i + 1), static_cast<int>(userLines.size()), "线段拟合");
    }

    return walls;
}

// 查找线段附近的点
std::vector<QVector3D> WallFittingAlgorithm::findPointsNearLine(const std::vector<QVector3D>& points,
                                                               const LineSegment& line,
                                                               float searchRadius)
{
    std::vector<QVector3D> nearbyPoints;

    QVector3D lineStart = line.startPoint;
    QVector3D lineEnd = line.endPoint;
    QVector3D lineDir = (lineEnd - lineStart).normalized();

    for (const QVector3D& point : points) {
        // 计算点到线段的距离
        QVector3D toPoint = point - lineStart;
        float projection = QVector3D::dotProduct(toPoint, lineDir);

        QVector3D closestPoint;
        if (projection < 0) {
            closestPoint = lineStart;
        } else if (projection > lineStart.distanceToPoint(lineEnd)) {
            closestPoint = lineEnd;
        } else {
            closestPoint = lineStart + lineDir * projection;
        }

        float distance = point.distanceToPoint(closestPoint);
        if (distance <= searchRadius) {
            nearbyPoints.push_back(point);
        }
    }

    return nearbyPoints;
}

// 基于线段和点拟合平面
Plane3D WallFittingAlgorithm::fitPlaneToLineAndPoints(const LineSegment& line,
                                                     const std::vector<QVector3D>& nearbyPoints)
{
    Plane3D plane;

    if (nearbyPoints.size() < 3) {
        return plane;
    }

    // 使用线段方向和垂直方向构建平面
    QVector3D lineDir = (line.endPoint - line.startPoint).normalized();
    QVector3D up = QVector3D(0, 0, 1);
    QVector3D normal = QVector3D::crossProduct(lineDir, up).normalized();

    plane.point = line.startPoint;
    plane.normal = normal;
    plane.distance = QVector3D::dotProduct(normal, line.startPoint);

    // 查找内点
    plane.inlierIndices = findPlaneInliers(nearbyPoints, plane, m_parameters.epsilon);
    plane.confidence = static_cast<float>(plane.inlierIndices.size()) / nearbyPoints.size();

    return plane;
}

// 几何优化
void WallFittingAlgorithm::optimizeWallGeometry(std::vector<WallSegment>& walls)
{
    if (walls.empty()) {
        return;
    }

    reportProgress(85, "优化墙面几何");

    // 合并平行墙面
    mergeParallelWalls(walls);

    // 规则化墙面交点
    regularizeWallIntersections(walls);

    qDebug() << "墙面几何优化完成";
}

void WallFittingAlgorithm::mergeParallelWalls(std::vector<WallSegment>& walls, float angleThreshold)
{
    // 简化实现：标记需要合并的墙面
    std::vector<bool> toRemove(walls.size(), false);

    for (size_t i = 0; i < walls.size(); ++i) {
        if (toRemove[i]) continue;

        for (size_t j = i + 1; j < walls.size(); ++j) {
            if (toRemove[j]) continue;

            // 检查是否平行
            float angle = qRadiansToDegrees(qAcos(qAbs(QVector3D::dotProduct(
                walls[i].normal, walls[j].normal))));

            if (angle < angleThreshold) {
                // 检查距离是否足够近
                float distance = walls[i].startPoint.distanceToPoint(walls[j].startPoint);
                if (distance < 1.0f) {
                    // 合并墙面（简化：保留第一个，标记第二个删除）
                    walls[i].supportingPoints.insert(walls[i].supportingPoints.end(),
                                                    walls[j].supportingPoints.begin(),
                                                    walls[j].supportingPoints.end());
                    toRemove[j] = true;
                }
            }
        }
    }

    // 移除标记的墙面
    walls.erase(std::remove_if(walls.begin(), walls.end(),
                              [&toRemove, &walls](const WallSegment& wall) {
                                  size_t index = &wall - &walls[0];
                                  return index < toRemove.size() && toRemove[index];
                              }), walls.end());
}

void WallFittingAlgorithm::regularizeWallIntersections(std::vector<WallSegment>& walls)
{
    // 简化实现：调整墙面端点以形成更好的交点
    for (size_t i = 0; i < walls.size(); ++i) {
        for (size_t j = i + 1; j < walls.size(); ++j) {
            // 检查墙面是否相交
            // 这里可以实现更复杂的交点计算和调整逻辑
            Q_UNUSED(walls[i])
            Q_UNUSED(walls[j])
        }
    }
}

// 静态工具方法
bool WallFittingAlgorithm::isVerticalPlane(const Plane3D& plane, float angleThreshold)
{
    // 计算法向量与水平面的夹角
    QVector3D horizontal(0, 0, 1);
    float angle = qRadiansToDegrees(qAcos(qAbs(QVector3D::dotProduct(plane.normal, horizontal))));

    // 垂直平面的法向量应该与水平面垂直（角度接近90度）
    return qAbs(angle - 90.0f) <= angleThreshold;
}

float WallFittingAlgorithm::calculatePlaneDistance(const Plane3D& plane1, const Plane3D& plane2)
{
    return qAbs(plane1.distance - plane2.distance);
}

bool WallFittingAlgorithm::arePlanesParallel(const Plane3D& plane1, const Plane3D& plane2,
                                            float angleThreshold)
{
    float angle = qRadiansToDegrees(qAcos(qAbs(QVector3D::dotProduct(plane1.normal, plane2.normal))));
    return angle <= angleThreshold;
}

// 几何计算辅助方法
QVector3D WallFittingAlgorithm::calculateCentroid(const std::vector<QVector3D>& points)
{
    if (points.empty()) {
        return QVector3D();
    }

    QVector3D centroid(0, 0, 0);
    for (const QVector3D& point : points) {
        centroid += point;
    }
    return centroid / static_cast<float>(points.size());
}

QVector3D WallFittingAlgorithm::calculateNormal(const std::vector<QVector3D>& points)
{
    if (points.size() < 3) {
        return QVector3D(0, 0, 1);
    }

    // 简化实现：使用前三个点计算法向量
    QVector3D v1 = points[1] - points[0];
    QVector3D v2 = points[2] - points[0];
    return QVector3D::crossProduct(v1, v2).normalized();
}

// 数据验证方法
bool WallFittingAlgorithm::validatePointCloud(const std::vector<QVector3D>& points)
{
    if (points.empty()) {
        emit warningOccurred("点云为空");
        return false;
    }

    if (points.size() < m_parameters.minPoints) {
        emit warningOccurred(QString("点云数量不足，需要至少 %1 个点").arg(m_parameters.minPoints));
        return false;
    }

    // 检查点云边界
    QVector3D minBounds = points[0];
    QVector3D maxBounds = points[0];

    for (const QVector3D& point : points) {
        minBounds.setX(qMin(minBounds.x(), point.x()));
        minBounds.setY(qMin(minBounds.y(), point.y()));
        minBounds.setZ(qMin(minBounds.z(), point.z()));

        maxBounds.setX(qMax(maxBounds.x(), point.x()));
        maxBounds.setY(qMax(maxBounds.y(), point.y()));
        maxBounds.setZ(qMax(maxBounds.z(), point.z()));
    }

    QVector3D size = maxBounds - minBounds;
    if (size.length() < 1.0f) {
        emit warningOccurred("点云范围过小");
        return false;
    }

    return true;
}

bool WallFittingAlgorithm::validateUserLines(const std::vector<LineSegment>& lines)
{
    if (lines.empty()) {
        emit warningOccurred("用户线段为空");
        return false;
    }

    for (const LineSegment& line : lines) {
        if (line.length() < 0.1f) {
            emit warningOccurred("存在过短的用户线段");
            return false;
        }
    }

    return true;
}

bool WallFittingAlgorithm::validateResult(const WallFittingResult& result)
{
    if (!result.success) {
        return false;
    }

    for (const WallSegment& wall : result.walls) {
        if (wall.length() < m_parameters.minWallLength) {
            return false;
        }
        if (wall.confidence < 0.1f) {
            return false;
        }
    }

    return true;
}

// 进度报告方法
void WallFittingAlgorithm::reportProgress(int percentage, const QString& status)
{
    emit progressChanged(percentage, status);

    if (m_progressCallback) {
        m_progressCallback(percentage, status);
    }
}

void WallFittingAlgorithm::updateProgress(int currentStep, int totalSteps, const QString& operation)
{
    int percentage = (currentStep * 100) / totalSteps;
    QString status = QString("%1 (%2/%3)").arg(operation).arg(currentStep).arg(totalSteps);
    reportProgress(percentage, status);
}

// 平面处理辅助方法
void WallFittingAlgorithm::filterVerticalPlanes(std::vector<Plane3D>& planes)
{
    planes.erase(std::remove_if(planes.begin(), planes.end(),
                               [this](const Plane3D& plane) {
                                   return !isVerticalPlane(plane);
                               }), planes.end());
}

void WallFittingAlgorithm::clusterPlanes(std::vector<Plane3D>& planes)
{
    // 简化实现：按距离聚类相似平面
    std::vector<bool> processed(planes.size(), false);

    for (size_t i = 0; i < planes.size(); ++i) {
        if (processed[i]) continue;

        for (size_t j = i + 1; j < planes.size(); ++j) {
            if (processed[j]) continue;

            if (arePlanesParallel(planes[i], planes[j], 5.0f) &&
                calculatePlaneDistance(planes[i], planes[j]) < 0.5f) {

                // 合并平面
                planes[i].inlierIndices.insert(planes[i].inlierIndices.end(),
                                              planes[j].inlierIndices.begin(),
                                              planes[j].inlierIndices.end());
                processed[j] = true;
            }
        }
    }

    // 移除已处理的平面
    planes.erase(std::remove_if(planes.begin(), planes.end(),
                               [&processed, &planes](const Plane3D& plane) {
                                   size_t index = &plane - &planes[0];
                                   return index < processed.size() && processed[index];
                               }), planes.end());
}

float WallFittingAlgorithm::calculateVariance(const std::vector<QVector3D>& points, const Plane3D& plane)
{
    if (points.empty()) {
        return 0.0f;
    }

    float variance = 0.0f;
    for (const QVector3D& point : points) {
        float distance = plane.distanceToPoint(point);
        variance += distance * distance;
    }

    return variance / points.size();
}

// 槽函数
void WallFittingAlgorithm::onInternalProgress(int percentage, const QString& status)
{
    qDebug() << "Wall fitting progress:" << percentage << "%" << status;
}

} // namespace WallExtraction
