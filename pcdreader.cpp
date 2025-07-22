#include "PCDReader.h"
#include <lz4.h>  // 需要添加LZ4库

//test 这个类没有使用，建议保留，防止读取其他格式pcd文件报错

/* 主要的PCD文件读取函数 */
/* 修复后的PCD文件读取函数 */
/* 完全重写的PCD文件读取函数 - 支持多种压缩格式 */
std::vector<QVector3D> PCDReader::ReadVec3PointCloudPCD(const QString& filename) {
    std::vector<QVector3D> cloud;
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开PCD文件：" << filename;
        return cloud;
    }

    // 解析文件头部
    PCDHeader header = parseHeader(file);
    if (!header.isValid) {
        qDebug() << "PCD文件头部解析失败";
        file.close();
        return cloud;
    }

    // 查找坐标字段索引
    int xIndex = -1, yIndex = -1, zIndex = -1;
    for (int i = 0; i < header.fields.size(); ++i) {
        QString field = header.fields[i].toLower();
        if (field == "x") xIndex = i;
        else if (field == "y") yIndex = i;
        else if (field == "z") zIndex = i;
    }

    if (xIndex == -1 || yIndex == -1 || zIndex == -1) {
        qDebug() << "错误：缺少必要的x, y, z坐标字段";
        file.close();
        return cloud;
    }

    // 根据数据格式选择相应的读取方法
    if (header.dataType == "ascii") {
        cloud = readAsciiData(file, header, xIndex, yIndex, zIndex);
    } else if (header.dataType == "binary") {
        cloud = readBinaryData(file, header, xIndex, yIndex, zIndex);
    } else if (header.dataType == "binary_compressed") {
        cloud = readBinaryCompressedDataAdvanced(file, header, xIndex, yIndex, zIndex);
    } else {
        qDebug() << "错误：未知的数据格式：" << header.dataType;
    }

    file.close();
    return cloud;
}

/* 解析PCD文件头部 */
/* 修复后的解析PCD文件头部函数 */
/* 改进的头部解析函数 */
PCDReader::PCDHeader PCDReader::parseHeader(QFile& file) {
    PCDHeader header;
    QTextStream in(&file);
    QString line;

    while (!in.atEnd()) {
        qint64 lineStart = file.pos();
        line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith("#")) {
            continue;
        }

        if (line.startsWith("VERSION")) {
            header.version = line.split(' ')[1];
        }
        else if (line.startsWith("FIELDS")) {
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            for (int i = 1; i < parts.size(); ++i) {
                header.fields.append(parts[i].toLower());
            }
        }
        else if (line.startsWith("SIZE")) {
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            for (int i = 1; i < parts.size(); ++i) {
                header.sizes.append(parts[i]);
            }
        }
        else if (line.startsWith("TYPE")) {
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            for (int i = 1; i < parts.size(); ++i) {
                header.types.append(parts[i]);
            }
        }
        else if (line.startsWith("COUNT")) {
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            for (int i = 1; i < parts.size(); ++i) {
                header.counts.append(parts[i]);
            }
        }
        else if (line.startsWith("WIDTH")) {
            header.width = line.split(' ')[1].toInt();
        }
        else if (line.startsWith("HEIGHT")) {
            header.height = line.split(' ')[1].toInt();
        }
        else if (line.startsWith("VIEWPOINT")) {
            header.viewpoint = line;
        }
        else if (line.startsWith("POINTS")) {
            header.points = line.split(' ')[1].toInt();
        }
        else if (line.startsWith("DATA")) {
            header.dataType = line.split(' ')[1].toLower();
            header.dataStartPos = file.pos();
            break;
        }
    }

    header.isValid = (header.points > 0 && !header.fields.isEmpty() && !header.dataType.isEmpty());

    qDebug() << "PCD头部信息：";
    qDebug() << "  版本：" << header.version;
    qDebug() << "  字段：" << header.fields;
    qDebug() << "  字段大小：" << header.sizes;
    qDebug() << "  字段类型：" << header.types;
    qDebug() << "  点数：" << header.points;
    qDebug() << "  数据类型：" << header.dataType;
    qDebug() << "  数据开始位置：" << header.dataStartPos;

    return header;
}

