#include "point_cloud_processor.h"
#include "../../pcdreader.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QtMath>
#include <algorithm>
#include <unordered_map>

namespace WallExtraction {

// PointCloudProcessorException 实现
PointCloudProcessorException::PointCloudProcessorException(const QString& message)
    : m_message(message)
    , m_detailedMessage(QString("PointCloudProcessorException: %1").arg(message))
{
}

const char* PointCloudProcessorException::what() const noexcept
{
    return m_message.toLocal8Bit().constData();
}

QString PointCloudProcessorException::getDetailedMessage() const
{
    return m_detailedMessage;
}

// PointCloudProcessor 实现
PointCloudProcessor::PointCloudProcessor(QObject* parent)
    : QObject(parent)
    , m_lasReader(std::make_unique<LASReader>(this))
{
    // 注册元类型
    qRegisterMetaType<PointCloudFormat>("PointCloudFormat");
    qRegisterMetaType<PointCloudMetadata>("PointCloudMetadata");
    
    // 设置默认处理参数
    m_processingParameters["outlier_removal_neighbors"] = 20;
    m_processingParameters["outlier_removal_std_dev"] = 2.0;
    m_processingParameters["downsample_voxel_size"] = 0.1;
    m_processingParameters["ground_threshold"] = 0.1;
    
    // 连接LAS读取器信号
    connect(m_lasReader.get(), &LASReader::readProgress,
            this, &PointCloudProcessor::processingProgress);
    connect(m_lasReader.get(), &LASReader::errorOccurred,
            this, &PointCloudProcessor::errorOccurred);
    
    qDebug() << "PointCloudProcessor created with support for multiple formats";
}

PointCloudProcessor::~PointCloudProcessor()
{
    qDebug() << "PointCloudProcessor destroyed";
}

QStringList PointCloudProcessor::getSupportedFormats() const
{
    QStringList formats;
    formats << "pcd" << "ply" << "xyz" << "txt";
    formats << m_lasReader->getSupportedFormats(); // 添加LAS/LAZ支持
    return formats;
}

bool PointCloudProcessor::canReadFile(const QString& filename) const
{
    if (!QFile::exists(filename)) {
        return false;
    }
    
    QFileInfo fileInfo(filename);
    QString suffix = fileInfo.suffix().toLower();
    
    // 检查支持的格式
    if (getSupportedFormats().contains(suffix)) {
        // 对于LAS/LAZ文件，使用专门的检查
        if (suffix == "las" || suffix == "laz") {
            return m_lasReader->canReadFile(filename);
        }
        return true;
    }
    
    return false;
}

PointCloudFormat PointCloudProcessor::detectFormat(const QString& filename) const
{
    if (!canReadFile(filename)) {
        return PointCloudFormat::Unknown;
    }
    
    QFileInfo fileInfo(filename);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "pcd") {
        // 检测PCD子格式
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            QString content = stream.readAll();
            if (content.contains("DATA ascii")) {
                return PointCloudFormat::PCD_ASCII;
            } else if (content.contains("DATA binary_compressed")) {
                return PointCloudFormat::PCD_BinaryCompressed;
            } else if (content.contains("DATA binary")) {
                return PointCloudFormat::PCD_Binary;
            }
        }
        return PointCloudFormat::PCD_ASCII; // 默认
    } else if (suffix == "ply") {
        // 检测PLY子格式
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            QString firstLine = stream.readLine();
            if (firstLine.contains("format ascii")) {
                return PointCloudFormat::PLY_ASCII;
            } else if (firstLine.contains("format binary")) {
                return PointCloudFormat::PLY_Binary;
            }
        }
        return PointCloudFormat::PLY_ASCII; // 默认
    } else if (suffix == "las") {
        return PointCloudFormat::LAS;
    } else if (suffix == "laz") {
        return PointCloudFormat::LAZ;
    } else if (suffix == "xyz") {
        return PointCloudFormat::XYZ;
    } else if (suffix == "txt") {
        return PointCloudFormat::TXT;
    }
    
    return PointCloudFormat::Unknown;
}

