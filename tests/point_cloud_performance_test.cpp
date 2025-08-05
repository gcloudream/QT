#include <QtTest/QtTest>
#include <QObject>
#include <QElapsedTimer>
#include <QVector3D>
#include <memory>
#include "point_cloud_lod_manager.h"
#include "spatial_index.h"
#include "point_cloud_memory_manager.h"

class PointCloudPerformanceTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // LOD系统测试
    void testLODManagerCreation();
    void testLODLevelGeneration();
    void testLODLevelSelection();
    void testLODPerformance();
    
    // 空间索引测试
    void testOctreeCreation();
    void testOctreeInsertion();
    void testOctreeQuery();
    void testKDTreeCreation();
    void testKDTreeQuery();
    void testSpatialIndexPerformance();
    
    // 内存管理测试
    void testMemoryManagerCreation();
    void testChunkedLoading();
    void testMemoryUsageOptimization();
    void testProgressiveRendering();
    
    // 性能基准测试
    void testLargeDatasetPerformance();
    void testRealTimeInteraction();
    void testMemoryFootprint();

private:
    std::unique_ptr<WallExtraction::PointCloudLODManager> m_lodManager;
    std::unique_ptr<WallExtraction::SpatialIndex> m_spatialIndex;
    std::unique_ptr<WallExtraction::PointCloudMemoryManager> m_memoryManager;
    
    // 测试数据生成
    std::vector<QVector3D> generateTestPointCloud(int pointCount);
    std::vector<QVector3D> generateLargeTestPointCloud(int pointCount = 1000000);
    bool validatePerformanceRequirements(qint64 loadTime, qint64 renderTime);
};

void PointCloudPerformanceTest::initTestCase()
{
    qDebug() << "Starting PointCloud Performance test suite";
}

void PointCloudPerformanceTest::cleanupTestCase()
{
    qDebug() << "Finished PointCloud Performance test suite";
}

void PointCloudPerformanceTest::init()
{
    m_lodManager = std::make_unique<WallExtraction::PointCloudLODManager>();
    m_spatialIndex = std::make_unique<WallExtraction::SpatialIndex>();
    m_memoryManager = std::make_unique<WallExtraction::PointCloudMemoryManager>();
}

void PointCloudPerformanceTest::cleanup()
{
    m_memoryManager.reset();
    m_spatialIndex.reset();
    m_lodManager.reset();
}

void PointCloudPerformanceTest::testLODManagerCreation()
{
    QVERIFY(m_lodManager != nullptr);
    QVERIFY(m_lodManager->isInitialized());
    QCOMPARE(m_lodManager->getLODLevelCount(), 0); // 初始状态无LOD级别
}

void PointCloudPerformanceTest::testLODLevelGeneration()
{
    auto testPoints = generateTestPointCloud(10000);
    
    QElapsedTimer timer;
    timer.start();
    
    bool result = m_lodManager->generateLODLevels(testPoints);
    qint64 elapsed = timer.elapsed();
    
    QVERIFY(result);
    QVERIFY(m_lodManager->getLODLevelCount() > 0);
    QVERIFY(elapsed < 1000); // 应该在1秒内完成
    
    qDebug() << "LOD generation time:" << elapsed << "ms for" << testPoints.size() << "points";
}

void PointCloudPerformanceTest::testLODLevelSelection()
{
    auto testPoints = generateTestPointCloud(50000);
    m_lodManager->generateLODLevels(testPoints);
    
    // 测试不同距离下的LOD选择
    QList<float> distances = {1.0f, 10.0f, 100.0f, 1000.0f};
    
    for (float distance : distances) {
        int lodLevel = m_lodManager->selectLODLevel(distance);
        QVERIFY(lodLevel >= 0);
        QVERIFY(lodLevel < m_lodManager->getLODLevelCount());
        
        auto lodPoints = m_lodManager->getLODPoints(lodLevel);
        QVERIFY(!lodPoints.empty());
        
        qDebug() << "Distance:" << distance << "LOD Level:" << lodLevel << "Points:" << lodPoints.size();
    }
}

