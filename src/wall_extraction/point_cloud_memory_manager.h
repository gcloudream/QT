#ifndef POINT_CLOUD_MEMORY_MANAGER_H
#define POINT_CLOUD_MEMORY_MANAGER_H

#include <QObject>
#include <QVector3D>
#include <QVariantMap>
#include <QDateTime>
#include <vector>
#include <memory>
#include <deque>

namespace WallExtraction {

// 点云数据块
struct PointCloudChunk {
    std::vector<QVector3D> points;      // 点云数据
    QVector3D boundingBoxMin;           // 边界框最小点
    QVector3D boundingBoxMax;           // 边界框最大点
    size_t memoryUsage;                 // 内存使用量
    int priority;                       // 优先级
    bool isLoaded;                      // 是否已加载
    qint64 lastAccessTime;              // 最后访问时间
    
    PointCloudChunk() : memoryUsage(0), priority(0), isLoaded(false), lastAccessTime(0) {}
};

// 内存管理策略
enum class MemoryStrategy {
    LRU,            // 最近最少使用
    LFU,            // 最少使用频率
    FIFO,           // 先进先出
    Priority        // 基于优先级
};

/**
 * @brief 点云内存管理器
 * 
 * 负责大数据量点云的内存管理，实现分块加载、渐进式渲染和内存优化。
 * 支持多种内存管理策略，确保系统在处理大型点云时的稳定性。
 */
class PointCloudMemoryManager : public QObject
{
    Q_OBJECT

public:
    explicit PointCloudMemoryManager(QObject* parent = nullptr);
    ~PointCloudMemoryManager();

    /**
     * @brief 检查管理器是否已初始化
     * @return 初始化状态
     */
    bool isInitialized() const;

    /**
     * @brief 设置内存管理策略
     * @param strategy 内存管理策略
     */
    void setMemoryStrategy(MemoryStrategy strategy);

    /**
     * @brief 获取内存管理策略
     * @return 当前策略
     */
    MemoryStrategy getMemoryStrategy() const;

    /**
     * @brief 设置最大内存使用量
     * @param maxMemoryMB 最大内存使用量（MB）
     */
    void setMaxMemoryUsage(size_t maxMemoryMB);

    /**
     * @brief 获取最大内存使用量
     * @return 最大内存使用量（MB）
     */
    size_t getMaxMemoryUsage() const;

    /**
     * @brief 分块加载点云数据
     * @param points 原始点云数据
     * @param chunkSize 每块的点数量
     * @return 加载是否成功
     */
    bool loadPointCloudChunked(const std::vector<QVector3D>& points, size_t chunkSize);

    /**
     * @brief 获取已加载的块数量
     * @return 块数量
     */
    size_t getLoadedChunkCount() const;

    /**
     * @brief 获取总块数量
     * @return 总块数量
     */
    size_t getTotalChunkCount() const;

    /**
     * @brief 获取指定块的点云数据
     * @param chunkIndex 块索引
     * @return 点云数据
     */
    std::vector<QVector3D> getChunkPoints(size_t chunkIndex) const;

    /**
     * @brief 获取用于渲染的点云数据
     * @param lodLevel LOD级别
     * @return 渲染用点云数据
     */
    std::vector<QVector3D> getPointsForRendering(int lodLevel) const;

    /**
     * @brief 获取视锥体内的点云数据
     * @param viewPosition 视点位置
     * @param viewDirection 视线方向
     * @param fov 视野角度
     * @param nearPlane 近平面距离
     * @param farPlane 远平面距离
     * @return 可见点云数据
     */
    std::vector<QVector3D> getVisiblePoints(const QVector3D& viewPosition,
                                           const QVector3D& viewDirection,
                                           float fov,
                                           float nearPlane,
                                           float farPlane) const;

    /**
     * @brief 预加载指定区域的点云数据
     * @param center 区域中心
     * @param radius 区域半径
     * @return 预加载是否成功
     */
    bool preloadRegion(const QVector3D& center, float radius);

    /**
     * @brief 卸载指定块的数据
     * @param chunkIndex 块索引
     * @return 卸载是否成功
     */
    bool unloadChunk(size_t chunkIndex);

    /**
     * @brief 优化内存使用
     * @return 优化是否成功
     */
    bool optimizeMemoryUsage();

    /**
     * @brief 获取当前内存使用量
     * @return 内存使用量（字节）
     */
    size_t getCurrentMemoryUsage() const;

