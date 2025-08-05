#include <QtTest/QtTest>
#include <QObject>
#include <QTemporaryFile>
#include <QVector3D>
#include <memory>
#include "las_reader.h"

class LASReaderTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基础功能测试
    void testReaderCreation();
    void testLASFormatDetection();
    void testLAZFormatDetection();
    void testInvalidFileHandling();
    
    // LAS文件解析测试
    void testLASHeaderParsing();
    void testLASPointDataParsing();
    void testLASVersionSupport();
    void testLASPointRecordFormats();
    
    // 坐标系统测试
    void testCoordinateSystemParsing();
    void testWKTCoordinateSystem();
    void testUTMCoordinateSystem();
    void testCoordinateTransformation();
    
    // 属性信息测试
    void testClassificationParsing();
    void testIntensityParsing();
    void testRGBColorParsing();
    void testExtendedAttributes();
    
    // 性能测试
    void testLargeFileHandling();
    void testMemoryUsage();

private:
    std::unique_ptr<WallExtraction::LASReader> m_reader;
    QString m_testDataDir;
    
    // 辅助方法
    void createTestLASFile(const QString& filename, int pointCount = 1000);
    void createTestLAZFile(const QString& filename, int pointCount = 1000);
    bool validatePointCloud(const std::vector<QVector3D>& points);
};

void LASReaderTest::initTestCase()
{
    qDebug() << "Starting LASReader test suite";
    m_testDataDir = QDir::tempPath() + "/las_test_data";
    QDir().mkpath(m_testDataDir);
}

void LASReaderTest::cleanupTestCase()
{
    // 清理测试数据
    QDir testDir(m_testDataDir);
    testDir.removeRecursively();
    qDebug() << "Finished LASReader test suite";
}

void LASReaderTest::init()
{
    m_reader = std::make_unique<WallExtraction::LASReader>();
}

void LASReaderTest::cleanup()
{
    m_reader.reset();
}

void LASReaderTest::testReaderCreation()
{
    QVERIFY(m_reader != nullptr);
    QVERIFY(m_reader->getSupportedFormats().contains("las"));
    QVERIFY(m_reader->getSupportedFormats().contains("laz"));
}

void LASReaderTest::testLASFormatDetection()
{
    QString testFile = m_testDataDir + "/test.las";
    createTestLASFile(testFile);
    
    QVERIFY(m_reader->canReadFile(testFile));
    QCOMPARE(m_reader->detectFormat(testFile), QString("las"));
}

void LASReaderTest::testLAZFormatDetection()
{
    QString testFile = m_testDataDir + "/test.laz";
    createTestLAZFile(testFile);
    
    QVERIFY(m_reader->canReadFile(testFile));
    QCOMPARE(m_reader->detectFormat(testFile), QString("laz"));
}

void LASReaderTest::testInvalidFileHandling()
{
    QString invalidFile = m_testDataDir + "/invalid.txt";
    QFile file(invalidFile);
    file.open(QIODevice::WriteOnly);
    file.write("This is not a LAS file");
    file.close();
    
    QVERIFY(!m_reader->canReadFile(invalidFile));
    
    // 测试异常处理
    try {
        auto points = m_reader->readPointCloud(invalidFile);
        QFAIL("Expected LASReaderException was not thrown");
    } catch (const WallExtraction::LASReaderException& e) {
        QVERIFY(QString(e.what()).contains("Invalid LAS file"));
    }
}

void LASReaderTest::testLASHeaderParsing()
{
    QString testFile = m_testDataDir + "/header_test.las";
    createTestLASFile(testFile, 500);
    
    auto header = m_reader->parseHeader(testFile);
    QVERIFY(header.isValid());
    QCOMPARE(header.pointCount, 500);
    QVERIFY(header.version.major >= 1);
    QVERIFY(header.version.minor >= 2);
}

void LASReaderTest::testLASPointDataParsing()
{
    QString testFile = m_testDataDir + "/points_test.las";
    createTestLASFile(testFile, 100);
    
    auto points = m_reader->readPointCloud(testFile);
    QCOMPARE(points.size(), 100);
    QVERIFY(validatePointCloud(points));
}

void LASReaderTest::testLASVersionSupport()
{
    // 测试LAS 1.2支持
    QVERIFY(m_reader->supportsVersion(1, 2));
    
    // 测试LAS 1.3支持
    QVERIFY(m_reader->supportsVersion(1, 3));
    
    // 测试LAS 1.4支持
    QVERIFY(m_reader->supportsVersion(1, 4));
    
    // 测试不支持的版本
    QVERIFY(!m_reader->supportsVersion(2, 0));
}

void LASReaderTest::testLASPointRecordFormats()
{
    // 测试不同的点记录格式支持
    QList<int> supportedFormats = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    for (int format : supportedFormats) {
        QVERIFY2(m_reader->supportsPointRecordFormat(format), 
                QString("Point record format %1 should be supported").arg(format).toLocal8Bit());
    }
}

void LASReaderTest::testCoordinateSystemParsing()
{
    QString testFile = m_testDataDir + "/coord_test.las";
    createTestLASFile(testFile);
    
    auto coordSystem = m_reader->parseCoordinateSystem(testFile);
    QVERIFY(coordSystem.isValid());
    QVERIFY(!coordSystem.wktString.isEmpty() || coordSystem.epsgCode > 0);
}

void LASReaderTest::testWKTCoordinateSystem()
{
    // 这里需要创建包含WKT坐标系统定义的测试文件
    // 由于复杂性，先创建基础测试框架
    QVERIFY(m_reader->supportsWKT());
}

void LASReaderTest::testUTMCoordinateSystem()
{
    // 测试UTM坐标系统支持
    QVERIFY(m_reader->supportsUTM());
}