void PointCloudPerformanceTest::testLODPerformance()
{
    auto largePoints = generateLargeTestPointCloud(500000);
    
    QElapsedTimer timer;
    timer.start();
    
    m_lodManager->generateLODLevels(largePoints);
    qint64 lodGenTime = timer.elapsed();
    
    timer.restart();
    auto lodPoints = m_lodManager->getLODPoints(2); // 中等LOD级别
    qint64 lodRetrievalTime = timer.elapsed();
    
    QVERIFY(lodGenTime < 5000); // LOD生成应在5秒内完成
    QVERIFY(lodRetrievalTime < 100); // LOD检索应在100ms内完成
    QVERIFY(lodPoints.size() < largePoints.size()); // LOD应该减少点数
    
    qDebug() << "LOD generation:" << lodGenTime << "ms, retrieval:" << lodRetrievalTime << "ms";
}

void PointCloudPerformanceTest::testOctreeCreation()
{
    QVERIFY(m_spatialIndex->getIndexType() == WallExtraction::SpatialIndexType::Octree ||
            m_spatialIndex->getIndexType() == WallExtraction::SpatialIndexType::KDTree);
    
    auto testPoints = generateTestPointCloud(1000);
    
    QElapsedTimer timer;
    timer.start();
    
    bool result = m_spatialIndex->buildIndex(testPoints);
    qint64 elapsed = timer.elapsed();
    
    QVERIFY(result);
    QVERIFY(elapsed < 500); // 构建索引应在500ms内完成
    
    qDebug() << "Spatial index build time:" << elapsed << "ms for" << testPoints.size() << "points";
}

void PointCloudPerformanceTest::testOctreeInsertion()
{
    auto testPoints = generateTestPointCloud(5000);
    m_spatialIndex->buildIndex(testPoints);
    
    // 测试插入新点
    QVector3D newPoint(100.0f, 100.0f, 100.0f);
    
    QElapsedTimer timer;
    timer.start();
    
    bool result = m_spatialIndex->insertPoint(newPoint);
    qint64 elapsed = timer.elapsed();
    
    QVERIFY(result);
    QVERIFY(elapsed < 10); // 插入应在10ms内完成
}

void PointCloudPerformanceTest::testOctreeQuery()
{
    auto testPoints = generateTestPointCloud(10000);
    m_spatialIndex->buildIndex(testPoints);
    
    QVector3D queryPoint(50.0f, 50.0f, 50.0f);
    float radius = 10.0f;
    
    QElapsedTimer timer;
    timer.start();
    
    auto nearbyPoints = m_spatialIndex->queryRadius(queryPoint, radius);
    qint64 elapsed = timer.elapsed();
    
    QVERIFY(elapsed < 50); // 查询应在50ms内完成
    QVERIFY(!nearbyPoints.empty()); // 应该找到一些点
    
    qDebug() << "Radius query time:" << elapsed << "ms, found" << nearbyPoints.size() << "points";
}

void PointCloudPerformanceTest::testKDTreeCreation()
{
    // 如果支持KD树，测试其创建
    if (m_spatialIndex->supportsKDTree()) {
        m_spatialIndex->setIndexType(WallExtraction::SpatialIndexType::KDTree);
        
        auto testPoints = generateTestPointCloud(1000);
        
        QElapsedTimer timer;
        timer.start();
        
        bool result = m_spatialIndex->buildIndex(testPoints);
        qint64 elapsed = timer.elapsed();
        
        QVERIFY(result);
        QVERIFY(elapsed < 500);
        
        qDebug() << "KD-Tree build time:" << elapsed << "ms";
    } else {
        QSKIP("KD-Tree not supported in this implementation");
    }
}