PointCloudMetadata PointCloudProcessor::getMetadata(const QString& filename) const
{
    // 检查缓存
    if (m_metadataCache.contains(filename)) {
        return m_metadataCache[filename];
    }
    
    PointCloudMetadata metadata;
    metadata.format = detectFormat(filename);
    metadata.originalFilename = filename;
    
    QFileInfo fileInfo(filename);
    metadata.fileSize = fileInfo.size();
    
    try {
        if (metadata.format == PointCloudFormat::LAS || metadata.format == PointCloudFormat::LAZ) {
            // 使用LAS读取器获取元数据
            LASHeader header = m_lasReader->parseHeader(filename);
            metadata.pointCount = header.pointCount;
            metadata.coordinateSystem = header.coordinateSystem;
            
            // 设置边界框
            metadata.boundingBoxMin = QVector3D(header.xMin, header.yMin, header.zMin);
            metadata.boundingBoxMax = QVector3D(header.xMax, header.yMax, header.zMax);
            
            // 设置属性信息
            QStringList attributes = m_lasReader->getAvailableAttributes(filename);
            metadata.attributes.hasIntensity = attributes.contains("intensity");
            metadata.attributes.hasClassification = attributes.contains("classification");
            metadata.attributes.hasRGB = attributes.contains("red");
            
        } else {
            // 对于其他格式，读取文件获取基本信息
            auto points = readPointCloud(filename);
            metadata.pointCount = points.size();
            
            if (!points.empty()) {
                auto bbox = computeBoundingBox(points);
                metadata.boundingBoxMin = bbox.first;
                metadata.boundingBoxMax = bbox.second;
            }
            
            // 默认坐标系统
            metadata.coordinateSystem.type = CoordinateSystem::Unknown;
        }
        
        // 缓存元数据
        m_metadataCache[filename] = metadata;
        
    } catch (const std::exception& e) {
        throw PointCloudProcessorException(QString("Failed to get metadata: %1").arg(e.what()));
    }
    
    return metadata;
}

std::vector<QVector3D> PointCloudProcessor::readPointCloud(const QString& filename) const
{
    QElapsedTimer timer;
    timer.start();
    
    emitStatusMessage(QString("Reading point cloud: %1").arg(QFileInfo(filename).fileName()));
    
    PointCloudFormat format = detectFormat(filename);
    std::vector<QVector3D> points;
    
    try {
        switch (format) {
            case PointCloudFormat::LAS:
            case PointCloudFormat::LAZ:
                points = m_lasReader->readPointCloud(filename);
                break;
                
            case PointCloudFormat::PCD_ASCII:
            case PointCloudFormat::PCD_Binary:
            case PointCloudFormat::PCD_BinaryCompressed:
                points = readPCDFile(filename);
                break;
                
            case PointCloudFormat::PLY_ASCII:
            case PointCloudFormat::PLY_Binary:
                points = readPLYFile(filename);
                break;
                
            case PointCloudFormat::XYZ:
                points = readXYZFile(filename);
                break;
                
            case PointCloudFormat::TXT:
                points = readTXTFile(filename);
                break;
                
            default:
                throw PointCloudProcessorException(QString("Unsupported format: %1").arg(filename));
        }
        
        qint64 elapsed = timer.elapsed();
        emitStatusMessage(QString("Loaded %1 points in %2 ms").arg(points.size()).arg(elapsed));
        
    } catch (const std::exception& e) {
        throw PointCloudProcessorException(QString("Failed to read point cloud: %1").arg(e.what()));
    }
    
    return points;
}

std::vector<PointWithAttributes> PointCloudProcessor::readPointCloudWithAttributes(const QString& filename) const
{
    PointCloudFormat format = detectFormat(filename);
    
    if (format == PointCloudFormat::LAS || format == PointCloudFormat::LAZ) {
        return m_lasReader->readPointCloudWithAttributes(filename);
    } else {
        // 对于其他格式，转换为带属性的格式
        auto points = readPointCloud(filename);
        std::vector<PointWithAttributes> pointsWithAttribs;
        pointsWithAttribs.reserve(points.size());
        
        for (const auto& point : points) {
            PointWithAttributes pointWithAttrib;
            pointWithAttrib.position = point;
            // 其他格式暂时没有额外属性
            pointsWithAttribs.push_back(pointWithAttrib);
        }
        
        return pointsWithAttribs;
    }
}

