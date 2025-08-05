#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <memory>
#include "../src/wall_extraction/point_cloud_lod_manager.h"
#include "../src/wall_extraction/spatial_index.h"
#include "../src/wall_extraction/point_cloud_memory_manager.h"

/**
 * 手动测试程序，验证T1.3任务的完成情况
 * 测试点云渲染性能优化功能
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== T1.3 Manual Test: Point Cloud Rendering Performance Optimization ===";
    
    bool allTestsPassed = true;
    
    // 生成测试数据
    auto generateTestPoints = [](int count) {
        std::vector<QVector3D> points;
        points.reserve(count);
        for (int i = 0; i < count; ++i) {
            float x = (i % 1000) * 0.1f;
            float y = ((i / 1000) % 1000) * 0.1f;
            float z = (i / 1000000) * 0.1f;
            points.emplace_back(x, y, z);
        }
        return points;
    };
    
    try {
        // 测试1: LOD管理器创建和基础功能
        qDebug() << "\n1. Testing PointCloudLODManager...";
        auto lodManager = std::make_unique<WallExtraction::PointCloudLODManager>();
        if (lodManager && lodManager->isInitialized()) {
            qDebug() << "✓ LOD Manager created and initialized";
        } else {
            qDebug() << "✗ Failed to create LOD Manager";
            allTestsPassed = false;
        }
        
        // 测试LOD级别生成
        auto testPoints = generateTestPoints(10000);
        QElapsedTimer timer;
        timer.start();
        
        bool lodResult = lodManager->generateLODLevels(testPoints);
        qint64 lodTime = timer.elapsed();
        
        if (lodResult && lodTime < 2000) { // 应在2秒内完成
            qDebug() << "✓ LOD levels generated successfully in" << lodTime << "ms";
            qDebug() << "  - LOD level count:" << lodManager->getLODLevelCount();
            qDebug() << "  - Memory usage:" << (lodManager->getTotalMemoryUsage() / 1024) << "KB";
        } else {
            qDebug() << "✗ LOD generation failed or too slow (" << lodTime << "ms)";
            allTestsPassed = false;
        }
        
        // 测试LOD级别选择
        QList<float> testDistances = {5.0f, 25.0f, 100.0f, 500.0f};
        for (float distance : testDistances) {
            int lodLevel = lodManager->selectLODLevel(distance);
            auto lodPoints = lodManager->getLODPoints(lodLevel);
            qDebug() << "  - Distance" << distance << "-> LOD level" << lodLevel 
                     << "(" << lodPoints.size() << "points)";
        }
        
        // 测试2: 空间索引创建和查询
        qDebug() << "\n2. Testing SpatialIndex...";
        auto spatialIndex = std::make_unique<WallExtraction::SpatialIndex>();
        if (spatialIndex) {
            qDebug() << "✓ Spatial Index created";
        } else {
            qDebug() << "✗ Failed to create Spatial Index";
            allTestsPassed = false;
        }
        
        // 测试八叉树构建
        timer.restart();
        bool indexResult = spatialIndex->buildIndex(testPoints);
        qint64 indexTime = timer.elapsed();
        
        if (indexResult && indexTime < 1000) { // 应在1秒内完成
            qDebug() << "✓ Octree index built successfully in" << indexTime << "ms";
            qDebug() << "  - Point count:" << spatialIndex->getPointCount();
            qDebug() << "  - Index type:" << (spatialIndex->getIndexType() == WallExtraction::SpatialIndexType::Octree ? "Octree" : "KDTree");
        } else {
            qDebug() << "✗ Index building failed or too slow (" << indexTime << "ms)";
            allTestsPassed = false;
        }
        
        // 测试范围查询
        QVector3D queryCenter(50.0f, 50.0f, 0.5f);
        float queryRadius = 10.0f;
        
        timer.restart();
        auto radiusResults = spatialIndex->queryRadius(queryCenter, queryRadius);
        qint64 queryTime = timer.elapsed();
        
        if (queryTime < 50 && !radiusResults.empty()) { // 应在50ms内完成
            qDebug() << "✓ Radius query completed in" << queryTime << "ms, found" << radiusResults.size() << "points";
        } else {
            qDebug() << "✗ Radius query failed or too slow (" << queryTime << "ms)";
            allTestsPassed = false;
        }
        
        // 测试KNN查询
        if (spatialIndex->supportsKDTree()) {
            spatialIndex->setIndexType(WallExtraction::SpatialIndexType::KDTree);
            spatialIndex->buildIndex(testPoints);
            
            timer.restart();
            auto knnResults = spatialIndex->queryKNN(queryCenter, 10);
            qint64 knnTime = timer.elapsed();
            
            if (knnTime < 30 && knnResults.size() == 10) { // 应在30ms内完成
                qDebug() << "✓ KNN query completed in" << knnTime << "ms, found" << knnResults.size() << "neighbors";
            } else {
                qDebug() << "✗ KNN query failed or too slow (" << knnTime << "ms)";
                allTestsPassed = false;
            }
        }
        
        // 测试3: 内存管理器
        qDebug() << "\n3. Testing PointCloudMemoryManager...";
        auto memoryManager = std::make_unique<WallExtraction::PointCloudMemoryManager>();
        if (memoryManager && memoryManager->isInitialized()) {
            qDebug() << "✓ Memory Manager created and initialized";
            qDebug() << "  - Max memory:" << memoryManager->getMaxMemoryUsage() << "MB";
        } else {
            qDebug() << "✗ Failed to create Memory Manager";
            allTestsPassed = false;
        }
        
        // 测试分块加载
        auto largePoints = generateTestPoints(100000);
        timer.restart();
        bool chunkResult = memoryManager->loadPointCloudChunked(largePoints, 25000);
        qint64 chunkTime = timer.elapsed();
        
        if (chunkResult && chunkTime < 3000) { // 应在3秒内完成
            qDebug() << "✓ Chunked loading completed in" << chunkTime << "ms";
            qDebug() << "  - Total chunks:" << memoryManager->getTotalChunkCount();
            qDebug() << "  - Loaded chunks:" << memoryManager->getLoadedChunkCount();
            qDebug() << "  - Memory usage:" << (memoryManager->getCurrentMemoryUsage() / (1024*1024)) << "MB";
        } else {
            qDebug() << "✗ Chunked loading failed or too slow (" << chunkTime << "ms)";
            allTestsPassed = false;
        }
        
        // 测试渐进式渲染
        for (int lodLevel = 0; lodLevel < 4; ++lodLevel) {
            timer.restart();
            auto renderPoints = memoryManager->getPointsForRendering(lodLevel);
            qint64 renderTime = timer.elapsed();
            
            if (renderTime < 100) { // 应在100ms内完成
                qDebug() << "✓ LOD" << lodLevel << "rendering data prepared in" << renderTime 
                         << "ms (" << renderPoints.size() << "points)";
            } else {
                qDebug() << "✗ LOD" << lodLevel << "rendering preparation too slow (" << renderTime << "ms)";
                allTestsPassed = false;
            }
        }
        
        // 测试内存优化
        timer.restart();
        bool optimizeResult = memoryManager->optimizeMemoryUsage();
        qint64 optimizeTime = timer.elapsed();
        
        if (optimizeResult && optimizeTime < 500) { // 应在500ms内完成
            qDebug() << "✓ Memory optimization completed in" << optimizeTime << "ms";
            qDebug() << "  - Memory after optimization:" << (memoryManager->getCurrentMemoryUsage() / (1024*1024)) << "MB";
        } else {
            qDebug() << "✗ Memory optimization failed or too slow (" << optimizeTime << "ms)";
            allTestsPassed = false;
        }
        
        // 测试4: 性能基准测试
        qDebug() << "\n4. Performance Benchmark Tests...";
        
        // 大数据量测试
        auto millionPoints = generateTestPoints(1000000);
        
        // LOD生成性能
        timer.restart();
        lodManager->generateLODLevels(millionPoints);
        qint64 largeLodTime = timer.elapsed();
        
        if (largeLodTime < 10000) { // 应在10秒内完成
            qDebug() << "✓ Large dataset LOD generation:" << largeLodTime << "ms for 1M points";
        } else {
            qDebug() << "✗ Large dataset LOD generation too slow:" << largeLodTime << "ms";
            allTestsPassed = false;
        }
        
        // 空间索引性能
        timer.restart();
        spatialIndex->buildIndex(millionPoints);
        qint64 largeIndexTime = timer.elapsed();
        
        if (largeIndexTime < 15000) { // 应在15秒内完成
            qDebug() << "✓ Large dataset index building:" << largeIndexTime << "ms for 1M points";
        } else {
            qDebug() << "✗ Large dataset index building too slow:" << largeIndexTime << "ms";
            allTestsPassed = false;
        }
        
        // 内存管理性能
        timer.restart();
        memoryManager->loadPointCloudChunked(millionPoints, 200000);
        qint64 largeMemoryTime = timer.elapsed();
        
        if (largeMemoryTime < 5000) { // 应在5秒内完成
            qDebug() << "✓ Large dataset chunked loading:" << largeMemoryTime << "ms for 1M points";
        } else {
            qDebug() << "✗ Large dataset chunked loading too slow:" << largeMemoryTime << "ms";
            allTestsPassed = false;
        }
        
        // 测试5: 实时交互性能
        qDebug() << "\n5. Real-time Interaction Performance...";
        
        // 模拟60帧的实时查询
        timer.restart();
        for (int frame = 0; frame < 60; ++frame) {
            QVector3D viewPos(frame * 5.0f, frame * 5.0f, 100.0f);
            float distance = viewPos.length();
            
            // LOD选择
            int lodLevel = lodManager->selectLODLevel(distance);
            auto lodPoints = lodManager->getLODPoints(lodLevel);
            
            // 空间查询
            auto visiblePoints = spatialIndex->queryRadius(viewPos, 200.0f);
        }
        qint64 interactionTime = timer.elapsed();
        float fps = 60000.0f / interactionTime;
        
        if (fps >= 30.0f) { // 应维持30FPS以上
            qDebug() << "✓ Real-time interaction performance:" << fps << "FPS";
        } else {
            qDebug() << "✗ Real-time interaction performance too low:" << fps << "FPS";
            allTestsPassed = false;
        }
        
        // 测试6: 内存使用验证
        qDebug() << "\n6. Memory Usage Validation...";
        
        auto memStats = memoryManager->getMemoryStatistics();
        qDebug() << "Memory Statistics:";
        for (auto it = memStats.begin(); it != memStats.end(); ++it) {
            qDebug() << "  -" << it.key() << ":" << it.value();
        }
        
        auto indexStats = spatialIndex->getIndexStatistics();
        qDebug() << "Index Statistics:";
        for (auto it = indexStats.begin(); it != indexStats.end(); ++it) {
            qDebug() << "  -" << it.key() << ":" << it.value();
        }
        
    } catch (const std::exception& e) {
        qDebug() << "✗ Unexpected exception:" << e.what();
        allTestsPassed = false;
    }
    
    // 测试结果总结
    qDebug() << "\n=== Test Results ===";
    if (allTestsPassed) {
        qDebug() << "✓ All tests PASSED! T1.3 task completed successfully.";
        qDebug() << "✓ Point cloud LOD system implemented";
        qDebug() << "✓ Spatial index structures (Octree/KDTree) added";
        qDebug() << "✓ Large dataset memory management optimized";
        qDebug() << "✓ Chunked loading and progressive rendering implemented";
        qDebug() << "✓ Performance requirements met (load time <5s, interaction >30fps)";
        return 0;
    } else {
        qDebug() << "✗ Some tests FAILED. Please review the implementation.";
        return 1;
    }
}
