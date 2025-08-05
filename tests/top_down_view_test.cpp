#include <QtTest/QtTest>
#include <QObject>
#include <QVector3D>
#include <QColor>
#include <memory>
#include "top_down_view_renderer.h"
#include "color_mapping_manager.h"
#include "view_projection_manager.h"

class TopDownViewTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 投影算法测试
    void testProjectionManagerCreation();
    void testOrthographicProjection();
    void testViewportTransformation();
    void testDepthBuffering();
    
    // 颜色映射测试
    void testColorMappingCreation();
    void testHeightBasedMapping();
    void testIntensityBasedMapping();
    void testCustomColorSchemes();
    void testColorInterpolation();
    
    // 渲染器测试
    void testRendererCreation();
    void testTopDownRendering();
    void testRenderingPerformance();
    void testInteractiveControls();
    
    // 交互控制测试
    void testPanControl();
    void testZoomControl();
    void testSelectionTool();
    void testMeasurementTool();

private:
    std::unique_ptr<WallExtraction::TopDownViewRenderer> m_renderer;
    std::unique_ptr<WallExtraction::ColorMappingManager> m_colorMapper;
    std::unique_ptr<WallExtraction::ViewProjectionManager> m_projectionManager;
    
    // 测试数据生成
    std::vector<QVector3D> generateBuildingPointCloud();
    std::vector<WallExtraction::PointWithAttributes> generatePointsWithAttributes();
};

void TopDownViewTest::initTestCase()
{
    qDebug() << "Starting TopDownView test suite";
}

void TopDownViewTest::cleanupTestCase()
{
    qDebug() << "Finished TopDownView test suite";
}

void TopDownViewTest::init()
{
    m_renderer = std::make_unique<WallExtraction::TopDownViewRenderer>();
    m_colorMapper = std::make_unique<WallExtraction::ColorMappingManager>();
    m_projectionManager = std::make_unique<WallExtraction::ViewProjectionManager>();
}

void TopDownViewTest::cleanup()
{
    m_projectionManager.reset();
    m_colorMapper.reset();
    m_renderer.reset();
}

void TopDownViewTest::testProjectionManagerCreation()
{
    QVERIFY(m_projectionManager != nullptr);
    QVERIFY(m_projectionManager->isInitialized());
    QCOMPARE(m_projectionManager->getProjectionType(), WallExtraction::ProjectionType::Orthographic);
}

void TopDownViewTest::testOrthographicProjection()
{
    auto testPoints = generateBuildingPointCloud();
    
    // 设置投影参数
    m_projectionManager->setViewBounds(-100.0f, 100.0f, -100.0f, 100.0f);
    m_projectionManager->setViewportSize(800, 600);
    
    // 执行投影
    auto projectedPoints = m_projectionManager->projectToTopDown(testPoints);
    
    QVERIFY(!projectedPoints.empty());
    QCOMPARE(projectedPoints.size(), testPoints.size());
    
    // 验证投影结果在视口范围内
    for (const auto& point : projectedPoints) {
        QVERIFY(point.x() >= 0 && point.x() <= 800);
        QVERIFY(point.y() >= 0 && point.y() <= 600);
    }
}

void TopDownViewTest::testViewportTransformation()
{
    QVector3D worldPoint(50.0f, 30.0f, 10.0f);
    
    m_projectionManager->setViewBounds(0.0f, 100.0f, 0.0f, 100.0f);
    m_projectionManager->setViewportSize(1000, 1000);
    
    auto screenPoint = m_projectionManager->worldToScreen(worldPoint);
    
    // 验证变换结果
    QCOMPARE(screenPoint.x(), 500.0f); // 50% of 1000
    QCOMPARE(screenPoint.y(), 300.0f); // 30% of 1000
    
    // 测试逆变换
    auto backToWorld = m_projectionManager->screenToWorld(screenPoint, 10.0f);
    QVERIFY(qAbs(backToWorld.x() - worldPoint.x()) < 0.1f);
    QVERIFY(qAbs(backToWorld.y() - worldPoint.y()) < 0.1f);
}

void TopDownViewTest::testDepthBuffering()
{
    // 创建有高度差的测试点
    std::vector<QVector3D> layeredPoints = {
        QVector3D(0, 0, 0),    // 地面
        QVector3D(0, 0, 5),    // 中层
        QVector3D(0, 0, 10)    // 顶层
    };
    
    m_projectionManager->enableDepthTesting(true);
    auto result = m_projectionManager->projectWithDepth(layeredPoints);
    
    QVERIFY(!result.empty());
    // 验证深度排序正确
    QVERIFY(result[0].depth < result[1].depth);
    QVERIFY(result[1].depth < result[2].depth);
}

void TopDownViewTest::testColorMappingCreation()
{
    QVERIFY(m_colorMapper != nullptr);
    QVERIFY(m_colorMapper->isInitialized());
    
    // 测试默认颜色方案
    auto schemes = m_colorMapper->getAvailableColorSchemes();
    QVERIFY(schemes.contains("Height"));
    QVERIFY(schemes.contains("Intensity"));
    QVERIFY(schemes.contains("Classification"));
}