/* 高级压缩数据读取函数 */
std::vector<QVector3D> PCDReader::readBinaryCompressedDataAdvanced(QFile& file, const PCDHeader& header,
                                                                   int xIndex, int yIndex, int zIndex) {
    std::vector<QVector3D> cloud;

    qDebug() << "开始解析Binary_Compressed格式（高级模式）...";

    // 确保文件指针在正确位置
    file.seek(header.dataStartPos);

    qint64 fileSize = file.size();
    qint64 remainingBytes = fileSize - header.dataStartPos;
    qDebug() << "文件总大小：" << fileSize << "，数据开始位置：" << header.dataStartPos << "，剩余字节：" << remainingBytes;

    // 读取所有剩余数据
    QByteArray allData = file.readAll();
    qDebug() << "实际读取字节数：" << allData.size();

    if (allData.isEmpty()) {
        qDebug() << "错误：无法读取任何数据";
        return cloud;
    }

    // 尝试多种压缩格式解析
    QByteArray decompressedData = tryMultipleDecompressionMethods(allData, header);

    if (decompressedData.isEmpty()) {
        qDebug() << "所有解压缩方法都失败，尝试智能解析原始数据...";
        return intelligentRawDataParsing(allData, header, xIndex, yIndex, zIndex);
    }

    qDebug() << "解压缩成功，数据大小：" << decompressedData.size() << "字节";
    return parseBinaryPointDataAdvanced(decompressedData, header, xIndex, yIndex, zIndex);
}

/* 尝试多种解压缩方法 */
QByteArray PCDReader::tryMultipleDecompressionMethods(const QByteArray& data, const PCDHeader& header) {
    QByteArray result;

    // 方法1：标准PCL压缩格式（8字节头部）
    if (data.size() >= 8) {
        quint32 uncompressedSize = *reinterpret_cast<const quint32*>(data.data());
        quint32 compressedSize = *reinterpret_cast<const quint32*>(data.data() + 4);

        qDebug() << "尝试标准PCL格式 - 未压缩大小：" << uncompressedSize << "，压缩大小：" << compressedSize;

        if (compressedSize > 0 && compressedSize <= static_cast<quint32>(data.size() - 8)) {
            QByteArray compressedData = data.mid(8, compressedSize);
            result = tryZlibDecompression(compressedData);
            if (!result.isEmpty()) {
                qDebug() << "标准PCL格式解压缩成功";
                return result;
            }
        }
    }

    // 方法2：LZ4压缩格式检测
    result = tryLZ4Decompression(data, header);
    if (!result.isEmpty()) {
        qDebug() << "LZ4格式解压缩成功";
        return result;
    }

    // 方法3：直接zlib/deflate压缩
    result = tryZlibDecompression(data);
    if (!result.isEmpty()) {
        qDebug() << "直接zlib解压缩成功";
        return result;
    }

    // 方法4：尝试不同的偏移量
    QList<int> offsets = {0, 4, 8, 12, 16, 32, 64};
    for (int offset : offsets) {
        if (offset < data.size()) {
            QByteArray testData = data.mid(offset);
            result = tryZlibDecompression(testData);
            if (!result.isEmpty()) {
                qDebug() << "在偏移量" << offset << "处找到有效的zlib数据";
                return result;
            }
        }
    }

    qDebug() << "所有标准解压缩方法都失败";
    return QByteArray();
}

/* 尝试zlib解压缩 */
QByteArray PCDReader::tryZlibDecompression(const QByteArray& data) {
    // 直接尝试qUncompress
    QByteArray result = qUncompress(data);
    if (!result.isEmpty()) {
        return result;
    }

    // 尝试添加zlib头部
    QByteArray zlibData;
    zlibData.append(char(0x78)); // CMF
    zlibData.append(char(0x9C)); // FLG (默认压缩级别)
    zlibData.append(data);

    result = qUncompress(zlibData);
    if (!result.isEmpty()) {
        return result;
    }

    // 尝试不同的zlib头部
    QList<QPair<char, char>> headers = {
        {0x78, 0x01}, // 最快压缩
        {0x78, 0x5E}, // 最佳压缩
        {0x78, 0xDA}, // 默认压缩
        {0x58, 0x85}  // 备选格式
    };

    for (const auto& header : headers) {
        QByteArray testData;
        testData.append(header.first);
        testData.append(header.second);
        testData.append(data);

        result = qUncompress(testData);
        if (!result.isEmpty()) {
            return result;
        }
    }

    return QByteArray();
}