    /**
     * @brief 获取内存使用统计
     * @return 统计信息映射
     */
    QVariantMap getMemoryStatistics() const;

    /**
     * @brief 清除所有数据
     */
    void clearAllData();

    /**
     * @brief 设置块优先级
     * @param chunkIndex 块索引
     * @param priority 优先级
     */
    void setChunkPriority(size_t chunkIndex, int priority);

    /**
     * @brief 启用/禁用自动内存管理
     * @param enabled 是否启用
     */
    void setAutoMemoryManagementEnabled(bool enabled);

    /**
     * @brief 检查自动内存管理是否启用
     * @return 是否启用
     */
    bool isAutoMemoryManagementEnabled() const;

signals:
    /**
     * @brief 内存使用量变化信号
     * @param currentUsage 当前使用量（字节）
     * @param maxUsage 最大使用量（字节）
     */
    void memoryUsageChanged(size_t currentUsage, size_t maxUsage);

    /**
     * @brief 块加载完成信号
     * @param chunkIndex 块索引
     */
    void chunkLoaded(size_t chunkIndex);

    /**
     * @brief 块卸载完成信号
     * @param chunkIndex 块索引
     */
    void chunkUnloaded(size_t chunkIndex);

    /**
     * @brief 内存警告信号
     * @param message 警告消息
     */
    void memoryWarning(const QString& message);

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
    /**
     * @brief 创建点云块
     * @param points 点云数据
     * @param startIndex 起始索引
     * @param endIndex 结束索引
     * @return 点云块
     */
    std::unique_ptr<PointCloudChunk> createChunk(const std::vector<QVector3D>& points,
                                                 size_t startIndex,
                                                 size_t endIndex);

    /**
     * @brief 计算块的边界框
     * @param points 点云数据
     * @return 边界框（最小点，最大点）
     */
    std::pair<QVector3D, QVector3D> computeChunkBoundingBox(const std::vector<QVector3D>& points) const;

    /**
     * @brief 检查块是否在视锥体内
     * @param chunk 点云块
     * @param viewPosition 视点位置
     * @param viewDirection 视线方向
     * @param fov 视野角度
     * @param nearPlane 近平面距离
     * @param farPlane 远平面距离
     * @return 是否可见
     */
    bool isChunkVisible(const PointCloudChunk& chunk,
                       const QVector3D& viewPosition,
                       const QVector3D& viewDirection,
                       float fov,
                       float nearPlane,
                       float farPlane) const;

    /**
     * @brief 根据策略选择要卸载的块
     * @param requiredMemory 需要的内存量
     * @return 要卸载的块索引列表
     */
    std::vector<size_t> selectChunksToUnload(size_t requiredMemory);

    /**
     * @brief 更新块的访问时间
     * @param chunkIndex 块索引
     */
    void updateChunkAccessTime(size_t chunkIndex);

    /**
     * @brief 检查内存使用是否超限
     * @return 是否超限
     */
    bool isMemoryLimitExceeded() const;

    /**
     * @brief 执行内存清理
     * @param targetMemory 目标内存使用量
     * @return 清理是否成功
     */
    bool performMemoryCleanup(size_t targetMemory);

    /**
     * @brief 更新内存统计
     */
    void updateMemoryStatistics();

    /**
     * 辅助方法用于从const方法中发射信号
     */
    void emitMemoryWarning(const QString& message);
    void emitChunkLoaded(size_t chunkIndex);
    void emitMemoryUsageChanged(size_t currentUsage, size_t maxUsage);

private:
    bool m_initialized;
    MemoryStrategy m_strategy;
    size_t m_maxMemoryUsage;        // 最大内存使用量（字节）
    bool m_autoMemoryManagement;
    
    std::vector<std::unique_ptr<PointCloudChunk>> m_chunks;
    std::deque<size_t> m_loadedChunks;  // 已加载块的索引队列
    
    mutable size_t m_currentMemoryUsage;
    mutable QVariantMap m_memoryStatistics;
    mutable bool m_statisticsValid;
    
    // 性能计数器
    size_t m_totalLoadOperations;
    size_t m_totalUnloadOperations;
    qint64 m_totalLoadTime;
    qint64 m_totalUnloadTime;
};

} // namespace WallExtraction

// 注册元类型
Q_DECLARE_METATYPE(WallExtraction::MemoryStrategy)
Q_DECLARE_METATYPE(WallExtraction::PointCloudChunk)

#endif // POINT_CLOUD_MEMORY_MANAGER_H