void LASReaderTest::testCoordinateTransformation()
{
    // 测试坐标系统转换功能
    QVector3D sourcePoint(100000.0, 200000.0, 100.0);
    
    // 从UTM转换到地理坐标系
    auto transformed = m_reader->transformCoordinates(sourcePoint, 
                                                     WallExtraction::CoordinateSystem::UTM_Zone33N,
                                                     WallExtraction::CoordinateSystem::WGS84);
    
    QVERIFY(transformed.x() != sourcePoint.x());
    QVERIFY(transformed.y() != sourcePoint.y());
}

void LASReaderTest::testClassificationParsing()
{
    QString testFile = m_testDataDir + "/classification_test.las";
    createTestLASFile(testFile);
    
    auto pointsWithAttributes = m_reader->readPointCloudWithAttributes(testFile);
    QVERIFY(!pointsWithAttributes.empty());
    
    // 验证分类信息
    for (const auto& point : pointsWithAttributes) {
        QVERIFY(point.attributes.contains("classification"));
        int classification = point.attributes["classification"].toInt();
        QVERIFY(classification >= 0 && classification <= 255);
    }
}

void LASReaderTest::testIntensityParsing()
{
    QString testFile = m_testDataDir + "/intensity_test.las";
    createTestLASFile(testFile);
    
    auto pointsWithAttributes = m_reader->readPointCloudWithAttributes(testFile);
    
    // 验证强度信息
    for (const auto& point : pointsWithAttributes) {
        QVERIFY(point.attributes.contains("intensity"));
        int intensity = point.attributes["intensity"].toInt();
        QVERIFY(intensity >= 0 && intensity <= 65535);
    }
}

void LASReaderTest::testRGBColorParsing()
{
    QString testFile = m_testDataDir + "/rgb_test.las";
    createTestLASFile(testFile);
    
    auto pointsWithAttributes = m_reader->readPointCloudWithAttributes(testFile);
    
    // 验证RGB颜色信息
    for (const auto& point : pointsWithAttributes) {
        if (point.attributes.contains("red")) {
            QVERIFY(point.attributes.contains("green"));
            QVERIFY(point.attributes.contains("blue"));
            
            int red = point.attributes["red"].toInt();
            int green = point.attributes["green"].toInt();
            int blue = point.attributes["blue"].toInt();
            
            QVERIFY(red >= 0 && red <= 65535);
            QVERIFY(green >= 0 && green <= 65535);
            QVERIFY(blue >= 0 && blue <= 65535);
        }
    }
}

void LASReaderTest::testExtendedAttributes()
{
    // 测试扩展属性解析
    QString testFile = m_testDataDir + "/extended_test.las";
    createTestLASFile(testFile);
    
    auto attributes = m_reader->getAvailableAttributes(testFile);
    
    // 基础属性应该总是存在
    QVERIFY(attributes.contains("x"));
    QVERIFY(attributes.contains("y"));
    QVERIFY(attributes.contains("z"));
}

void LASReaderTest::testLargeFileHandling()
{
    // 测试大文件处理能力
    QString largeFile = m_testDataDir + "/large_test.las";
    createTestLASFile(largeFile, 100000); // 10万个点
    
    QElapsedTimer timer;
    timer.start();
    
    auto points = m_reader->readPointCloud(largeFile);
    
    qint64 elapsed = timer.elapsed();
    qDebug() << "Large file processing time:" << elapsed << "ms";
    
    QCOMPARE(points.size(), 100000);
    QVERIFY(elapsed < 10000); // 应该在10秒内完成
}

void LASReaderTest::testMemoryUsage()
{
    // 测试内存使用情况
    QString testFile = m_testDataDir + "/memory_test.las";
    createTestLASFile(testFile, 10000);
    
    // 这里可以添加内存使用监控
    auto points = m_reader->readPointCloud(testFile);
    QCOMPARE(points.size(), 10000);
}

// 辅助方法实现
void LASReaderTest::createTestLASFile(const QString& filename, int pointCount)
{
    // 创建简单的LAS文件用于测试
    // 这里创建一个最小的有效LAS文件结构
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    
    // 写入LAS文件头（简化版本）
    QByteArray header(227, 0); // LAS 1.2 header size
    header[0] = 'L'; header[1] = 'A'; header[2] = 'S'; header[3] = 'F'; // 文件签名
    header[24] = 1; header[25] = 2; // 版本 1.2
    // 设置点数量
    *reinterpret_cast<quint32*>(header.data() + 107) = pointCount;
    
    file.write(header);
    
    // 写入点数据（简化版本）
    for (int i = 0; i < pointCount; ++i) {
        QByteArray pointData(20, 0); // 点记录格式0的大小
        // 设置X, Y, Z坐标
        *reinterpret_cast<qint32*>(pointData.data() + 0) = i * 100;
        *reinterpret_cast<qint32*>(pointData.data() + 4) = i * 100;
        *reinterpret_cast<qint32*>(pointData.data() + 8) = i * 10;
        file.write(pointData);
    }
    
    file.close();
}

void LASReaderTest::createTestLAZFile(const QString& filename, int pointCount)
{
    // LAZ文件是压缩的LAS文件，这里创建一个简单的测试文件
    // 实际实现中需要使用LASzip库
    createTestLASFile(filename, pointCount);
}

bool LASReaderTest::validatePointCloud(const std::vector<QVector3D>& points)
{
    if (points.empty()) return false;
    
    for (const auto& point : points) {
        if (std::isnan(point.x()) || std::isnan(point.y()) || std::isnan(point.z())) {
            return false;
        }
    }
    
    return true;
}

QTEST_MAIN(LASReaderTest)
#include "las_reader_test.moc"