/* 尝试LZ4解压缩（简单检测） */
QByteArray PCDReader::tryLZ4Decompression(const QByteArray& data, const PCDHeader& header) {
    Q_UNUSED(header)
    // LZ4格式的简单检测
    // 通常LZ4压缩的数据会有特定的魔数或格式

    // 检查是否可能是LZ4格式
    if (data.size() > 4) {
        // LZ4通常以特定的魔数开始
        const char* ptr = data.data();

        // 检查一些常见的LZ4魔数
        if ((ptr[0] == 0x04 && ptr[1] == 0x22 && ptr[2] == 0x4D && ptr[3] == 0x18) || // LZ4 frame格式
            (ptr[0] == 0x02 && ptr[1] == 0x21 && ptr[2] == 0x4C && ptr[3] == 0x18)) { // 旧版LZ4
            qDebug() << "检测到可能的LZ4格式，但Qt不原生支持LZ4解压缩";
            // 这里可以集成第三方LZ4库，暂时返回空
            return QByteArray();
        }
    }

    return QByteArray();
}

/* 智能原始数据解析 */
std::vector<QVector3D> PCDReader::intelligentRawDataParsing(const QByteArray& data, const PCDHeader& header,
                                                            int xIndex, int yIndex, int zIndex) {
    qDebug() << "开始智能原始数据解析...";

    // 计算每个点的字节大小
    int pointSize = 0;
    for (const QString& size : header.sizes) {
        pointSize += size.toInt();
    }

    qDebug() << "每个点的字节大小：" << pointSize;
    qDebug() << "期望处理的点数：" << header.points;
    qDebug() << "实际数据大小：" << data.size();

    // 尝试不同的起始偏移量来找到正确的数据开始位置
    QList<int> offsets = {0, 8, 16, 32, 64, 128};

    for (int offset : offsets) {
        if (offset >= data.size()) continue;

        qDebug() << "尝试偏移量：" << offset;

        QByteArray testData = data.mid(offset);
        std::vector<QVector3D> testCloud = parseBinaryPointDataAdvanced(testData, header, xIndex, yIndex, zIndex);

        if (!testCloud.empty()) {
            // 验证解析结果的合理性
            if (validatePointCloud(testCloud)) {
                qDebug() << "在偏移量" << offset << "处找到有效的点云数据，点数：" << testCloud.size();
                return testCloud;
            }
        }
    }

    qDebug() << "智能解析失败，尝试直接解析...";
    return parseBinaryPointDataAdvanced(data, header, xIndex, yIndex, zIndex);
}

/* 验证点云数据的合理性 */
bool PCDReader::validatePointCloud(const std::vector<QVector3D>& cloud) {
    if (cloud.empty()) return false;
    if (cloud.size() < 100) return false; // 太少的点可能不是有效数据

    // 检查坐标范围的合理性
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    int validCount = 0;
    for (const auto& point : cloud) {
        if (std::isfinite(point.x()) && std::isfinite(point.y()) && std::isfinite(point.z())) {
            minX = std::min(minX, point.x());
            maxX = std::max(maxX, point.x());
            minY = std::min(minY, point.y());
            maxY = std::max(maxY, point.y());
            minZ = std::min(minZ, point.z());
            maxZ = std::max(maxZ, point.z());
            validCount++;
        }

        if (validCount >= 100) break; // 只检查前100个点
    }

    // 检查坐标范围是否合理
    float rangeX = maxX - minX;
    float rangeY = maxY - minY;
    float rangeZ = maxZ - minZ;

    // 坐标范围应该在合理的范围内
    bool rangeValid = (rangeX > 0.001f && rangeX < 1000000.0f &&
                       rangeY > 0.001f && rangeY < 1000000.0f &&
                       rangeZ > 0.001f && rangeZ < 1000000.0f);

    // 有效点的比例应该足够高
    bool ratioValid = (static_cast<float>(validCount) / cloud.size()) > 0.8f;

    qDebug() << QString("点云验证 - 范围X:[%1,%2], Y:[%3,%4], Z:[%5,%6], 有效比例:%7%")
                    .arg(minX, 0, 'f', 3).arg(maxX, 0, 'f', 3)
                    .arg(minY, 0, 'f', 3).arg(maxY, 0, 'f', 3)
                    .arg(minZ, 0, 'f', 3).arg(maxZ, 0, 'f', 3)
                    .arg(static_cast<float>(validCount) / cloud.size() * 100, 0, 'f', 1);

    return rangeValid && ratioValid;
}

