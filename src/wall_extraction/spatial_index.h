#ifndef SPATIAL_INDEX_H
#define SPATIAL_INDEX_H

#include <QObject>
#include <QVector3D>
#include <QVariantMap>
#include <vector>
#include <memory>

namespace WallExtraction {

// 空间索引类型
enum class SpatialIndexType {
    Octree,     // 八叉树
    KDTree      // KD树
};

// 八叉树节点
struct OctreeNode {
    QVector3D center;           // 节点中心
    float halfSize;             // 节点半径
    std::vector<size_t> pointIndices;  // 点索引
    std::array<std::unique_ptr<OctreeNode>, 8> children;  // 8个子节点
    bool isLeaf;                // 是否为叶节点
    
    OctreeNode(const QVector3D& c, float hs) 
        : center(c), halfSize(hs), isLeaf(true) {}
};

// KD树节点
struct KDTreeNode {
    size_t pointIndex;          // 点索引
    int splitDimension;         // 分割维度 (0=x, 1=y, 2=z)
    std::unique_ptr<KDTreeNode> left;   // 左子树
    std::unique_ptr<KDTreeNode> right;  // 右子树
    
    KDTreeNode(size_t idx, int dim) 
        : pointIndex(idx), splitDimension(dim) {}
};

// 查询结果
struct QueryResult {
    size_t pointIndex;          // 点索引
    float distance;             // 距离

    QueryResult() : pointIndex(0), distance(0.0f) {}
    QueryResult(size_t idx, float dist) : pointIndex(idx), distance(dist) {}
};

/**
 * @brief 空间索引类
 * 
 * 提供高效的空间查询功能，支持八叉树和KD树两种索引结构。
 * 用于加速邻域搜索、范围查询和K近邻查询。
 */
class SpatialIndex : public QObject
{
    Q_OBJECT

public:
    explicit SpatialIndex(QObject* parent = nullptr);
    ~SpatialIndex();

    /**
     * @brief 设置索引类型
     * @param type 索引类型
     */
    void setIndexType(SpatialIndexType type);

    /**
     * @brief 获取索引类型
     * @return 当前索引类型
     */
    SpatialIndexType getIndexType() const;

    /**
     * @brief 检查是否支持KD树
     * @return 是否支持
     */
    bool supportsKDTree() const;

    /**
     * @brief 构建空间索引
     * @param points 点云数据
     * @return 构建是否成功
     */
    bool buildIndex(const std::vector<QVector3D>& points);

    /**
     * @brief 插入新点
     * @param point 新点坐标
     * @return 插入是否成功
     */
    bool insertPoint(const QVector3D& point);

    /**
     * @brief 删除点
     * @param pointIndex 点索引
     * @return 删除是否成功
     */
    bool removePoint(size_t pointIndex);

    /**
     * @brief 范围查询
     * @param center 查询中心
     * @param radius 查询半径
     * @return 查询结果
     */
    std::vector<QueryResult> queryRadius(const QVector3D& center, float radius) const;

    /**
     * @brief K近邻查询
     * @param queryPoint 查询点
     * @param k 邻居数量
     * @return 查询结果
     */
    std::vector<QueryResult> queryKNN(const QVector3D& queryPoint, int k) const;

    /**
     * @brief 边界框查询
     * @param minPoint 边界框最小点
     * @param maxPoint 边界框最大点
     * @return 查询结果
     */
    std::vector<QueryResult> queryBoundingBox(const QVector3D& minPoint, 
                                             const QVector3D& maxPoint) const;

    /**
     * @brief 获取索引统计信息
     * @return 统计信息映射
     */
    QVariantMap getIndexStatistics() const;

    /**
     * @brief 清除索引
     */
    void clearIndex();

    /**
     * @brief 检查索引是否已构建
     * @return 是否已构建
     */
    bool isIndexBuilt() const;

    /**
     * @brief 获取点数量
     * @return 点数量
     */
    size_t getPointCount() const;

    /**
     * @brief 设置最大叶节点容量
     * @param capacity 容量
     */
    void setMaxLeafCapacity(int capacity);

    /**
     * @brief 获取最大叶节点容量
     * @return 容量
     */
    int getMaxLeafCapacity() const;

    /**
     * @brief 设置最大树深度
     * @param depth 深度
     */
    void setMaxTreeDepth(int depth);

    /**
     * @brief 获取最大树深度
     * @return 深度
     */
    int getMaxTreeDepth() const;

signals:
    /**
     * @brief 索引构建进度信号
     * @param percentage 进度百分比 (0-100)
     */
    void indexBuildProgress(int percentage);

    /**
     * @brief 状态消息信号
     * @param message 状态消息
     */
    void statusMessage(const QString& message);

    /**
     * @brief 错误发生信号
     * @param error 错误消息
     */
    void errorOccurred(const QString& error);

private:
    // 八叉树相关方法
    bool buildOctree(const std::vector<QVector3D>& points);
    void insertPointOctree(const QVector3D& point, size_t pointIndex);
    std::unique_ptr<OctreeNode> createOctreeNode(const QVector3D& center, float halfSize);
    void subdivideOctreeNode(OctreeNode* node);
    int getOctant(const QVector3D& point, const QVector3D& center) const;
    QVector3D getChildCenter(const QVector3D& parentCenter, float parentHalfSize, int octant) const;
    
    void queryRadiusOctree(const OctreeNode* node, const QVector3D& center, 
                          float radius, std::vector<QueryResult>& results) const;
    void queryBoundingBoxOctree(const OctreeNode* node, const QVector3D& minPoint, 
                               const QVector3D& maxPoint, std::vector<QueryResult>& results) const;
    
    // KD树相关方法
    bool buildKDTree(const std::vector<QVector3D>& points);
    std::unique_ptr<KDTreeNode> buildKDTreeRecursive(std::vector<size_t>& indices, int depth);
    void queryRadiusKDTree(const KDTreeNode* node, const QVector3D& center, 
                          float radius, std::vector<QueryResult>& results) const;
    void queryKNNKDTree(const KDTreeNode* node, const QVector3D& queryPoint, 
                       int k, std::vector<QueryResult>& results) const;
    
    // 辅助方法
    float calculateDistance(const QVector3D& p1, const QVector3D& p2) const;
    bool isPointInRadius(const QVector3D& point, const QVector3D& center, float radius) const;
    bool isPointInBoundingBox(const QVector3D& point, const QVector3D& minPoint, 
                             const QVector3D& maxPoint) const;
    std::pair<QVector3D, QVector3D> computeBoundingBox(const std::vector<QVector3D>& points) const;
    
    void updateStatistics();

private:
    SpatialIndexType m_indexType;
    bool m_indexBuilt;
    
    // 点云数据
    std::vector<QVector3D> m_points;
    
    // 八叉树根节点
    std::unique_ptr<OctreeNode> m_octreeRoot;
    
    // KD树根节点
    std::unique_ptr<KDTreeNode> m_kdtreeRoot;
    
    // 参数设置
    int m_maxLeafCapacity;      // 叶节点最大容量
    int m_maxTreeDepth;         // 最大树深度
    
    // 边界框
    QVector3D m_boundingBoxMin;
    QVector3D m_boundingBoxMax;
    
    // 统计信息
    mutable QVariantMap m_statistics;
    mutable bool m_statisticsValid;

    // 辅助方法用于从const方法中发射信号
    void emitErrorOccurred(const QString& error);
};

} // namespace WallExtraction

// 注册元类型
Q_DECLARE_METATYPE(WallExtraction::SpatialIndexType)
Q_DECLARE_METATYPE(WallExtraction::QueryResult)

#endif // SPATIAL_INDEX_H
