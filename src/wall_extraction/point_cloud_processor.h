#ifndef POINT_CLOUD_PROCESSOR_H
#define POINT_CLOUD_PROCESSOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector3D>
#include <QVariantMap>
#include <vector>
#include <memory>
#include "las_reader.h"

// 前向声明
class PCDReader;

namespace WallExtraction {

// 点云格式枚举
enum class PointCloudFormat {
    Unknown,
    PCD_ASCII,
    PCD_Binary,
    PCD_BinaryCompressed,
    PLY_ASCII,
    PLY_Binary,
    LAS,
    LAZ,
    XYZ,
    TXT
};

// 点云属性信息
struct PointCloudAttributes {
    bool hasIntensity = false;
    bool hasClassification = false;
    bool hasRGB = false;
    bool hasNormals = false;
    QStringList customAttributes;
};

// 点云元数据
struct PointCloudMetadata {
    PointCloudFormat format;
    quint32 pointCount;
    PointCloudAttributes attributes;
    CoordinateSystemInfo coordinateSystem;
    QVector3D boundingBoxMin;
    QVector3D boundingBoxMax;
    QString originalFilename;
    qint64 fileSize;
    
    bool isValid() const { return format != PointCloudFormat::Unknown && pointCount > 0; }
};

// 点云处理异常类
class PointCloudProcessorException : public std::exception {
public:
    explicit PointCloudProcessorException(const QString& message);
    const char* what() const noexcept override;
    QString getDetailedMessage() const;

private:
    QString m_message;
    QString m_detailedMessage;
};

/**
 * @brief 统一的点云处理器类
 * 
 * 支持多种点云格式的读取、处理和转换
 * 集成LAS/LAZ和PCD格式支持
 * 提供坐标系统转换和属性处理功能
 */
class PointCloudProcessor : public QObject
{
    Q_OBJECT

public:
    explicit PointCloudProcessor(QObject* parent = nullptr);
    ~PointCloudProcessor();

    /**
     * @brief 获取支持的文件格式
     * @return 支持的格式列表
     */
    QStringList getSupportedFormats() const;

    /**
     * @brief 检查是否可以读取指定文件
     * @param filename 文件路径
     * @return 是否可以读取
     */
    bool canReadFile(const QString& filename) const;

    /**
     * @brief 检测文件格式
     * @param filename 文件路径
     * @return 检测到的格式
     */
    PointCloudFormat detectFormat(const QString& filename) const;

    /**
     * @brief 获取点云元数据
     * @param filename 文件路径
     * @return 点云元数据
     * @throws PointCloudProcessorException
     */
    PointCloudMetadata getMetadata(const QString& filename) const;

    /**
     * @brief 读取点云数据（仅坐标）
     * @param filename 文件路径
     * @return 点云坐标数组
     * @throws PointCloudProcessorException
     */
    std::vector<QVector3D> readPointCloud(const QString& filename) const;

    /**
     * @brief 读取点云数据（包含属性）
     * @param filename 文件路径
     * @return 带属性的点云数据
     * @throws PointCloudProcessorException
     */
    std::vector<PointWithAttributes> readPointCloudWithAttributes(const QString& filename) const;

    /**
     * @brief 预处理点云数据
     * @param points 原始点云数据
     * @param removeOutliers 是否移除离群点
     * @param downsample 是否进行下采样
     * @param voxelSize 体素大小（用于下采样）
     * @return 预处理后的点云数据
     */
    std::vector<QVector3D> preprocessPointCloud(const std::vector<QVector3D>& points,
                                               bool removeOutliers = true,
                                               bool downsample = false,
                                               float voxelSize = 0.1f) const;

    /**
     * @brief 计算点云边界框
     * @param points 点云数据
     * @return 边界框（最小点，最大点）
     */
    std::pair<QVector3D, QVector3D> computeBoundingBox(const std::vector<QVector3D>& points) const;

    /**
     * @brief 点云去噪
     * @param points 原始点云数据
     * @param neighborCount 邻居点数量阈值
     * @param stdDevThreshold 标准差阈值
     * @return 去噪后的点云数据
     */
    std::vector<QVector3D> removeOutliers(const std::vector<QVector3D>& points,
                                         int neighborCount = 20,
                                         float stdDevThreshold = 2.0f) const;