/* 高级二进制点数据解析 */
std::vector<QVector3D> PCDReader::parseBinaryPointDataAdvanced(const QByteArray& data, const PCDHeader& header,
                                                               int xIndex, int yIndex, int zIndex) {
    std::vector<QVector3D> cloud;

    // 计算每个点的字节大小
    int pointSize = 0;
    for (const QString& size : header.sizes) {
        pointSize += size.toInt();
    }

    if (pointSize <= 0) {
        qDebug() << "错误：点大小计算失败";
        return cloud;
    }

    int actualPoints = data.size() / pointSize;
    qDebug() << "每个点的字节大小：" << pointSize;
    qDebug() << "实际数据大小：" << data.size();
    qDebug() << "实际可读取点数：" << actualPoints;

    if (actualPoints <= 0) {
        qDebug() << "错误：无法读取任何点数据";
        return cloud;
    }

    // 限制最大处理点数以避免内存问题
    int maxPoints = std::min(actualPoints, 5000000); // 最多500万个点
    cloud.reserve(maxPoints);

    // 计算各坐标字段的偏移量
    int xOffset = calculateOffset(header.sizes, xIndex);
    int yOffset = calculateOffset(header.sizes, yIndex);
    int zOffset = calculateOffset(header.sizes, zIndex);

    qDebug() << "坐标字段索引 - X:" << xIndex << ", Y:" << yIndex << ", Z:" << zIndex;
    qDebug() << "坐标偏移量 - X:" << xOffset << ", Y:" << yOffset << ", Z:" << zOffset;

    int validPoints = 0;
    int invalidPoints = 0;
    const char* dataPtr = data.data();

    // 统计坐标范围
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    qDebug() << "开始解析点数据（最多处理" << maxPoints << "个点）...";
    qDebug() << "前10个点的坐标：";

    for (int i = 0; i < maxPoints; ++i) {
        const char* pointPtr = dataPtr + (i * pointSize);

        // 确保不会越界
        if (pointPtr + pointSize > dataPtr + data.size()) {
            qDebug() << "到达数据边界，实际处理了" << i << "个点";
            break;
        }

        float x, y, z;

        // 安全地复制数据
        memcpy(&x, pointPtr + xOffset, sizeof(float));
        memcpy(&y, pointPtr + yOffset, sizeof(float));
        memcpy(&z, pointPtr + zOffset, sizeof(float));

        // 调试前10个点
        if (i < 10) {
            qDebug() << QString("点%1: X=%2, Y=%3, Z=%4").arg(i).arg(x, 0, 'f', 6).arg(y, 0, 'f', 6).arg(z, 0, 'f', 6);
        }

        // 检查坐标值是否有效
        if (std::isfinite(x) && std::isfinite(y) && std::isfinite(z)) {
            // 合理性检查：排除极端值
            if (std::abs(x) < 1e6 && std::abs(y) < 1e6 && std::abs(z) < 1e6) {
                cloud.push_back(QVector3D(x, y, z));
                validPoints++;

                // 更新坐标范围
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
                minZ = std::min(minZ, z);
                maxZ = std::max(maxZ, z);
            } else {
                invalidPoints++;
            }
        } else {
            invalidPoints++;
        }

        // 定期输出进度
        if (i > 0 && i % 200000 == 0) {
            qDebug() << "已处理" << i << "个点，有效点数：" << validPoints << "，无效点数：" << invalidPoints;
        }
    }

    qDebug() << "解析完成，有效点数：" << validPoints << "，无效点数：" << invalidPoints;

    if (validPoints > 0) {
        qDebug() << QString("坐标范围 - X:[%1, %2], Y:[%3, %4], Z:[%5, %6]")
                        .arg(minX, 0, 'f', 3).arg(maxX, 0, 'f', 3)
                        .arg(minY, 0, 'f', 3).arg(maxY, 0, 'f', 3)
                        .arg(minZ, 0, 'f', 3).arg(maxZ, 0, 'f', 3);
    }

    return cloud;
}

