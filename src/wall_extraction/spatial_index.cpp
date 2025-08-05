#include "spatial_index.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

namespace WallExtraction {

SpatialIndex::SpatialIndex(QObject* parent)
    : QObject(parent)
    , m_indexType(SpatialIndexType::Octree)
    , m_indexBuilt(false)
    , m_maxLeafCapacity(10)
    , m_maxTreeDepth(10)
    , m_statisticsValid(false)
{
    // 注册元类型
    qRegisterMetaType<SpatialIndexType>("SpatialIndexType");
    qRegisterMetaType<QueryResult>("QueryResult");
    
    qDebug() << "SpatialIndex created with Octree as default";
}

SpatialIndex::~SpatialIndex()
{
    clearIndex();
    qDebug() << "SpatialIndex destroyed";
}

void SpatialIndex::setIndexType(SpatialIndexType type)
{
    if (m_indexType != type) {
        m_indexType = type;
        
        // 如果已有索引，需要重新构建
        if (m_indexBuilt) {
            emit statusMessage("Index type changed, rebuilding required");
            clearIndex();
        }
        
        emit statusMessage(QString("Index type set to %1")
                          .arg(type == SpatialIndexType::Octree ? "Octree" : "KDTree"));
    }
}

SpatialIndexType SpatialIndex::getIndexType() const
{
    return m_indexType;
}

bool SpatialIndex::supportsKDTree() const
{
    return true; // 当前实现支持KD树
}

bool SpatialIndex::buildIndex(const std::vector<QVector3D>& points)
{
    if (points.empty()) {
        emit errorOccurred("Cannot build index from empty point cloud");
        return false;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    emit statusMessage(QString("Building %1 index for %2 points...")
                      .arg(m_indexType == SpatialIndexType::Octree ? "Octree" : "KDTree")
                      .arg(points.size()));
    
    // 清除现有索引
    clearIndex();
    
    // 存储点云数据
    m_points = points;
    
    // 计算边界框
    auto boundingBox = computeBoundingBox(points);
    m_boundingBoxMin = boundingBox.first;
    m_boundingBoxMax = boundingBox.second;
    
    bool success = false;
    
    try {
        // 根据索引类型构建
        if (m_indexType == SpatialIndexType::Octree) {
            success = buildOctree(points);
        } else {
            success = buildKDTree(points);
        }
        
        if (success) {
            m_indexBuilt = true;
            updateStatistics();
            
            qint64 elapsed = timer.elapsed();
            emit statusMessage(QString("Index built successfully in %1 ms").arg(elapsed));
        } else {
            emit errorOccurred("Failed to build spatial index");
        }
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Exception during index building: %1").arg(e.what()));
        clearIndex();
        success = false;
    }
    
    return success;
}

bool SpatialIndex::insertPoint(const QVector3D& point)
{
    if (!m_indexBuilt) {
        emit errorOccurred("Cannot insert point: index not built");
        return false;
    }
    
    try {
        // 添加到点云数据
        m_points.push_back(point);
        size_t pointIndex = m_points.size() - 1;
        
        // 根据索引类型插入
        if (m_indexType == SpatialIndexType::Octree) {
            insertPointOctree(point, pointIndex);
        } else {
            // KD树插入比较复杂，这里简化为重建
            emit statusMessage("KDTree insertion requires rebuild");
            return buildIndex(m_points);
        }
        
        // 更新边界框
        m_boundingBoxMin.setX(qMin(m_boundingBoxMin.x(), point.x()));
        m_boundingBoxMin.setY(qMin(m_boundingBoxMin.y(), point.y()));
        m_boundingBoxMin.setZ(qMin(m_boundingBoxMin.z(), point.z()));
        
        m_boundingBoxMax.setX(qMax(m_boundingBoxMax.x(), point.x()));
        m_boundingBoxMax.setY(qMax(m_boundingBoxMax.y(), point.y()));
        m_boundingBoxMax.setZ(qMax(m_boundingBoxMax.z(), point.z()));
        
        m_statisticsValid = false; // 标记统计信息需要更新
        return true;
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Exception during point insertion: %1").arg(e.what()));
        return false;
    }
}

bool SpatialIndex::removePoint(size_t pointIndex)
{
    if (!m_indexBuilt || pointIndex >= m_points.size()) {
        return false;
    }
    
    // 简化实现：标记点为无效而不是真正删除
    // 实际应用中可能需要更复杂的删除逻辑
    emit statusMessage("Point removal requires index rebuild");
    
    // 从点云数据中移除
    m_points.erase(m_points.begin() + pointIndex);
    
    // 重建索引
    return buildIndex(m_points);
}

std::vector<QueryResult> SpatialIndex::queryRadius(const QVector3D& center, float radius) const
{
    std::vector<QueryResult> results;
    
    if (!m_indexBuilt || radius <= 0.0f) {
        return results;
    }
    
    try {
        if (m_indexType == SpatialIndexType::Octree) {
            queryRadiusOctree(m_octreeRoot.get(), center, radius, results);
        } else {
            queryRadiusKDTree(m_kdtreeRoot.get(), center, radius, results);
        }
        
        // 按距离排序
        std::sort(results.begin(), results.end(), 
                 [](const QueryResult& a, const QueryResult& b) {
                     return a.distance < b.distance;
                 });
        
    } catch (const std::exception& e) {
        const_cast<SpatialIndex*>(this)->emitErrorOccurred(QString("Exception during radius query: %1").arg(e.what()));
    }
    
    return results;
}

std::vector<QueryResult> SpatialIndex::queryKNN(const QVector3D& queryPoint, int k) const
{
    std::vector<QueryResult> results;
    
    if (!m_indexBuilt || k <= 0) {
        return results;
    }
    
    try {
        if (m_indexType == SpatialIndexType::KDTree) {
            queryKNNKDTree(m_kdtreeRoot.get(), queryPoint, k, results);
        } else {
            // 对于八叉树，使用渐进式范围查询实现KNN
            float radius = 1.0f;
            while (results.size() < k && radius < 1000.0f) {
                results = queryRadius(queryPoint, radius);
                radius *= 2.0f;
            }
            
            // 截取前k个结果
            if (results.size() > k) {
                results.resize(k);
            }
        }
        
    } catch (const std::exception& e) {
        const_cast<SpatialIndex*>(this)->emitErrorOccurred(QString("Exception during KNN query: %1").arg(e.what()));
    }
    
    return results;
}

std::vector<QueryResult> SpatialIndex::queryBoundingBox(const QVector3D& minPoint, 
                                                       const QVector3D& maxPoint) const
{
    std::vector<QueryResult> results;
    
    if (!m_indexBuilt) {
        return results;
    }
    
    try {
        if (m_indexType == SpatialIndexType::Octree) {
            queryBoundingBoxOctree(m_octreeRoot.get(), minPoint, maxPoint, results);
        } else {
            // KD树的边界框查询实现较复杂，这里使用简化版本
            for (size_t i = 0; i < m_points.size(); ++i) {
                if (isPointInBoundingBox(m_points[i], minPoint, maxPoint)) {
                    float distance = calculateDistance(m_points[i], (minPoint + maxPoint) * 0.5f);
                    results.emplace_back(i, distance);
                }
            }
        }
        
    } catch (const std::exception& e) {
        const_cast<SpatialIndex*>(this)->emitErrorOccurred(QString("Exception during bounding box query: %1").arg(e.what()));
    }
    
    return results;
}

QVariantMap SpatialIndex::getIndexStatistics() const
{
    if (!m_statisticsValid) {
        const_cast<SpatialIndex*>(this)->updateStatistics();
    }
    
    return m_statistics;
}

void SpatialIndex::clearIndex()
{
    m_octreeRoot.reset();
    m_kdtreeRoot.reset();
    m_points.clear();
    m_indexBuilt = false;
    m_statisticsValid = false;
    m_statistics.clear();
    
    emit statusMessage("Index cleared");
}

bool SpatialIndex::isIndexBuilt() const
{
    return m_indexBuilt;
}

size_t SpatialIndex::getPointCount() const
{
    return m_points.size();
}

void SpatialIndex::setMaxLeafCapacity(int capacity)
{
    if (capacity > 0) {
        m_maxLeafCapacity = capacity;
        emit statusMessage(QString("Max leaf capacity set to %1").arg(capacity));
    }
}

int SpatialIndex::getMaxLeafCapacity() const
{
    return m_maxLeafCapacity;
}

void SpatialIndex::setMaxTreeDepth(int depth)
{
    if (depth > 0) {
        m_maxTreeDepth = depth;
        emit statusMessage(QString("Max tree depth set to %1").arg(depth));
    }
}

int SpatialIndex::getMaxTreeDepth() const
{
    return m_maxTreeDepth;
}

// 辅助方法实现
float SpatialIndex::calculateDistance(const QVector3D& p1, const QVector3D& p2) const
{
    return (p1 - p2).length();
}

bool SpatialIndex::isPointInRadius(const QVector3D& point, const QVector3D& center, float radius) const
{
    return calculateDistance(point, center) <= radius;
}

bool SpatialIndex::isPointInBoundingBox(const QVector3D& point, const QVector3D& minPoint, 
                                       const QVector3D& maxPoint) const
{
    return point.x() >= minPoint.x() && point.x() <= maxPoint.x() &&
           point.y() >= minPoint.y() && point.y() <= maxPoint.y() &&
           point.z() >= minPoint.z() && point.z() <= maxPoint.z();
}

std::pair<QVector3D, QVector3D> SpatialIndex::computeBoundingBox(const std::vector<QVector3D>& points) const
{
    if (points.empty()) {
        return {QVector3D(), QVector3D()};
    }
    
    QVector3D minPoint = points[0];
    QVector3D maxPoint = points[0];
    
    for (const auto& point : points) {
        minPoint.setX(qMin(minPoint.x(), point.x()));
        minPoint.setY(qMin(minPoint.y(), point.y()));
        minPoint.setZ(qMin(minPoint.z(), point.z()));
        
        maxPoint.setX(qMax(maxPoint.x(), point.x()));
        maxPoint.setY(qMax(maxPoint.y(), point.y()));
        maxPoint.setZ(qMax(maxPoint.z(), point.z()));
    }
    
    return {minPoint, maxPoint};
}

void SpatialIndex::updateStatistics()
{
    m_statistics.clear();

    m_statistics["index_type"] = (m_indexType == SpatialIndexType::Octree) ? "Octree" : "KDTree";
    m_statistics["point_count"] = static_cast<qulonglong>(m_points.size());
    m_statistics["index_built"] = m_indexBuilt;
    m_statistics["max_leaf_capacity"] = m_maxLeafCapacity;
    m_statistics["max_tree_depth"] = m_maxTreeDepth;

    if (m_indexBuilt) {
        QVector3D size = m_boundingBoxMax - m_boundingBoxMin;
        m_statistics["bounding_box_size_x"] = size.x();
        m_statistics["bounding_box_size_y"] = size.y();
        m_statistics["bounding_box_size_z"] = size.z();
        m_statistics["bounding_box_volume"] = size.x() * size.y() * size.z();
    }

    m_statisticsValid = true;
}

// 八叉树实现
bool SpatialIndex::buildOctree(const std::vector<QVector3D>& points)
{
    if (points.empty()) {
        return false;
    }

    // 计算根节点的中心和大小
    QVector3D center = (m_boundingBoxMin + m_boundingBoxMax) * 0.5f;
    QVector3D size = m_boundingBoxMax - m_boundingBoxMin;
    float halfSize = qMax(qMax(size.x(), size.y()), size.z()) * 0.5f;

    // 创建根节点
    m_octreeRoot = createOctreeNode(center, halfSize);

    // 插入所有点
    for (size_t i = 0; i < points.size(); ++i) {
        insertPointOctree(points[i], i);

        // 更新进度
        if (i % 1000 == 0) {
            int progress = static_cast<int>((i * 100) / points.size());
            emit indexBuildProgress(progress);
        }
    }

    emit indexBuildProgress(100);
    return true;
}

std::unique_ptr<OctreeNode> SpatialIndex::createOctreeNode(const QVector3D& center, float halfSize)
{
    return std::make_unique<OctreeNode>(center, halfSize);
}

void SpatialIndex::insertPointOctree(const QVector3D& point, size_t pointIndex)
{
    if (!m_octreeRoot) {
        return;
    }

    std::function<void(OctreeNode*, int)> insertRecursive = [&](OctreeNode* node, int depth) {
        if (depth >= m_maxTreeDepth) {
            node->pointIndices.push_back(pointIndex);
            return;
        }

        if (node->isLeaf) {
            node->pointIndices.push_back(pointIndex);

            // 如果超过容量且未达到最大深度，进行细分
            if (node->pointIndices.size() > m_maxLeafCapacity && depth < m_maxTreeDepth) {
                subdivideOctreeNode(node);

                // 重新分配点到子节点
                std::vector<size_t> tempIndices = node->pointIndices;
                node->pointIndices.clear();

                for (size_t idx : tempIndices) {
                    int octant = getOctant(m_points[idx], node->center);
                    if (octant >= 0 && octant < 8 && node->children[octant]) {
                        insertRecursive(node->children[octant].get(), depth + 1);
                    }
                }
            }
        } else {
            // 内部节点，找到合适的子节点
            int octant = getOctant(point, node->center);
            if (octant >= 0 && octant < 8 && node->children[octant]) {
                insertRecursive(node->children[octant].get(), depth + 1);
            }
        }
    };

    insertRecursive(m_octreeRoot.get(), 0);
}

void SpatialIndex::subdivideOctreeNode(OctreeNode* node)
{
    if (!node || !node->isLeaf) {
        return;
    }

    node->isLeaf = false;
    float childHalfSize = node->halfSize * 0.5f;

    // 创建8个子节点
    for (int i = 0; i < 8; ++i) {
        QVector3D childCenter = getChildCenter(node->center, node->halfSize, i);
        node->children[i] = createOctreeNode(childCenter, childHalfSize);
    }
}

int SpatialIndex::getOctant(const QVector3D& point, const QVector3D& center) const
{
    int octant = 0;
    if (point.x() >= center.x()) octant |= 1;
    if (point.y() >= center.y()) octant |= 2;
    if (point.z() >= center.z()) octant |= 4;
    return octant;
}

QVector3D SpatialIndex::getChildCenter(const QVector3D& parentCenter, float parentHalfSize, int octant) const
{
    float offset = parentHalfSize * 0.5f;
    QVector3D childCenter = parentCenter;

    if (octant & 1) childCenter.setX(childCenter.x() + offset);
    else childCenter.setX(childCenter.x() - offset);

    if (octant & 2) childCenter.setY(childCenter.y() + offset);
    else childCenter.setY(childCenter.y() - offset);

    if (octant & 4) childCenter.setZ(childCenter.z() + offset);
    else childCenter.setZ(childCenter.z() - offset);

    return childCenter;
}

void SpatialIndex::queryRadiusOctree(const OctreeNode* node, const QVector3D& center,
                                     float radius, std::vector<QueryResult>& results) const
{
    if (!node) {
        return;
    }

    // 检查节点边界框是否与查询球相交
    QVector3D nodeMin = node->center - QVector3D(node->halfSize, node->halfSize, node->halfSize);
    QVector3D nodeMax = node->center + QVector3D(node->halfSize, node->halfSize, node->halfSize);

    // 计算节点到查询中心的最近距离
    QVector3D closestPoint;
    closestPoint.setX(qMax(nodeMin.x(), qMin(center.x(), nodeMax.x())));
    closestPoint.setY(qMax(nodeMin.y(), qMin(center.y(), nodeMax.y())));
    closestPoint.setZ(qMax(nodeMin.z(), qMin(center.z(), nodeMax.z())));

    float distanceToNode = calculateDistance(center, closestPoint);
    if (distanceToNode > radius) {
        return; // 节点不在查询范围内
    }

    if (node->isLeaf) {
        // 叶节点，检查所有点
        for (size_t idx : node->pointIndices) {
            if (idx < m_points.size()) {
                float distance = calculateDistance(m_points[idx], center);
                if (distance <= radius) {
                    results.emplace_back(idx, distance);
                }
            }
        }
    } else {
        // 内部节点，递归查询子节点
        for (const auto& child : node->children) {
            if (child) {
                queryRadiusOctree(child.get(), center, radius, results);
            }
        }
    }
}

void SpatialIndex::queryBoundingBoxOctree(const OctreeNode* node, const QVector3D& minPoint,
                                         const QVector3D& maxPoint, std::vector<QueryResult>& results) const
{
    if (!node) {
        return;
    }

    // 检查节点边界框是否与查询边界框相交
    QVector3D nodeMin = node->center - QVector3D(node->halfSize, node->halfSize, node->halfSize);
    QVector3D nodeMax = node->center + QVector3D(node->halfSize, node->halfSize, node->halfSize);

    if (nodeMax.x() < minPoint.x() || nodeMin.x() > maxPoint.x() ||
        nodeMax.y() < minPoint.y() || nodeMin.y() > maxPoint.y() ||
        nodeMax.z() < minPoint.z() || nodeMin.z() > maxPoint.z()) {
        return; // 不相交
    }

    if (node->isLeaf) {
        // 叶节点，检查所有点
        for (size_t idx : node->pointIndices) {
            if (idx < m_points.size() && isPointInBoundingBox(m_points[idx], minPoint, maxPoint)) {
                QVector3D center = (minPoint + maxPoint) * 0.5f;
                float distance = calculateDistance(m_points[idx], center);
                results.emplace_back(idx, distance);
            }
        }
    } else {
        // 内部节点，递归查询子节点
        for (const auto& child : node->children) {
            if (child) {
                queryBoundingBoxOctree(child.get(), minPoint, maxPoint, results);
            }
        }
    }
}

// KD树实现
bool SpatialIndex::buildKDTree(const std::vector<QVector3D>& points)
{
    if (points.empty()) {
        return false;
    }

    // 创建点索引数组
    std::vector<size_t> indices(points.size());
    std::iota(indices.begin(), indices.end(), 0);

    // 递归构建KD树
    m_kdtreeRoot = buildKDTreeRecursive(indices, 0);

    emit indexBuildProgress(100);
    return m_kdtreeRoot != nullptr;
}

std::unique_ptr<KDTreeNode> SpatialIndex::buildKDTreeRecursive(std::vector<size_t>& indices, int depth)
{
    if (indices.empty()) {
        return nullptr;
    }

    if (indices.size() == 1) {
        return std::make_unique<KDTreeNode>(indices[0], depth % 3);
    }

    // 选择分割维度
    int dimension = depth % 3;

    // 按选定维度排序
    std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        switch (dimension) {
            case 0: return m_points[a].x() < m_points[b].x();
            case 1: return m_points[a].y() < m_points[b].y();
            case 2: return m_points[a].z() < m_points[b].z();
            default: return false;
        }
    });

