#ifndef LAS_READER_H
#define LAS_READER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector3D>
#include <QVariantMap>
#include <vector>
#include <stdexcept>

namespace WallExtraction {

// LAS文件版本信息
struct LASVersion {
    quint8 major;
    quint8 minor;
    
    bool isValid() const { return major > 0; }
};

// 坐标系统枚举
enum class CoordinateSystem {
    Unknown,
    WGS84,
    UTM_Zone33N,
    UTM_Zone34N,
    // 可以根据需要添加更多坐标系统
};

// 坐标系统信息
struct CoordinateSystemInfo {
    CoordinateSystem type;
    QString wktString;
    int epsgCode;
    
    bool isValid() const { return type != CoordinateSystem::Unknown || !wktString.isEmpty() || epsgCode > 0; }
};

// LAS文件头信息
struct LASHeader {
    LASVersion version;
    quint32 pointCount;
    quint8 pointDataRecordFormat;
    quint16 pointDataRecordLength;
    double xScale, yScale, zScale;
    double xOffset, yOffset, zOffset;
    double xMin, xMax, yMin, yMax, zMin, zMax;
    CoordinateSystemInfo coordinateSystem;
    
    bool isValid() const { return version.isValid() && pointCount > 0; }
};

// 带属性的点云数据
struct PointWithAttributes {
    QVector3D position;
    QVariantMap attributes;
};

// LAS读取器异常类
class LASReaderException : public std::exception {
public:
    explicit LASReaderException(const QString& message);
    const char* what() const noexcept override;
    QString getDetailedMessage() const;

private:
    QString m_message;
    QString m_detailedMessage;
};

/**
 * @brief LAS/LAZ格式点云文件读取器
 * 
 * 支持LAS 1.2, 1.3, 1.4版本
 * 支持LAZ压缩格式
 * 支持坐标系统转换
 * 支持属性信息解析（分类、强度、RGB等）
 */
class LASReader : public QObject
{
    Q_OBJECT

public:
    explicit LASReader(QObject* parent = nullptr);
    ~LASReader();

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
     * @return 文件格式（"las" 或 "laz"）
     */
    QString detectFormat(const QString& filename) const;

    /**
     * @brief 检查是否支持指定LAS版本
     * @param major 主版本号
     * @param minor 次版本号
     * @return 是否支持
     */
    bool supportsVersion(quint8 major, quint8 minor) const;

    /**
     * @brief 检查是否支持指定点记录格式
     * @param format 点记录格式
     * @return 是否支持
     */
    bool supportsPointRecordFormat(quint8 format) const;

    /**
     * @brief 检查是否支持WKT坐标系统
     * @return 是否支持
     */
    bool supportsWKT() const;

    /**
     * @brief 检查是否支持UTM坐标系统
     * @return 是否支持
     */
    bool supportsUTM() const;

    /**
     * @brief 解析LAS文件头
     * @param filename 文件路径
     * @return LAS文件头信息
     * @throws LASReaderException
     */
    LASHeader parseHeader(const QString& filename) const;

    /**
     * @brief 解析坐标系统信息
     * @param filename 文件路径
     * @return 坐标系统信息
     * @throws LASReaderException
     */
    CoordinateSystemInfo parseCoordinateSystem(const QString& filename) const;

    /**
     * @brief 读取点云数据（仅坐标）
     * @param filename 文件路径
     * @return 点云坐标数组
     * @throws LASReaderException
     */
    std::vector<QVector3D> readPointCloud(const QString& filename) const;

    /**
     * @brief 读取点云数据（包含属性）
     * @param filename 文件路径
     * @return 带属性的点云数据
     * @throws LASReaderException
     */
    std::vector<PointWithAttributes> readPointCloudWithAttributes(const QString& filename) const;

    /**
     * @brief 获取文件中可用的属性列表
     * @param filename 文件路径
     * @return 属性名称列表
     * @throws LASReaderException
     */
    QStringList getAvailableAttributes(const QString& filename) const;

    /**
     * @brief 坐标系统转换
     * @param point 原始点坐标
     * @param sourceSystem 源坐标系统
     * @param targetSystem 目标坐标系统
     * @return 转换后的坐标
     * @throws LASReaderException
     */
    QVector3D transformCoordinates(const QVector3D& point,
                                   CoordinateSystem sourceSystem,
                                   CoordinateSystem targetSystem) const;

    /**
     * @brief 批量坐标系统转换
     * @param points 原始点坐标数组
     * @param sourceSystem 源坐标系统
     * @param targetSystem 目标坐标系统
     * @return 转换后的坐标数组
     * @throws LASReaderException
     */
    std::vector<QVector3D> transformCoordinates(const std::vector<QVector3D>& points,
                                                CoordinateSystem sourceSystem,
                                                CoordinateSystem targetSystem) const;

signals:
    /**
     * @brief 读取进度信号
     * @param percentage 进度百分比 (0-100)
     */
    void readProgress(int percentage);

    /**
     * @brief 错误发生信号
     * @param error 错误消息
     */
    void errorOccurred(const QString& error);

private:
    /**
     * @brief 验证LAS文件签名
     * @param filename 文件路径
     * @return 是否为有效的LAS文件
     */
    bool validateLASSignature(const QString& filename) const;

    /**
     * @brief 读取LAS文件头
     * @param filename 文件路径
     * @return 文件头数据
     * @throws LASReaderException
     */
    QByteArray readLASHeader(const QString& filename) const;

    /**
     * @brief 解析点数据记录
     * @param data 点数据
     * @param format 点记录格式
     * @param header LAS文件头
     * @return 解析后的点信息
     */
    PointWithAttributes parsePointRecord(const QByteArray& data, 
                                        quint8 format, 
                                        const LASHeader& header) const;

    /**
     * @brief 应用坐标缩放和偏移
     * @param rawCoord 原始坐标值
     * @param scale 缩放因子
     * @param offset 偏移量
     * @return 实际坐标值
     */
    double applyScaleAndOffset(qint32 rawCoord, double scale, double offset) const;

    /**
     * @brief 解析WKT坐标系统字符串
     * @param wktString WKT字符串
     * @return 坐标系统信息
     */
    CoordinateSystemInfo parseWKTString(const QString& wktString) const;

    /**
     * @brief 检查是否为LAZ压缩文件
     * @param filename 文件路径
     * @return 是否为LAZ文件
     */
    bool isLAZFile(const QString& filename) const;

    /**
     * @brief 解压LAZ文件数据
     * @param compressedData 压缩数据
     * @return 解压后的数据
     * @throws LASReaderException
     */
    QByteArray decompressLAZData(const QByteArray& compressedData) const;

    /**
     * @brief 辅助方法用于从const方法中发射信号
     */
    void emitReadProgress(int percentage);

private:
    // 支持的LAS版本
    QList<LASVersion> m_supportedVersions;
    
    // 支持的点记录格式
    QList<quint8> m_supportedPointFormats;
    
    // 缓存的文件头信息
    mutable QHash<QString, LASHeader> m_headerCache;
};

} // namespace WallExtraction

// 注册元类型
Q_DECLARE_METATYPE(WallExtraction::CoordinateSystem)
Q_DECLARE_METATYPE(WallExtraction::LASVersion)
Q_DECLARE_METATYPE(WallExtraction::LASHeader)

#endif // LAS_READER_H