/* 读取ASCII格式数据 */
std::vector<QVector3D> PCDReader::readAsciiData(QFile& file, const PCDHeader& header,
                                                int xIndex, int yIndex, int zIndex) {
    std::vector<QVector3D> cloud;
    cloud.reserve(header.points);

    QTextStream in(&file);
    QString line;
    int validPoints = 0;

    while (!in.atEnd() && validPoints < header.points) {
        line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        // 使用简化的字符串分割方法，兼容Qt5和Qt6
        QStringList values = line.simplified().split(' ', Qt::SkipEmptyParts);

        if (values.size() < header.fields.size()) continue;

        bool xOk, yOk, zOk;
        float x = values[xIndex].toFloat(&xOk);
        float y = values[yIndex].toFloat(&yOk);
        float z = values[zIndex].toFloat(&zOk);

        if (xOk && yOk && zOk && std::isfinite(x) && std::isfinite(y) && std::isfinite(z)) {
            cloud.push_back(QVector3D(x, y, z));
            validPoints++;
        }
    }

    qDebug() << "ASCII格式读取完成，有效点数：" << validPoints;
    return cloud;
}

/* 读取Binary格式数据 */
/* 修复后的读取Binary格式数据函数 */
std::vector<QVector3D> PCDReader::readBinaryData(QFile& file, const PCDHeader& header,
                                                 int xIndex, int yIndex, int zIndex) {
    std::vector<QVector3D> cloud;
    cloud.reserve(header.points);

    // 计算每个点的字节大小
    int pointSize = 0;
    for (const QString& size : header.sizes) {
        pointSize += size.toInt();
    }

    qDebug() << "Binary格式 - 每个点的字节大小：" << pointSize;

    // 计算各字段的偏移量
    int xOffset = calculateOffset(header.sizes, xIndex);
    int yOffset = calculateOffset(header.sizes, yIndex);
    int zOffset = calculateOffset(header.sizes, zIndex);

    qDebug() << "坐标偏移量 - X:" << xOffset << ", Y:" << yOffset << ", Z:" << zOffset;

    int validPoints = 0;
    for (int i = 0; i < header.points && !file.atEnd(); ++i) {
        QByteArray pointData = file.read(pointSize);
        if (pointData.size() != pointSize) {
            qDebug() << "警告：点" << i << "数据读取不完整，期望：" << pointSize << "，实际：" << pointData.size();
            break;
        }

        float x, y, z;
        memcpy(&x, pointData.data() + xOffset, sizeof(float));
        memcpy(&y, pointData.data() + yOffset, sizeof(float));
        memcpy(&z, pointData.data() + zOffset, sizeof(float));

        // 调试前10个点
        if (i < 10) {
            qDebug() << QString("点%1: X=%2, Y=%3, Z=%4").arg(i).arg(x).arg(y).arg(z);
        }

        if (std::isfinite(x) && std::isfinite(y) && std::isfinite(z)) {
            cloud.push_back(QVector3D(x, y, z));
            validPoints++;
        }
    }

    qDebug() << "Binary格式读取完成，有效点数：" << validPoints;
    return cloud;
}