    // 选择中位数作为分割点
    size_t medianIndex = indices.size() / 2;
    auto node = std::make_unique<KDTreeNode>(indices[medianIndex], dimension);

    // 分割点集
    std::vector<size_t> leftIndices(indices.begin(), indices.begin() + medianIndex);
    std::vector<size_t> rightIndices(indices.begin() + medianIndex + 1, indices.end());

    // 递归构建子树
    node->left = buildKDTreeRecursive(leftIndices, depth + 1);
    node->right = buildKDTreeRecursive(rightIndices, depth + 1);

    return node;
}

void SpatialIndex::queryRadiusKDTree(const KDTreeNode* node, const QVector3D& center,
                                     float radius, std::vector<QueryResult>& results) const
{
    if (!node) {
        return;
    }

    // 检查当前节点的点
    if (node->pointIndex < m_points.size()) {
        float distance = calculateDistance(m_points[node->pointIndex], center);
        if (distance <= radius) {
            results.emplace_back(node->pointIndex, distance);
        }
    }

    // 计算到分割平面的距离
    float planeDistance = 0.0f;
    switch (node->splitDimension) {
        case 0: planeDistance = center.x() - m_points[node->pointIndex].x(); break;
        case 1: planeDistance = center.y() - m_points[node->pointIndex].y(); break;
        case 2: planeDistance = center.z() - m_points[node->pointIndex].z(); break;
    }

    // 决定先搜索哪一边
    KDTreeNode* nearSide = (planeDistance < 0) ? node->left.get() : node->right.get();
    KDTreeNode* farSide = (planeDistance < 0) ? node->right.get() : node->left.get();

    // 搜索近侧
    queryRadiusKDTree(nearSide, center, radius, results);

    // 如果查询球与分割平面相交，也搜索远侧
    if (qAbs(planeDistance) <= radius) {
        queryRadiusKDTree(farSide, center, radius, results);
    }
}