std::vector<QVector3D> PointCloudProcessor::preprocessPointCloud(const std::vector<QVector3D>& points,
                                                                bool removeOutliers,
                                                                bool downsample,
                                                                float voxelSize) const
{
    emitStatusMessage("Preprocessing point cloud...");
    
    std::vector<QVector3D> processedPoints = points;
    
    // 去除离群点
    if (removeOutliers) {
        emitStatusMessage("Removing outliers...");
        int neighborCount = m_processingParameters["outlier_removal_neighbors"].toInt();
        float stdDevThreshold = m_processingParameters["outlier_removal_std_dev"].toFloat();
        processedPoints = this->removeOutliers(processedPoints, neighborCount, stdDevThreshold);
        emitStatusMessage(QString("Removed %1 outliers").arg(points.size() - processedPoints.size()));
    }
    
    // 下采样
    if (downsample) {
        emitStatusMessage("Downsampling point cloud...");
        processedPoints = downsamplePointCloud(processedPoints, voxelSize);
        emitStatusMessage(QString("Downsampled to %1 points").arg(processedPoints.size()));
    }
    
    emitStatusMessage("Preprocessing completed");
    return processedPoints;
}

std::pair<QVector3D, QVector3D> PointCloudProcessor::computeBoundingBox(const std::vector<QVector3D>& points) const
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

// 私有方法实现
std::vector<QVector3D> PointCloudProcessor::readPCDFile(const QString& filename) const
{
    // 使用现有的PCDReader
    return PCDReader::ReadVec3PointCloudPCD(filename);
}

std::vector<QVector3D> PointCloudProcessor::readPLYFile(const QString& filename) const
{
    // PLY格式读取的简化实现
    std::vector<QVector3D> points;
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        throw PointCloudProcessorException(QString("Cannot open PLY file: %1").arg(filename));
    }
    
    QTextStream stream(&file);
    QString line;
    bool inHeader = true;
    int vertexCount = 0;
    
    // 解析头部
    while (!stream.atEnd() && inHeader) {
        line = stream.readLine().trimmed();
        
        if (line.startsWith("element vertex")) {
            QStringList parts = line.split(' ');
            if (parts.size() >= 3) {
                vertexCount = parts[2].toInt();
            }
        } else if (line == "end_header") {
            inHeader = false;
        }
    }
    
    // 读取顶点数据
    points.reserve(vertexCount);
    for (int i = 0; i < vertexCount && !stream.atEnd(); ++i) {
        line = stream.readLine().trimmed();
        QStringList coords = line.split(' ');
        
        if (coords.size() >= 3) {
            float x = coords[0].toFloat();
            float y = coords[1].toFloat();
            float z = coords[2].toFloat();
            points.emplace_back(x, y, z);
        }
    }
    
    return points;
}

std::vector<QVector3D> PointCloudProcessor::readXYZFile(const QString& filename) const
{
    std::vector<QVector3D> points;
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        throw PointCloudProcessorException(QString("Cannot open XYZ file: %1").arg(filename));
    }
    
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue; // 跳过空行和注释
        }
        
        QStringList coords = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (coords.size() >= 3) {
            float x = coords[0].toFloat();
            float y = coords[1].toFloat();
            float z = coords[2].toFloat();
            points.emplace_back(x, y, z);
        }
    }
    
    return points;
}

std::vector<QVector3D> PointCloudProcessor::readTXTFile(const QString& filename) const
{
    // TXT格式与XYZ格式类似
    return readXYZFile(filename);
}

float PointCloudProcessor::calculateDistance(const QVector3D& p1, const QVector3D& p2) const
{
    return (p1 - p2).length();
}

std::vector<QVector3D> PointCloudProcessor::removeOutliers(const std::vector<QVector3D>& points,
                                                          int neighborCount,
                                                          float stdDevThreshold) const
{
    if (points.size() < neighborCount) {
        return points; // 点数太少，无法进行离群点检测
    }

    std::vector<QVector3D> filteredPoints;
    filteredPoints.reserve(points.size());

    emitStatusMessage("Computing neighbor distances...");

    for (size_t i = 0; i < points.size(); ++i) {
        // 计算到最近邻居的距离
        std::vector<float> distances;
        distances.reserve(neighborCount);

        for (size_t j = 0; j < points.size(); ++j) {
            if (i != j) {
                float dist = calculateDistance(points[i], points[j]);
                distances.push_back(dist);
            }
        }

        // 排序并取前k个最近邻居
        std::sort(distances.begin(), distances.end());
        if (distances.size() > neighborCount) {
            distances.resize(neighborCount);
        }

        // 计算平均距离
        float meanDistance = 0.0f;
        for (float dist : distances) {
            meanDistance += dist;
        }
        meanDistance /= distances.size();

        // 计算标准差
        float variance = 0.0f;
        for (float dist : distances) {
            variance += (dist - meanDistance) * (dist - meanDistance);
        }
        float stdDev = qSqrt(variance / distances.size());

        // 检查是否为离群点
        if (meanDistance <= (stdDevThreshold * stdDev)) {
            filteredPoints.push_back(points[i]);
        }

        // 更新进度
        if (i % 1000 == 0) {
            int progress = static_cast<int>((i * 100) / points.size());
            emitProcessingProgress(progress);
        }
    }

    emitProcessingProgress(100);
    return filteredPoints;
}