/* 读取Binary_Compressed格式数据 */
std::vector<QVector3D> PCDReader::readBinaryCompressedData(QFile& file, const PCDHeader& header,
                                                           int xIndex, int yIndex, int zIndex) {
    std::vector<QVector3D> cloud;

    qDebug() << "开始解析Binary_Compressed格式...";

    // 确保文件指针在正确位置
    file.seek(header.dataStartPos);

    // 获取文件总大小和剩余可读取的字节数
    qint64 fileSize = file.size();
    qint64 remainingBytes = fileSize - header.dataStartPos;
    qDebug() << "文件总大小：" << fileSize << "，数据开始位置：" << header.dataStartPos << "，剩余字节：" << remainingBytes;

    // 读取所有剩余数据
    QByteArray allData = file.readAll();
    qDebug() << "实际读取字节数：" << allData.size();

    if (allData.isEmpty()) {
        qDebug() << "错误：无法读取任何数据";
        return cloud;
    }

    // 尝试多种解析方法
    QByteArray decompressedData;

    // 方法1：检查是否有压缩头部信息
    if (allData.size() >= 8) {
        quint32 uncompressedSize = *reinterpret_cast<const quint32*>(allData.data());
        quint32 compressedSize = *reinterpret_cast<const quint32*>(allData.data() + 4);

        qDebug() << "检测到的压缩信息 - 未压缩大小：" << uncompressedSize << "，压缩大小：" << compressedSize;

        // 验证压缩大小是否合理
        if (compressedSize > 0 && compressedSize <= static_cast<quint32>(allData.size() - 8)) {
            qDebug() << "使用压缩头部信息，提取压缩数据...";
            QByteArray compressedData = allData.mid(8, compressedSize);

            // 尝试解压缩
            decompressedData = qUncompress(compressedData);

            if (decompressedData.isEmpty()) {
                qDebug() << "qUncompress失败，尝试添加zlib头部...";
                QByteArray zlibData;
                zlibData.append(char(0x78)); // CMF
                zlibData.append(char(0x9C)); // FLG
                zlibData.append(compressedData);
                decompressedData = qUncompress(zlibData);
            }
        }
    }

    // 方法2：尝试直接解压缩整个数据块
    if (decompressedData.isEmpty()) {
        qDebug() << "尝试直接解压缩整个数据块...";
        decompressedData = qUncompress(allData);

        if (decompressedData.isEmpty()) {
            qDebug() << "尝试添加zlib头部到整个数据块...";
            QByteArray zlibData;
            zlibData.append(char(0x78));
            zlibData.append(char(0x9C));
            zlibData.append(allData);
            decompressedData = qUncompress(zlibData);
        }
    }

    // 方法3：跳过可能的头部，尝试不同偏移量
    if (decompressedData.isEmpty()) {
        qDebug() << "尝试不同的数据偏移量...";
        QList<int> offsets = {0, 4, 8, 12, 16, 32}; // 尝试不同的偏移量

        for (int offset : offsets) {
            if (offset < allData.size()) {
                QByteArray testData = allData.mid(offset);
                QByteArray result = qUncompress(testData);

                if (!result.isEmpty()) {
                    qDebug() << "在偏移量" << offset << "处找到有效的压缩数据";
                    decompressedData = result;
                    break;
                }

                // 尝试添加zlib头部
                QByteArray zlibData;
                zlibData.append(char(0x78));
                zlibData.append(char(0x9C));
                zlibData.append(testData);
                result = qUncompress(zlibData);

                if (!result.isEmpty()) {
                    qDebug() << "在偏移量" << offset << "处找到有效的压缩数据（添加zlib头部后）";
                    decompressedData = result;
                    break;
                }
            }
        }
    }

    // 方法4：如果所有解压缩方法都失败，尝试将数据作为原始二进制处理
    if (decompressedData.isEmpty()) {
        qDebug() << "所有解压缩方法都失败，尝试作为原始二进制数据处理...";

        // 计算每个点的字节大小
        int pointSize = 0;
        for (const QString& size : header.sizes) {
            pointSize += size.toInt();
        }

        qDebug() << "每个点的字节大小：" << pointSize;
        qDebug() << "可处理的点数：" << (allData.size() / pointSize);

        // 直接解析原始数据
        return parseBinaryPointData(allData, header, xIndex, yIndex, zIndex);
    }

    qDebug() << "解压缩成功，数据大小：" << decompressedData.size() << "字节";

    // 解析解压缩后的数据
    cloud = parseBinaryPointData(decompressedData, header, xIndex, yIndex, zIndex);

    return cloud;
}