void PointCloudPerformanceTest::testKDTreeQuery()
{
    if (m_spatialIndex->supportsKDTree()) {
        m_spatialIndex->setIndexType(WallExtraction::SpatialIndexType::KDTree);
        
        auto testPoints = generateTestPointCloud(10000);
        m_spatialIndex->buildIndex(testPoints);
        
        QVector3D queryPoint(25.0f, 25.0f, 25.0f);
        int k = 10;
        
        QElapsedTimer timer;
        timer.start();
        
        auto knnPoints = m_spatialIndex->queryKNN(queryPoint, k);
        qint64 elapsed = timer.elapsed();
        
        QVERIFY(elapsed < 20); // KNN查询应在20ms内完成
        QCOMPARE(knnPoints.size(), k);
        
        qDebug() << "KNN query time:" << elapsed << "ms for k=" << k;
    } else {
        QSKIP("KD-Tree not supported in this implementation");
    }
}

void PointCloudPerformanceTest::testSpatialIndexPerformance()
{
    auto largePoints = generateLargeTestPointCloud(100000);
    
    QElapsedTimer timer;
    timer.start();
    
    m_spatialIndex->buildIndex(largePoints);
    qint64 buildTime = timer.elapsed();
    
    // 执行多次查询测试
    timer.restart();
    for (int i = 0; i < 100; ++i) {
        QVector3D queryPoint(i * 10.0f, i * 10.0f, i * 10.0f);
        auto results = m_spatialIndex->queryRadius(queryPoint, 50.0f);
    }
    qint64 queryTime = timer.elapsed();
    
    QVERIFY(buildTime < 10000); // 构建应在10秒内完成
    QVERIFY(queryTime < 1000); // 100次查询应在1秒内完成
    
    qDebug() << "Large dataset - Build:" << buildTime << "ms, 100 queries:" << queryTime << "ms";
}

void PointCloudPerformanceTest::testMemoryManagerCreation()
{
    QVERIFY(m_memoryManager != nullptr);
    QVERIFY(m_memoryManager->isInitialized());
    QCOMPARE(m_memoryManager->getLoadedChunkCount(), 0);
}

void PointCloudPerformanceTest::testChunkedLoading()
{
    auto largePoints = generateLargeTestPointCloud(200000);
    
    QElapsedTimer timer;
    timer.start();
    
    bool result = m_memoryManager->loadPointCloudChunked(largePoints, 50000); // 50k点每块
    qint64 elapsed = timer.elapsed();
    
    QVERIFY(result);
    QVERIFY(m_memoryManager->getLoadedChunkCount() > 0);
    QVERIFY(elapsed < 3000); // 分块加载应在3秒内完成
    
    qDebug() << "Chunked loading time:" << elapsed << "ms for" << largePoints.size() << "points";
}

void PointCloudPerformanceTest::testMemoryUsageOptimization()
{
    auto largePoints = generateLargeTestPointCloud(500000);
    
    // 测试内存使用优化
    size_t initialMemory = m_memoryManager->getCurrentMemoryUsage();
    
    m_memoryManager->loadPointCloudChunked(largePoints, 100000);
    size_t loadedMemory = m_memoryManager->getCurrentMemoryUsage();
    
    m_memoryManager->optimizeMemoryUsage();
    size_t optimizedMemory = m_memoryManager->getCurrentMemoryUsage();
    
    QVERIFY(loadedMemory > initialMemory);
    QVERIFY(optimizedMemory <= loadedMemory); // 优化后内存使用应不增加
    
    qDebug() << "Memory usage - Initial:" << initialMemory 
             << "Loaded:" << loadedMemory 
             << "Optimized:" << optimizedMemory;
}

void PointCloudPerformanceTest::testProgressiveRendering()
{
    auto largePoints = generateLargeTestPointCloud(300000);
    m_memoryManager->loadPointCloudChunked(largePoints, 75000);
    
    QElapsedTimer timer;
    timer.start();
    
    // 测试渐进式渲染
    for (int level = 0; level < 4; ++level) {
        auto renderPoints = m_memoryManager->getPointsForRendering(level);
        QVERIFY(!renderPoints.empty());
        
        qint64 levelTime = timer.elapsed();
        QVERIFY(levelTime < 100); // 每级渲染准备应在100ms内完成
        
        timer.restart();
    }
}

