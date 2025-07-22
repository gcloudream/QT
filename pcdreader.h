#ifndef PCDREADER_H
#define PCDREADER_H

#include <QtCore>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector3D>
#include <QFileInfo>
#include <QMessageBox>
#include <QTime>
#include <QApplication>
#include <QRegularExpression>
#include <vector>
#include <cmath>
#include <algorithm>

// 如果使用LZ4压缩，需要包含LZ4头文件
// #include <lz4.h>

/**
 * @brief PCD文件读取器类
 * 支持ASCII、Binary和Binary_Compressed格式的PCD文件读取
 */
class PCDReader {

public:
    struct PCDHeader {
        QString version;
        QStringList fields;
        QStringList sizes;
        QStringList types;
        QStringList counts;
        int width = 0;
        int height = 0;
        QString viewpoint;
        int points = 0;
        QString dataType;
        qint64 dataStartPos = 0;
        bool isValid = false;
    };

    /**
     * @brief 读取PCD文件并返回3D点云数据
     * @param filename PCD文件路径
     * @return 3D点的向量，每个点包含x,y,z坐标
     */
    static std::vector<QVector3D> ReadVec3PointCloudPCD(const QString& filename);

private:
    /**
     * @brief 解析PCD文件头部信息
     * @param file 已打开的文件对象
     * @return PCD头部信息结构体
     */
    static PCDHeader parseHeader(QFile& file);

    /**
     * @brief 读取ASCII格式的点云数据
     * @param file 已打开的文件对象
     * @param header PCD头部信息
     * @param xIndex X坐标字段索引
     * @param yIndex Y坐标字段索引
     * @param zIndex Z坐标字段索引
     * @return 3D点的向量
     */
    static std::vector<QVector3D> readAsciiData(QFile& file, const PCDHeader& header,
                                                int xIndex, int yIndex, int zIndex);

    /**
     * @brief 读取Binary格式的点云数据
     * @param file 已打开的文件对象
     * @param header PCD头部信息
     * @param xIndex X坐标字段索引
     * @param yIndex Y坐标字段索引
     * @param zIndex Z坐标字段索引
     * @return 3D点的向量
     */
    static std::vector<QVector3D> readBinaryData(QFile& file, const PCDHeader& header,
                                                 int xIndex, int yIndex, int zIndex);

    /**
     * @brief 读取Binary_Compressed格式的点云数据
     * @param file 已打开的文件对象
     * @param header PCD头部信息
     * @param xIndex X坐标字段索引
     * @param yIndex Y坐标字段索引
     * @param zIndex Z坐标字段索引
     * @return 3D点的向量
     */
    static std::vector<QVector3D> readBinaryCompressedData(QFile& file, const PCDHeader& header,
                                                           int xIndex, int yIndex, int zIndex);

    /**
     * @brief 解析二进制点数据
     * @param data 二进制数据
     * @param header PCD头部信息
     * @param xIndex X坐标字段索引
     * @param yIndex Y坐标字段索引
     * @param zIndex Z坐标字段索引
     * @return 3D点的向量
     */
    static std::vector<QVector3D> parseBinaryPointData(const QByteArray& data, const PCDHeader& header,
                                                       int xIndex, int yIndex, int zIndex);

    /**
     * @brief 计算字段在数据中的偏移量
     * @param sizes 字段大小列表
     * @param index 目标字段索引
     * @return 字节偏移量
     */
    static int calculateOffset(const QStringList& sizes, int index);

    // 新增函数声明
    static std::vector<QVector3D> readBinaryCompressedDataAdvanced(QFile& file, const PCDHeader& header, int xIndex, int yIndex, int zIndex);
    static QByteArray tryMultipleDecompressionMethods(const QByteArray& data, const PCDHeader& header);
    static QByteArray tryZlibDecompression(const QByteArray& data);
    static QByteArray tryLZ4Decompression(const QByteArray& data, const PCDHeader& header);
    static std::vector<QVector3D> intelligentRawDataParsing(const QByteArray& data, const PCDHeader& header, int xIndex, int yIndex, int zIndex);
    static bool validatePointCloud(const std::vector<QVector3D>& cloud);
    static std::vector<QVector3D> parseBinaryPointDataAdvanced(const QByteArray& data, const PCDHeader& header, int xIndex, int yIndex, int zIndex);
};

#endif // PCDREADER_H