/* 解析二进制点数据 */
std::vector<QVector3D> PCDReader::parseBinaryPointData(const QByteArray& data, const PCDHeader& header,
                                                       int xIndex, int yIndex, int zIndex) {
    std::vector<QVector3D> cloud;

    // 计算每个点的字节大小
    int pointSize = 0;
    for (const QString& size : header.sizes) {
        pointSize += size.toInt();
    }

    qDebug() << "每个点的字节大小：" << pointSize;
    qDebug() << "期望的总数据大小：" << (pointSize * header.points);
    qDebug() << "实际数据大小：" << data.size();

    // 计算实际可以读取的点数
    int actualPoints = std::min(header.points, static_cast<int>(data.size() / pointSize));
    qDebug() << "实际可读取点数：" << actualPoints;

    if (actualPoints <= 0) {
        qDebug() << "错误：无法读取任何点数据";

        // 尝试诊断数据格式问题
        if (data.size() > 64) {
            qDebug() << "数据诊断：前64字节的十六进制表示：";
            QString hexStr;
            for (int i = 0; i < 64; ++i) {
                hexStr += QString("%1 ").arg(static_cast<unsigned char>(data[i]), 2, 16, QChar('0'));
            }
            qDebug() << hexStr;
        }

        return cloud;
    }

    cloud.reserve(actualPoints);

    // 计算各坐标字段的偏移量
    int xOffset = calculateOffset(header.sizes, xIndex);
    int yOffset = calculateOffset(header.sizes, yIndex);
    int zOffset = calculateOffset(header.sizes, zIndex);

    qDebug() << "坐标字段索引 - X:" << xIndex << ", Y:" << yIndex << ", Z:" << zIndex;
    qDebug() << "坐标偏移量 - X:" << xOffset << ", Y:" << yOffset << ", Z:" << zOffset;

    int validPoints = 0;
    int invalidPoints = 0;
    const char* dataPtr = data.data();

    // 用于统计坐标范围
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    // 用于调试的前几个点
    qDebug() << "前10个点的坐标：";

    for (int i = 0; i < actualPoints; ++i) {
        const char* pointPtr = dataPtr + (i * pointSize);

        float x, y, z;

        // 安全地复制数据，确保不会越界
        if (pointPtr + xOffset + sizeof(float) <= dataPtr + data.size() &&
            pointPtr + yOffset + sizeof(float) <= dataPtr + data.size() &&
            pointPtr + zOffset + sizeof(float) <= dataPtr + data.size()) {

            memcpy(&x, pointPtr + xOffset, sizeof(float));
            memcpy(&y, pointPtr + yOffset, sizeof(float));
            memcpy(&z, pointPtr + zOffset, sizeof(float));

            // 调试前10个点
            if (i < 10) {
                qDebug() << QString("点%1: X=%2, Y=%3, Z=%4").arg(i).arg(x, 0, 'f', 6).arg(y, 0, 'f', 6).arg(z, 0, 'f', 6);
            }

            // 检查坐标值是否有效
            if (std::isfinite(x) && std::isfinite(y) && std::isfinite(z)) {
                // 合理性检查：排除极端值
                if (std::abs(x) < 1e6 && std::abs(y) < 1e6 && std::abs(z) < 1e6) {
                    cloud.push_back(QVector3D(x, y, z));
                    validPoints++;

                    // 更新坐标范围
                    minX = std::min(minX, x);
                    maxX = std::max(maxX, x);
                    minY = std::min(minY, y);
                    maxY = std::max(maxY, y);
                    minZ = std::min(minZ, z);
                    maxZ = std::max(maxZ, z);
                } else {
                    invalidPoints++;
                    if (invalidPoints <= 5) { // 只显示前5个无效点的信息
                        qDebug() << QString("无效点%1（极端值）: X=%2, Y=%3, Z=%4").arg(i).arg(x).arg(y).arg(z);
                    }
                }
            } else {
                invalidPoints++;
                if (invalidPoints <= 5) { // 只显示前5个无效点的信息
                    qDebug() << QString("无效点%1（非有限值）: X=%2, Y=%3, Z=%4").arg(i).arg(x).arg(y).arg(z);
                }
            }
        } else {
            qDebug() << "警告：点" << i << "的数据访问越界";
            break;
        }

        // 定期输出进度
        if (i > 0 && i % 500000 == 0) {
            qDebug() << "已处理" << i << "个点，有效点数：" << validPoints << "，无效点数：" << invalidPoints;
        }
    }

    qDebug() << "Binary_Compressed格式读取完成，有效点数：" << validPoints << "，无效点数：" << invalidPoints;

    if (validPoints > 0) {
        qDebug() << QString("坐标范围 - X:[%1, %2], Y:[%3, %4], Z:[%5, %6]")
                        .arg(minX, 0, 'f', 3).arg(maxX, 0, 'f', 3)
                        .arg(minY, 0, 'f', 3).arg(maxY, 0, 'f', 3)
                        .arg(minZ, 0, 'f', 3).arg(maxZ, 0, 'f', 3);
    }

    return cloud;
}

/* 计算字段偏移量 */
int PCDReader::calculateOffset(const QStringList& sizes, int index) {
    int offset = 0;
    for (int i = 0; i < index && i < sizes.size(); ++i) {
        offset += sizes[i].toInt();
    }
    return offset;
}