void TopDownViewTest::testHeightBasedMapping()
{
    auto testPoints = generatePointsWithAttributes();
    
    // 设置高度映射
    m_colorMapper->setColorScheme(WallExtraction::ColorScheme::Height);
    m_colorMapper->setValueRange(0.0f, 20.0f); // 0-20米高度范围
    
    auto coloredPoints = m_colorMapper->applyColorMapping(testPoints);
    
    QCOMPARE(coloredPoints.size(), testPoints.size());
    
    // 验证颜色映射
    for (size_t i = 0; i < coloredPoints.size(); ++i) {
        QVERIFY(coloredPoints[i].color.isValid());
        
        // 高度越高，颜色应该越"热"（更偏红）
        float height = testPoints[i].position.z();
        if (height > 15.0f) {
            QVERIFY(coloredPoints[i].color.red() > 200); // 高处应该偏红
        } else if (height < 5.0f) {
            QVERIFY(coloredPoints[i].color.blue() > 200); // 低处应该偏蓝
        }
    }
}

void TopDownViewTest::testIntensityBasedMapping()
{
    auto testPoints = generatePointsWithAttributes();
    
    // 设置强度映射
    m_colorMapper->setColorScheme(WallExtraction::ColorScheme::Intensity);
    m_colorMapper->setValueRange(0.0f, 65535.0f); // LAS强度范围
    
    auto coloredPoints = m_colorMapper->applyColorMapping(testPoints);
    
    QCOMPARE(coloredPoints.size(), testPoints.size());
    
    // 验证强度映射
    for (const auto& point : coloredPoints) {
        QVERIFY(point.color.isValid());
        // 强度映射应该产生灰度值
        int gray = (point.color.red() + point.color.green() + point.color.blue()) / 3;
        QVERIFY(qAbs(point.color.red() - gray) < 50);
    }
}

void TopDownViewTest::testCustomColorSchemes()
{
    // 创建自定义颜色方案
    WallExtraction::ColorSchemeDefinition customScheme;
    customScheme.name = "Custom";
    customScheme.colors = {
        {0.0f, QColor(Qt::blue)},
        {0.5f, QColor(Qt::green)},
        {1.0f, QColor(Qt::red)}
    };
    
    bool result = m_colorMapper->addCustomColorScheme(customScheme);
    QVERIFY(result);
    
    auto schemes = m_colorMapper->getAvailableColorSchemes();
    QVERIFY(schemes.contains("Custom"));
    
    // 测试自定义方案
    m_colorMapper->setColorScheme("Custom");
    auto testPoints = generatePointsWithAttributes();
    auto coloredPoints = m_colorMapper->applyColorMapping(testPoints);
    
    QVERIFY(!coloredPoints.empty());
}

void TopDownViewTest::testColorInterpolation()
{
    m_colorMapper->setColorScheme(WallExtraction::ColorScheme::Height);
    m_colorMapper->setValueRange(0.0f, 10.0f);
    
    // 测试中间值的颜色插值
    QColor color1 = m_colorMapper->getColorForValue(2.5f);  // 25%
    QColor color2 = m_colorMapper->getColorForValue(5.0f);  // 50%
    QColor color3 = m_colorMapper->getColorForValue(7.5f);  // 75%
    
    QVERIFY(color1.isValid());
    QVERIFY(color2.isValid());
    QVERIFY(color3.isValid());
    
    // 验证颜色渐变
    QVERIFY(color1 != color2);
    QVERIFY(color2 != color3);
}

void TopDownViewTest::testRendererCreation()
{
    QVERIFY(m_renderer != nullptr);
    QVERIFY(m_renderer->isInitialized());
    
    // 验证默认设置
    QCOMPARE(m_renderer->getRenderMode(), WallExtraction::TopDownRenderMode::Points);
    QVERIFY(m_renderer->getViewportSize().width() > 0);
    QVERIFY(m_renderer->getViewportSize().height() > 0);
}

void TopDownViewTest::testTopDownRendering()
{
    auto testPoints = generatePointsWithAttributes();
    
    // 设置渲染参数
    m_renderer->setViewBounds(-50.0f, 50.0f, -50.0f, 50.0f);
    m_renderer->setPointSize(2.0f);
    m_renderer->setRenderMode(WallExtraction::TopDownRenderMode::Points);
    
    QElapsedTimer timer;
    timer.start();
    
    bool result = m_renderer->renderTopDownView(testPoints);
    qint64 renderTime = timer.elapsed();
    
    QVERIFY(result);
    QVERIFY(renderTime < 100); // 应在100ms内完成
    
    // 验证渲染结果
    auto renderBuffer = m_renderer->getRenderBuffer();
    QVERIFY(!renderBuffer.isNull());
    QVERIFY(renderBuffer.width() > 0);
    QVERIFY(renderBuffer.height() > 0);
}