void SpatialIndex::queryKNNKDTree(const KDTreeNode* node, const QVector3D& queryPoint,
                                  int k, std::vector<QueryResult>& results) const
{
    if (!node) {
        return;
    }

    // 使用优先队列维护k个最近邻
    static std::priority_queue<QueryResult, std::vector<QueryResult>,
                              std::function<bool(const QueryResult&, const QueryResult&)>>
        maxHeap([](const QueryResult& a, const QueryResult& b) { return a.distance < b.distance; });

    std::function<void(const KDTreeNode*)> searchRecursive = [&](const KDTreeNode* currentNode) {
        if (!currentNode) {
            return;
        }

        // 计算到当前节点的距离
        if (currentNode->pointIndex < m_points.size()) {
            float distance = calculateDistance(m_points[currentNode->pointIndex], queryPoint);

            if (maxHeap.size() < k) {
                maxHeap.emplace(currentNode->pointIndex, distance);
            } else if (distance < maxHeap.top().distance) {
                maxHeap.pop();
                maxHeap.emplace(currentNode->pointIndex, distance);
            }
        }

        // 计算到分割平面的距离
        float planeDistance = 0.0f;
        switch (currentNode->splitDimension) {
            case 0: planeDistance = queryPoint.x() - m_points[currentNode->pointIndex].x(); break;
            case 1: planeDistance = queryPoint.y() - m_points[currentNode->pointIndex].y(); break;
            case 2: planeDistance = queryPoint.z() - m_points[currentNode->pointIndex].z(); break;
        }

        // 决定搜索顺序
        KDTreeNode* nearSide = (planeDistance < 0) ? currentNode->left.get() : currentNode->right.get();
        KDTreeNode* farSide = (planeDistance < 0) ? currentNode->right.get() : currentNode->left.get();

        // 搜索近侧
        searchRecursive(nearSide);

        // 如果需要，搜索远侧
        if (maxHeap.size() < k || qAbs(planeDistance) < maxHeap.top().distance) {
            searchRecursive(farSide);
        }
    };

    // 清空之前的结果
    while (!maxHeap.empty()) {
        maxHeap.pop();
    }

    // 开始搜索
    searchRecursive(node);

    // 将结果转移到输出向量
    results.clear();
    results.reserve(maxHeap.size());

    while (!maxHeap.empty()) {
        results.push_back(maxHeap.top());
        maxHeap.pop();
    }

    // 按距离排序（从近到远）
    std::sort(results.begin(), results.end(),
             [](const QueryResult& a, const QueryResult& b) {
                 return a.distance < b.distance;
             });
}

// 辅助方法实现
void SpatialIndex::emitErrorOccurred(const QString& error)
{
    emit errorOccurred(error);
}

} // namespace WallExtraction