std::vector<QVector3D> PointCloudProcessor::downsamplePointCloud(const std::vector<QVector3D>& points,
                                                                 float voxelSize) const
{
    if (points.empty() || voxelSize <= 0.0f) {
        return points;
    }

    emitStatusMessage("Downsampling point cloud...");

    // 使用体素网格进行下采样
    std::unordered_map<std::string, std::vector<QVector3D>> voxelMap;

    for (const auto& point : points) {
        // 计算体素索引
        int vx = static_cast<int>(std::floor(point.x() / voxelSize));
        int vy = static_cast<int>(std::floor(point.y() / voxelSize));
        int vz = static_cast<int>(std::floor(point.z() / voxelSize));

        // 创建体素键
        std::string voxelKey = std::to_string(vx) + "_" + std::to_string(vy) + "_" + std::to_string(vz);

        voxelMap[voxelKey].push_back(point);
    }

    // 计算每个体素的中心点
    std::vector<QVector3D> downsampledPoints;
    downsampledPoints.reserve(voxelMap.size());

    for (const auto& voxel : voxelMap) {
        QVector3D centroid(0, 0, 0);
        for (const auto& point : voxel.second) {
            centroid += point;
        }
        centroid /= voxel.second.size();
        downsampledPoints.push_back(centroid);
    }

    return downsampledPoints;
}

std::vector<QVector3D> PointCloudProcessor::filterByHeight(const std::vector<QVector3D>& points,
                                                          float minHeight,
                                                          float maxHeight) const
{
    std::vector<QVector3D> filteredPoints;
    filteredPoints.reserve(points.size());

    for (const auto& point : points) {
        if (point.z() >= minHeight && point.z() <= maxHeight) {
            filteredPoints.push_back(point);
        }
    }

    return filteredPoints;
}

std::pair<std::vector<QVector3D>, std::vector<QVector3D>>
PointCloudProcessor::separateGroundPoints(const std::vector<QVector3D>& points,
                                         float groundThreshold) const
{
    std::vector<QVector3D> groundPoints;
    std::vector<QVector3D> nonGroundPoints;

    if (points.empty()) {
        return {groundPoints, nonGroundPoints};
    }

    // 简化的地面分离算法：基于高度阈值
    auto bbox = computeBoundingBox(points);
    float groundLevel = bbox.first.z() + groundThreshold;

    for (const auto& point : points) {
        if (point.z() <= groundLevel) {
            groundPoints.push_back(point);
        } else {
            nonGroundPoints.push_back(point);
        }
    }

    return {groundPoints, nonGroundPoints};
}

std::vector<QVector3D> PointCloudProcessor::transformCoordinates(const std::vector<QVector3D>& points,
                                                                CoordinateSystem sourceSystem,
                                                                CoordinateSystem targetSystem) const
{
    return m_lasReader->transformCoordinates(points, sourceSystem, targetSystem);
}

void PointCloudProcessor::setProcessingParameters(const QVariantMap& parameters)
{
    m_processingParameters = parameters;
    emitStatusMessage("Processing parameters updated");
}

QVariantMap PointCloudProcessor::getProcessingParameters() const
{
    return m_processingParameters;
}

// 辅助方法实现
void PointCloudProcessor::emitStatusMessage(const QString& message) const
{
    const_cast<PointCloudProcessor*>(this)->statusMessage(message);
}

void PointCloudProcessor::emitProcessingProgress(int percentage) const
{
    const_cast<PointCloudProcessor*>(this)->processingProgress(percentage);
}

} // namespace WallExtraction