void PointCloudPerformanceTest::testLargeDatasetPerformance()
{
    // 测试百万级点云的性能
    auto millionPoints = generateLargeTestPointCloud(1000000);
    
    QElapsedTimer timer;
    timer.start();
    
    // 测试LOD生成
    m_lodManager->generateLODLevels(millionPoints);
    qint64 lodTime = timer.elapsed();
    
    timer.restart();
    
    // 测试空间索引构建
    m_spatialIndex->buildIndex(millionPoints);
    qint64 indexTime = timer.elapsed();
    
    timer.restart();
    
    // 测试分块加载
    m_memoryManager->loadPointCloudChunked(millionPoints, 200000);
    qint64 loadTime = timer.elapsed();
    
    // 验证性能要求
    QVERIFY(validatePerformanceRequirements(loadTime, 0));
    
    qDebug() << "Million points performance - LOD:" << lodTime 
             << "ms, Index:" << indexTime 
             << "ms, Load:" << loadTime << "ms";
}

void PointCloudPerformanceTest::testRealTimeInteraction()
{
    auto testPoints = generateTestPointCloud(100000);
    m_spatialIndex->buildIndex(testPoints);
    m_lodManager->generateLODLevels(testPoints);
    
    // 模拟实时交互查询
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < 60; ++i) { // 模拟60帧
        // 模拟视点变化
        QVector3D viewPoint(i * 5.0f, i * 5.0f, 100.0f);
        float distance = viewPoint.length();
        
        // LOD选择
        int lodLevel = m_lodManager->selectLODLevel(distance);
        auto lodPoints = m_lodManager->getLODPoints(lodLevel);
        
        // 空间查询
        auto visiblePoints = m_spatialIndex->queryRadius(viewPoint, 200.0f);
    }
    
    qint64 elapsed = timer.elapsed();
    float fps = 60000.0f / elapsed; // 计算FPS
    
    QVERIFY(fps >= 30.0f); // 应该维持30FPS以上
    
    qDebug() << "Real-time interaction test - FPS:" << fps;
}

void PointCloudPerformanceTest::testMemoryFootprint()
{
    auto testPoints = generateTestPointCloud(50000);
    
    size_t initialMemory = m_memoryManager->getCurrentMemoryUsage();
    
    m_memoryManager->loadPointCloudChunked(testPoints, 10000);
    size_t afterLoadMemory = m_memoryManager->getCurrentMemoryUsage();
    
    size_t memoryIncrease = afterLoadMemory - initialMemory;
    size_t expectedMemory = testPoints.size() * sizeof(QVector3D);
    
    // 内存使用应该在合理范围内（不超过预期的150%）
    QVERIFY(memoryIncrease <= expectedMemory * 1.5);
    
    qDebug() << "Memory footprint - Expected:" << expectedMemory 
             << "Actual:" << memoryIncrease 
             << "Ratio:" << (float)memoryIncrease / expectedMemory;
}

// 辅助方法实现
std::vector<QVector3D> PointCloudPerformanceTest::generateTestPointCloud(int pointCount)
{
    std::vector<QVector3D> points;
    points.reserve(pointCount);
    
    for (int i = 0; i < pointCount; ++i) {
        float x = (i % 100) * 1.0f;
        float y = ((i / 100) % 100) * 1.0f;
        float z = (i / 10000) * 1.0f;
        points.emplace_back(x, y, z);
    }
    
    return points;
}

std::vector<QVector3D> PointCloudPerformanceTest::generateLargeTestPointCloud(int pointCount)
{
    std::vector<QVector3D> points;
    points.reserve(pointCount);
    
    for (int i = 0; i < pointCount; ++i) {
        float x = (qrand() % 10000) * 0.1f;
        float y = (qrand() % 10000) * 0.1f;
        float z = (qrand() % 1000) * 0.1f;
        points.emplace_back(x, y, z);
    }
    
    return points;
}

bool PointCloudPerformanceTest::validatePerformanceRequirements(qint64 loadTime, qint64 renderTime)
{
    // 性能要求：百万级点云加载时间<5秒，交互帧率>30fps
    return loadTime < 5000; // && renderTime < 33; // 33ms = 30fps
}

QTEST_MAIN(PointCloudPerformanceTest)
#include "point_cloud_performance_test.moc"
