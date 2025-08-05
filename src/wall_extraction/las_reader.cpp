#include "las_reader.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDataStream>
#include <QtMath>
#include <QElapsedTimer>

namespace WallExtraction {

// LASReaderException 实现
LASReaderException::LASReaderException(const QString& message)
    : m_message(message)
    , m_detailedMessage(QString("LASReaderException: %1").arg(message))
{
}

const char* LASReaderException::what() const noexcept
{
    return m_message.toLocal8Bit().constData();
}

QString LASReaderException::getDetailedMessage() const
{
    return m_detailedMessage;
}

// LASReader 实现
LASReader::LASReader(QObject* parent)
    : QObject(parent)
{
    // 初始化支持的LAS版本
    m_supportedVersions = {
        {1, 2}, {1, 3}, {1, 4}
    };
    
    // 初始化支持的点记录格式
    m_supportedPointFormats = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // 注册元类型
    qRegisterMetaType<CoordinateSystem>("CoordinateSystem");
    qRegisterMetaType<LASVersion>("LASVersion");
    qRegisterMetaType<LASHeader>("LASHeader");
    
    qDebug() << "LASReader created, supporting LAS versions 1.2-1.4";
}

LASReader::~LASReader()
{
    qDebug() << "LASReader destroyed";
}

QStringList LASReader::getSupportedFormats() const
{
    return {"las", "laz"};
}

bool LASReader::canReadFile(const QString& filename) const
{
    if (!QFile::exists(filename)) {
        return false;
    }
    
    QFileInfo fileInfo(filename);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix != "las" && suffix != "laz") {
        return false;
    }
    
    return validateLASSignature(filename);
}

QString LASReader::detectFormat(const QString& filename) const
{
    if (!canReadFile(filename)) {
        return QString();
    }
    
    QFileInfo fileInfo(filename);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "laz" || isLAZFile(filename)) {
        return "laz";
    } else if (suffix == "las") {
        return "las";
    }
    
    return QString();
}

bool LASReader::supportsVersion(quint8 major, quint8 minor) const
{
    for (const auto& version : m_supportedVersions) {
        if (version.major == major && version.minor == minor) {
            return true;
        }
    }
    return false;
}

bool LASReader::supportsPointRecordFormat(quint8 format) const
{
    return m_supportedPointFormats.contains(format);
}

bool LASReader::supportsWKT() const
{
    return true; // 基础WKT支持
}

bool LASReader::supportsUTM() const
{
    return true; // 基础UTM支持
}

LASHeader LASReader::parseHeader(const QString& filename) const
{
    // 检查缓存
    if (m_headerCache.contains(filename)) {
        return m_headerCache[filename];
    }
    
    if (!canReadFile(filename)) {
        throw LASReaderException(QString("Cannot read file: %1").arg(filename));
    }
    
    QByteArray headerData = readLASHeader(filename);
    if (headerData.size() < 227) { // LAS 1.2最小头大小
        throw LASReaderException("Invalid LAS header size");
    }
    
    LASHeader header;
    
    // 解析版本信息
    header.version.major = static_cast<quint8>(headerData[24]);
    header.version.minor = static_cast<quint8>(headerData[25]);
    
    if (!supportsVersion(header.version.major, header.version.minor)) {
        throw LASReaderException(QString("Unsupported LAS version: %1.%2")
                               .arg(header.version.major).arg(header.version.minor));
    }
    
    // 解析点数量
    header.pointCount = *reinterpret_cast<const quint32*>(headerData.data() + 107);
    
    // 解析点数据记录格式
    header.pointDataRecordFormat = static_cast<quint8>(headerData[104]);
    header.pointDataRecordLength = *reinterpret_cast<const quint16*>(headerData.data() + 105);
    
    if (!supportsPointRecordFormat(header.pointDataRecordFormat)) {
        throw LASReaderException(QString("Unsupported point record format: %1")
                               .arg(header.pointDataRecordFormat));
    }
    
    // 解析缩放和偏移
    header.xScale = *reinterpret_cast<const double*>(headerData.data() + 131);
    header.yScale = *reinterpret_cast<const double*>(headerData.data() + 139);
    header.zScale = *reinterpret_cast<const double*>(headerData.data() + 147);
    header.xOffset = *reinterpret_cast<const double*>(headerData.data() + 155);
    header.yOffset = *reinterpret_cast<const double*>(headerData.data() + 163);
    header.zOffset = *reinterpret_cast<const double*>(headerData.data() + 171);
    
    // 解析边界框
    header.xMax = *reinterpret_cast<const double*>(headerData.data() + 179);
    header.xMin = *reinterpret_cast<const double*>(headerData.data() + 187);
    header.yMax = *reinterpret_cast<const double*>(headerData.data() + 195);
    header.yMin = *reinterpret_cast<const double*>(headerData.data() + 203);
    header.zMax = *reinterpret_cast<const double*>(headerData.data() + 211);
    header.zMin = *reinterpret_cast<const double*>(headerData.data() + 219);
    
    // 解析坐标系统（简化实现）
    header.coordinateSystem.type = CoordinateSystem::Unknown;
    header.coordinateSystem.epsgCode = 0;
    
    // 缓存头信息
    m_headerCache[filename] = header;
    
    qDebug() << "Parsed LAS header:" << filename 
             << "Version:" << header.version.major << "." << header.version.minor
             << "Points:" << header.pointCount;
    
    return header;
}