void TopDownViewTest::testRenderingPerformance()
{
    // 生成大量测试数据
    std::vector<WallExtraction::PointWithAttributes> largeDataset;
    largeDataset.reserve(100000);
    
    for (int i = 0; i < 100000; ++i) {
        WallExtraction::PointWithAttributes point;
        point.position = QVector3D(i % 1000 * 0.1f, (i / 1000) * 0.1f, (i % 100) * 0.1f);
        point.attributes["intensity"] = i % 65536;
        largeDataset.push_back(point);
    }
    
    QElapsedTimer timer;
    timer.start();
    
    bool result = m_renderer->renderTopDownView(largeDataset);
    qint64 renderTime = timer.elapsed();
    
    QVERIFY(result);
    QVERIFY(renderTime < 1000); // 10万点应在1秒内渲染完成
    
    qDebug() << "Large dataset rendering time:" << renderTime << "ms for" << largeDataset.size() << "points";
}

void TopDownViewTest::testInteractiveControls()
{
    // 测试交互控制器
    auto controller = m_renderer->getInteractionController();
    QVERIFY(controller != nullptr);
    
    // 测试平移
    QPointF initialCenter = controller->getViewCenter();
    controller->pan(QPointF(10, 10));
    QPointF newCenter = controller->getViewCenter();
    QVERIFY(newCenter != initialCenter);
    
    // 测试缩放
    float initialZoom = controller->getZoomLevel();
    controller->zoom(1.5f);
    float newZoom = controller->getZoomLevel();
    QVERIFY(qAbs(newZoom - initialZoom * 1.5f) < 0.1f);
}

void TopDownViewTest::testPanControl()
{
    auto controller = m_renderer->getInteractionController();
    
    QPointF startCenter = controller->getViewCenter();
    QPointF panDelta(20.0f, 15.0f);
    
    controller->startPan(QPointF(100, 100));
    controller->updatePan(QPointF(120, 115));
    controller->endPan();
    
    QPointF endCenter = controller->getViewCenter();
    QPointF actualDelta = endCenter - startCenter;
    
    QVERIFY(qAbs(actualDelta.x() - panDelta.x()) < 1.0f);
    QVERIFY(qAbs(actualDelta.y() - panDelta.y()) < 1.0f);
}

void TopDownViewTest::testZoomControl()
{
    auto controller = m_renderer->getInteractionController();
    
    float initialZoom = controller->getZoomLevel();
    
    // 测试滚轮缩放
    controller->wheelZoom(QPointF(400, 300), 120); // 正向滚轮
    float zoomedIn = controller->getZoomLevel();
    QVERIFY(zoomedIn > initialZoom);
    
    controller->wheelZoom(QPointF(400, 300), -120); // 反向滚轮
    float zoomedOut = controller->getZoomLevel();
    QVERIFY(qAbs(zoomedOut - initialZoom) < 0.1f);
}

void TopDownViewTest::testSelectionTool()
{
    auto testPoints = generatePointsWithAttributes();
    m_renderer->renderTopDownView(testPoints);
    
    auto controller = m_renderer->getInteractionController();
    
    // 测试矩形选择
    QRectF selectionRect(100, 100, 200, 150);
    auto selectedPoints = controller->selectPointsInRect(selectionRect);
    
    QVERIFY(!selectedPoints.empty());
    
    // 验证选中的点确实在选择区域内
    for (size_t pointIndex : selectedPoints) {
        QVERIFY(pointIndex < testPoints.size());
        // 这里需要验证投影后的点在选择矩形内
    }
}

void TopDownViewTest::testMeasurementTool()
{
    auto controller = m_renderer->getInteractionController();
    
    // 测试距离测量
    QPointF point1(100, 100);
    QPointF point2(200, 200);
    
    float screenDistance = controller->measureScreenDistance(point1, point2);
    QVERIFY(screenDistance > 0);
    
    float worldDistance = controller->measureWorldDistance(point1, point2);
    QVERIFY(worldDistance > 0);
    QVERIFY(worldDistance != screenDistance); // 应该不同，因为有缩放
}

// 辅助方法实现
std::vector<QVector3D> TopDownViewTest::generateBuildingPointCloud()
{
    std::vector<QVector3D> points;
    points.reserve(1000);
    
    // 生成一个简单的建筑物点云
    for (int x = 0; x < 50; ++x) {
        for (int y = 0; y < 20; ++y) {
            // 地面点
            points.emplace_back(x * 0.5f, y * 0.5f, 0.0f);
            
            // 墙面点
            if (x == 0 || x == 49 || y == 0 || y == 19) {
                for (int z = 1; z < 10; ++z) {
                    points.emplace_back(x * 0.5f, y * 0.5f, z * 0.5f);
                }
            }
        }
    }
    
    return points;
}

std::vector<WallExtraction::PointWithAttributes> TopDownViewTest::generatePointsWithAttributes()
{
    std::vector<WallExtraction::PointWithAttributes> points;
    points.reserve(500);
    
    for (int i = 0; i < 500; ++i) {
        WallExtraction::PointWithAttributes point;
        point.position = QVector3D(i % 25 * 2.0f, (i / 25) % 20 * 2.0f, (i % 20) * 1.0f);
        point.attributes["intensity"] = (i * 1000) % 65536;
        point.attributes["classification"] = i % 10;
        points.push_back(point);
    }
    
    return points;
}

QTEST_MAIN(TopDownViewTest)
#include "top_down_view_test.moc"
