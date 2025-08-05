#ifndef POINT_CLOUD_LOD_MANAGER_H
#define POINT_CLOUD_LOD_MANAGER_H

#include <QObject>
#include <QVector3D>
#include <vector>
#include <memory>

namespace WallExtraction {

// LOD级别信息
struct LODLevel {
    int level;                          // LOD级别 (0=最高质量)
    std::vector<QVector3D> points;      // 该级别的点云数据
    float reductionRatio;               // 相对于原始数据的缩减比例
    float minDistance;                  // 适用的最小距离
    float maxDistance;                  // 适用的最大距离
    size_t memoryUsage;                 // 内存使用量
    
    bool isValid() const { return !points.empty() && level >= 0; }
};

// LOD生成策略
enum class LODStrategy {
    UniformDownsampling,    // 均匀下采样
    VoxelGridDownsampling,  // 体素网格下采样
    RandomSampling,         // 随机采样
    ImportanceBasedSampling // 基于重要性的采样
};

/**
 * @brief 点云LOD（Level of Detail）管理器
 * 
 * 负责生成和管理多级细节的点云数据，根据视距自动选择合适的LOD级别，
 * 以优化渲染性能和内存使用。
 */
class PointCloudLODManager : public QObject
{
    Q_OBJECT

public:
    explicit PointCloudLODManager(QObject* parent = nullptr);
    ~PointCloudLODManager();

    /**
     * @brief 检查管理器是否已初始化
     * @return 初始化状态
     */
    bool isInitialized() const;

    /**
     * @brief 设置LOD生成策略
     * @param strategy LOD生成策略
     */
    void setLODStrategy(LODStrategy strategy);

    /**
     * @brief 获取当前LOD生成策略
     * @return 当前策略
     */
    LODStrategy getLODStrategy() const;

    /**
     * @brief 设置LOD级别数量
     * @param levelCount LOD级别数量
     */
    void setLODLevelCount(int levelCount);

    /**
     * @brief 获取LOD级别数量
     * @return LOD级别数量
     */
    int getLODLevelCount() const;

    /**
     * @brief 生成LOD级别
     * @param originalPoints 原始点云数据
     * @return 生成是否成功
     */
    bool generateLODLevels(const std::vector<QVector3D>& originalPoints);

    /**
     * @brief 根据距离选择合适的LOD级别
     * @param distance 视点到点云的距离
     * @return 选择的LOD级别
     */
    int selectLODLevel(float distance) const;

    /**
     * @brief 根据视锥体选择LOD级别
     * @param viewPosition 视点位置
     * @param viewDirection 视线方向
     * @param fov 视野角度
     * @return 选择的LOD级别
     */
    int selectLODLevel(const QVector3D& viewPosition, 
                      const QVector3D& viewDirection, 
                      float fov) const;

    /**
     * @brief 获取指定LOD级别的点云数据
     * @param level LOD级别
     * @return 点云数据
     */
    std::vector<QVector3D> getLODPoints(int level) const;

    /**
     * @brief 获取LOD级别信息
     * @param level LOD级别
     * @return LOD级别信息
     */
    LODLevel getLODLevelInfo(int level) const;

    /**
     * @brief 获取所有LOD级别信息
     * @return LOD级别信息列表
     */
    std::vector<LODLevel> getAllLODLevels() const;

    /**
     * @brief 清除所有LOD数据
     */
    void clearLODData();

    /**
     * @brief 获取总内存使用量
     * @return 内存使用量（字节）
     */
    size_t getTotalMemoryUsage() const;

    /**
     * @brief 获取原始点云数量
     * @return 原始点数量
     */
    size_t getOriginalPointCount() const;

    /**
     * @brief 设置距离阈值
     * @param thresholds 距离阈值数组
     */
    void setDistanceThresholds(const std::vector<float>& thresholds);

    /**
     * @brief 获取距离阈值
     * @return 距离阈值数组
     */
    std::vector<float> getDistanceThresholds() const;

    /**
     * @brief 启用/禁用自适应LOD
     * @param enabled 是否启用
     */
    void setAdaptiveLODEnabled(bool enabled);

    /**
     * @brief 检查自适应LOD是否启用
     * @return 是否启用
     */
    bool isAdaptiveLODEnabled() const;

signals:
    /**
     * @brief LOD生成进度信号
     * @param percentage 进度百分比 (0-100)
     */
    void lodGenerationProgress(int percentage);

    /**
     * @brief LOD级别切换信号
     * @param oldLevel 旧级别
     * @param newLevel 新级别
     */
    void lodLevelChanged(int oldLevel, int newLevel);

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
     * @brief 使用均匀下采样生成LOD
     * @param points 原始点云
     * @param reductionRatio 缩减比例
     * @return 下采样后的点云
     */
    std::vector<QVector3D> generateUniformDownsampling(const std::vector<QVector3D>& points,
                                                       float reductionRatio) const;

    /**
     * @brief 使用体素网格下采样生成LOD
     * @param points 原始点云
     * @param voxelSize 体素大小
     * @return 下采样后的点云
     */
    std::vector<QVector3D> generateVoxelGridDownsampling(const std::vector<QVector3D>& points,
                                                         float voxelSize) const;

    /**
     * @brief 使用随机采样生成LOD
     * @param points 原始点云
     * @param reductionRatio 缩减比例
     * @return 下采样后的点云
     */
    std::vector<QVector3D> generateRandomSampling(const std::vector<QVector3D>& points,
                                                  float reductionRatio) const;

    /**
     * @brief 使用基于重要性的采样生成LOD
     * @param points 原始点云
     * @param reductionRatio 缩减比例
     * @return 下采样后的点云
     */
    std::vector<QVector3D> generateImportanceBasedSampling(const std::vector<QVector3D>& points,
                                                           float reductionRatio) const;

    /**
     * @brief 计算点的重要性分数
     * @param points 点云数据
     * @param index 点索引
     * @return 重要性分数
     */
    float calculateImportanceScore(const std::vector<QVector3D>& points, size_t index) const;

    /**
     * @brief 计算点云边界框
     * @param points 点云数据
     * @return 边界框（最小点，最大点）
     */
    std::pair<QVector3D, QVector3D> computeBoundingBox(const std::vector<QVector3D>& points) const;

    /**
     * @brief 计算默认距离阈值
     * @param boundingBox 点云边界框
     * @return 距离阈值数组
     */
    std::vector<float> computeDefaultDistanceThresholds(const std::pair<QVector3D, QVector3D>& boundingBox) const;

    /**
     * @brief 验证LOD数据完整性
     * @return 验证结果
     */
    bool validateLODData() const;

private:
    bool m_initialized;
    LODStrategy m_strategy;
    int m_levelCount;
    bool m_adaptiveLODEnabled;
    
    std::vector<LODLevel> m_lodLevels;
    std::vector<float> m_distanceThresholds;
    size_t m_originalPointCount;
    
    int m_currentLODLevel;
    
    // 性能统计
    mutable size_t m_totalMemoryUsage;
    mutable qint64 m_lastGenerationTime;
};

} // namespace WallExtraction

// 注册元类型
Q_DECLARE_METATYPE(WallExtraction::LODStrategy)
Q_DECLARE_METATYPE(WallExtraction::LODLevel)

#endif // POINT_CLOUD_LOD_MANAGER_H