    /**
     * @brief 点云下采样
     * @param points 原始点云数据
     * @param voxelSize 体素大小
     * @return 下采样后的点云数据
     */
    std::vector<QVector3D> downsamplePointCloud(const std::vector<QVector3D>& points,
                                               float voxelSize) const;

    /**
     * @brief 点云滤波（基于高度）
     * @param points 原始点云数据
     * @param minHeight 最小高度
     * @param maxHeight 最大高度
     * @return 滤波后的点云数据
     */
    std::vector<QVector3D> filterByHeight(const std::vector<QVector3D>& points,
                                         float minHeight,
                                         float maxHeight) const;

    /**
     * @brief 分离地面点
     * @param points 原始点云数据
     * @param groundThreshold 地面阈值
     * @return 地面点和非地面点的分离结果
     */
    std::pair<std::vector<QVector3D>, std::vector<QVector3D>> 
    separateGroundPoints(const std::vector<QVector3D>& points,
                        float groundThreshold = 0.1f) const;

    /**
     * @brief 坐标系统转换
     * @param points 原始点云数据
     * @param sourceSystem 源坐标系统
     * @param targetSystem 目标坐标系统
     * @return 转换后的点云数据
     * @throws PointCloudProcessorException
     */
    std::vector<QVector3D> transformCoordinates(const std::vector<QVector3D>& points,
                                               CoordinateSystem sourceSystem,
                                               CoordinateSystem targetSystem) const;

    /**
     * @brief 设置处理参数
     * @param parameters 参数映射
     */
    void setProcessingParameters(const QVariantMap& parameters);

    /**
     * @brief 获取处理参数
     * @return 当前参数映射
     */
    QVariantMap getProcessingParameters() const;

signals:
    /**
     * @brief 处理进度信号
     * @param percentage 进度百分比 (0-100)
     */
    void processingProgress(int percentage);

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
     * @brief 读取PCD格式文件
     * @param filename 文件路径
     * @return 点云数据
     */
    std::vector<QVector3D> readPCDFile(const QString& filename) const;

    /**
     * @brief 读取PLY格式文件
     * @param filename 文件路径
     * @return 点云数据
     */
    std::vector<QVector3D> readPLYFile(const QString& filename) const;

    /**
     * @brief 读取XYZ格式文件
     * @param filename 文件路径
     * @return 点云数据
     */
    std::vector<QVector3D> readXYZFile(const QString& filename) const;

    /**
     * @brief 读取TXT格式文件
     * @param filename 文件路径
     * @return 点云数据
     */
    std::vector<QVector3D> readTXTFile(const QString& filename) const;

    /**
     * @brief 计算点到点的距离
     * @param p1 点1
     * @param p2 点2
     * @return 距离
     */
    float calculateDistance(const QVector3D& p1, const QVector3D& p2) const;

    /**
     * @brief 查找K近邻
     * @param points 点云数据
     * @param queryPoint 查询点
     * @param k 邻居数量
     * @return 邻居点索引
     */
    std::vector<int> findKNearestNeighbors(const std::vector<QVector3D>& points,
                                          const QVector3D& queryPoint,
                                          int k) const;

    /**
     * @brief 计算点云统计信息
     * @param points 点云数据
     * @return 统计信息（均值、标准差等）
     */
    QVariantMap computeStatistics(const std::vector<QVector3D>& points) const;

private:
    // LAS读取器
    std::unique_ptr<LASReader> m_lasReader;
    
    // 处理参数
    QVariantMap m_processingParameters;
    
    // 缓存的元数据
    mutable QHash<QString, PointCloudMetadata> m_metadataCache;

    // 辅助方法用于从const方法中发射信号
    void emitStatusMessage(const QString& message) const;
    void emitProcessingProgress(int percentage) const;
};

} // namespace WallExtraction

// 注册元类型
Q_DECLARE_METATYPE(WallExtraction::PointCloudFormat)
Q_DECLARE_METATYPE(WallExtraction::PointCloudMetadata)

#endif // POINT_CLOUD_PROCESSOR_H