CoordinateSystemInfo LASReader::parseCoordinateSystem(const QString& filename) const
{
    LASHeader header = parseHeader(filename);
    return header.coordinateSystem;
}

std::vector<QVector3D> LASReader::readPointCloud(const QString& filename) const
{
    QElapsedTimer timer;
    timer.start();
    
    LASHeader header = parseHeader(filename);
    std::vector<QVector3D> points;
    points.reserve(header.pointCount);
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        throw LASReaderException(QString("Cannot open file: %1").arg(filename));
    }
    
    // 跳过头部，定位到点数据
    quint32 pointDataOffset = *reinterpret_cast<const quint32*>(readLASHeader(filename).data() + 96);
    file.seek(pointDataOffset);
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    for (quint32 i = 0; i < header.pointCount; ++i) {
        QByteArray pointData = file.read(header.pointDataRecordLength);
        if (pointData.size() != header.pointDataRecordLength) {
            throw LASReaderException(QString("Unexpected end of file at point %1").arg(i));
        }
        
        // 解析X, Y, Z坐标
        qint32 rawX = *reinterpret_cast<const qint32*>(pointData.data() + 0);
        qint32 rawY = *reinterpret_cast<const qint32*>(pointData.data() + 4);
        qint32 rawZ = *reinterpret_cast<const qint32*>(pointData.data() + 8);
        
        // 应用缩放和偏移
        double x = applyScaleAndOffset(rawX, header.xScale, header.xOffset);
        double y = applyScaleAndOffset(rawY, header.yScale, header.yOffset);
        double z = applyScaleAndOffset(rawZ, header.zScale, header.zOffset);
        
        points.emplace_back(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
        
        // 发送进度信号
        if (i % 10000 == 0) {
            int progress = static_cast<int>((i * 100) / header.pointCount);
            const_cast<LASReader*>(this)->emitReadProgress(progress);
        }
    }
    
    const_cast<LASReader*>(this)->emitReadProgress(100);
    
    qint64 elapsed = timer.elapsed();
    qDebug() << "Read" << points.size() << "points in" << elapsed << "ms";
    
    return points;
}

std::vector<PointWithAttributes> LASReader::readPointCloudWithAttributes(const QString& filename) const
{
    LASHeader header = parseHeader(filename);
    std::vector<PointWithAttributes> points;
    points.reserve(header.pointCount);
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        throw LASReaderException(QString("Cannot open file: %1").arg(filename));
    }
    
    // 跳过头部，定位到点数据
    quint32 pointDataOffset = *reinterpret_cast<const quint32*>(readLASHeader(filename).data() + 96);
    file.seek(pointDataOffset);
    
    for (quint32 i = 0; i < header.pointCount; ++i) {
        QByteArray pointData = file.read(header.pointDataRecordLength);
        if (pointData.size() != header.pointDataRecordLength) {
            throw LASReaderException(QString("Unexpected end of file at point %1").arg(i));
        }
        
        PointWithAttributes point = parsePointRecord(pointData, header.pointDataRecordFormat, header);
        points.push_back(point);
        
        // 发送进度信号
        if (i % 10000 == 0) {
            int progress = static_cast<int>((i * 100) / header.pointCount);
            const_cast<LASReader*>(this)->emitReadProgress(progress);
        }
    }
    
    const_cast<LASReader*>(this)->emitReadProgress(100);
    return points;
}

QStringList LASReader::getAvailableAttributes(const QString& filename) const
{
    LASHeader header = parseHeader(filename);
    QStringList attributes = {"x", "y", "z", "intensity", "classification"};
    
    // 根据点记录格式添加可用属性
    switch (header.pointDataRecordFormat) {
        case 2:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
            attributes << "red" << "green" << "blue";
            break;
        default:
            break;
    }
    
    return attributes;
}

QVector3D LASReader::transformCoordinates(const QVector3D& point,
                                         CoordinateSystem sourceSystem,
                                         CoordinateSystem targetSystem) const
{
    // 简化的坐标转换实现
    // 实际应用中需要使用专业的坐标转换库如PROJ
    
    if (sourceSystem == targetSystem) {
        return point;
    }
    
    // 这里只是示例转换，实际需要实现完整的坐标转换
    QVector3D transformed = point;
    
    // 简单的偏移转换示例
    if (sourceSystem == CoordinateSystem::UTM_Zone33N && targetSystem == CoordinateSystem::WGS84) {
        // UTM到地理坐标的简化转换
        transformed.setX(point.x() / 100000.0); // 简化的经度转换
        transformed.setY(point.y() / 100000.0); // 简化的纬度转换
    }
    
    return transformed;
}

std::vector<QVector3D> LASReader::transformCoordinates(const std::vector<QVector3D>& points,
                                                       CoordinateSystem sourceSystem,
                                                       CoordinateSystem targetSystem) const
{
    std::vector<QVector3D> transformed;
    transformed.reserve(points.size());
    
    for (const auto& point : points) {
        transformed.push_back(transformCoordinates(point, sourceSystem, targetSystem));
    }
    
    return transformed;
}

// 私有方法实现
bool LASReader::validateLASSignature(const QString& filename) const
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray signature = file.read(4);
    return signature == "LASF";
}

QByteArray LASReader::readLASHeader(const QString& filename) const
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        throw LASReaderException(QString("Cannot open file: %1").arg(filename));
    }
    
    // 读取最小头大小
    QByteArray header = file.read(227);
    if (header.size() < 227) {
        throw LASReaderException("File too small to contain valid LAS header");
    }
    
    return header;
}

PointWithAttributes LASReader::parsePointRecord(const QByteArray& data, 
                                               quint8 format, 
                                               const LASHeader& header) const
{
    PointWithAttributes point;
    
    // 解析坐标
    qint32 rawX = *reinterpret_cast<const qint32*>(data.data() + 0);
    qint32 rawY = *reinterpret_cast<const qint32*>(data.data() + 4);
    qint32 rawZ = *reinterpret_cast<const qint32*>(data.data() + 8);
    
    double x = applyScaleAndOffset(rawX, header.xScale, header.xOffset);
    double y = applyScaleAndOffset(rawY, header.yScale, header.yOffset);
    double z = applyScaleAndOffset(rawZ, header.zScale, header.zOffset);
    
    point.position = QVector3D(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    
    // 解析强度
    quint16 intensity = *reinterpret_cast<const quint16*>(data.data() + 12);
    point.attributes["intensity"] = intensity;
    
    // 解析分类
    quint8 classification = static_cast<quint8>(data[15]);
    point.attributes["classification"] = classification;
    
    // 根据格式解析RGB颜色
    if (format == 2 || format == 3 || format == 5 || format == 7 || format == 8 || format == 10) {
        if (data.size() >= 26) { // 确保有足够的数据
            quint16 red = *reinterpret_cast<const quint16*>(data.data() + 20);
            quint16 green = *reinterpret_cast<const quint16*>(data.data() + 22);
            quint16 blue = *reinterpret_cast<const quint16*>(data.data() + 24);
            
            point.attributes["red"] = red;
            point.attributes["green"] = green;
            point.attributes["blue"] = blue;
        }
    }
    
    return point;
}

double LASReader::applyScaleAndOffset(qint32 rawCoord, double scale, double offset) const
{
    return rawCoord * scale + offset;
}

CoordinateSystemInfo LASReader::parseWKTString(const QString& wktString) const
{
    CoordinateSystemInfo info;
    info.wktString = wktString;
    info.type = CoordinateSystem::Unknown;
    info.epsgCode = 0;
    
    // 简化的WKT解析
    if (wktString.contains("WGS84", Qt::CaseInsensitive)) {
        info.type = CoordinateSystem::WGS84;
        info.epsgCode = 4326;
    } else if (wktString.contains("UTM", Qt::CaseInsensitive)) {
        if (wktString.contains("33N", Qt::CaseInsensitive)) {
            info.type = CoordinateSystem::UTM_Zone33N;
            info.epsgCode = 32633;
        } else if (wktString.contains("34N", Qt::CaseInsensitive)) {
            info.type = CoordinateSystem::UTM_Zone34N;
            info.epsgCode = 32634;
        }
    }
    
    return info;
}

bool LASReader::isLAZFile(const QString& filename) const
{
    // 简化的LAZ检测，实际需要检查压缩标志
    return filename.toLower().endsWith(".laz");
}

QByteArray LASReader::decompressLAZData(const QByteArray& compressedData) const
{
    // LAZ解压缩需要LASzip库，这里返回原数据作为占位符
    qWarning() << "LAZ decompression not implemented, returning original data";
    return compressedData;
}

// 辅助方法实现
void LASReader::emitReadProgress(int percentage)
{
    emit readProgress(percentage);
}

} // namespace WallExtraction
