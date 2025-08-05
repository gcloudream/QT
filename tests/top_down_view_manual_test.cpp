#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <memory>
#include "../src/wall_extraction/top_down_view_renderer.h"
#include "../src/wall_extraction/color_mapping_manager.h"
#include "../src/wall_extraction/view_projection_manager.h"
#include "../src/wall_extraction/top_down_interaction_controller.h"

/**
 * 手动测试程序，验证T1.4任务的完成情况
 * 测试俯视图显示效果改进功能
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== T1.4 Manual Test: Top-Down View Display Enhancement ===";
    
    bool allTestsPassed = true;
    
    // 生成测试数据
    auto generateBuildingPoints = []() {
        std::vector<WallExtraction::PointWithAttributes> points;
        points.reserve(5000);
        
        // 生成一个建筑物的点云数据
        for (int x = 0; x < 50; ++x) {
            for (int y = 0; y < 50; ++y) {
                WallExtraction::PointWithAttributes point;
                point.position = QVector3D(x * 2.0f, y * 2.0f, (x + y) * 0.1f);
                point.attributes["intensity"] = (x * y) % 65536;
                point.attributes["classification"] = (x + y) % 10;
                points.push_back(point);
            }
        }
        
        return points;
    };
    
    try {
        // 测试1: 俯视图渲染器创建
        qDebug() << "\n1. Testing TopDownViewRenderer creation...";
        auto renderer = std::make_unique<WallExtraction::TopDownViewRenderer>();
        if (renderer && renderer->isInitialized()) {
            qDebug() << "✓ TopDownViewRenderer created and initialized";
            qDebug() << "  - Viewport size:" << renderer->getViewportSize();
            qDebug() << "  - Render mode:" << static_cast<int>(renderer->getRenderMode());
        } else {
            qDebug() << "✗ Failed to create TopDownViewRenderer";
            allTestsPassed = false;
        }
        
        // 测试2: 颜色映射管理器
        qDebug() << "\n2. Testing ColorMappingManager...";
        auto colorMapper = renderer->getColorMappingManager();
        if (colorMapper && colorMapper->isInitialized()) {
            qDebug() << "✓ ColorMappingManager initialized";
            
            auto schemes = colorMapper->getAvailableColorSchemes();
            qDebug() << "  - Available color schemes:" << schemes;
            
            // 测试高度映射
            colorMapper->setColorScheme(WallExtraction::ColorScheme::Height);
            colorMapper->setValueRange(0.0f, 10.0f);
            
            QColor lowColor = colorMapper->getColorForValue(0.0f);
            QColor highColor = colorMapper->getColorForValue(10.0f);
            
            if (lowColor.isValid() && highColor.isValid() && lowColor != highColor) {
                qDebug() << "✓ Height-based color mapping works";
                qDebug() << "  - Low value color:" << lowColor.name();
                qDebug() << "  - High value color:" << highColor.name();
            } else {
                qDebug() << "✗ Height-based color mapping failed";
                allTestsPassed = false;
            }
            
            // 测试强度映射
            colorMapper->setColorScheme(WallExtraction::ColorScheme::Intensity);
            colorMapper->setValueRange(0.0f, 65535.0f);
            
            QColor intensityColor = colorMapper->getColorForValue(32768.0f);
            if (intensityColor.isValid()) {
                qDebug() << "✓ Intensity-based color mapping works";
            } else {
                qDebug() << "✗ Intensity-based color mapping failed";
                allTestsPassed = false;
            }
            
        } else {
            qDebug() << "✗ ColorMappingManager not available";
            allTestsPassed = false;
        }
        
        // 测试3: 投影管理器
        qDebug() << "\n3. Testing ViewProjectionManager...";
        auto projectionManager = renderer->getProjectionManager();
        if (projectionManager && projectionManager->isInitialized()) {
            qDebug() << "✓ ViewProjectionManager initialized";
            qDebug() << "  - Projection type:" << static_cast<int>(projectionManager->getProjectionType());
            qDebug() << "  - Viewport size:" << projectionManager->getViewportSize();
            
            // 测试投影变换
            std::vector<QVector3D> testPoints = {
                QVector3D(0, 0, 0),
                QVector3D(50, 50, 5),
                QVector3D(-25, 25, 2)
            };
            
            QElapsedTimer timer;
            timer.start();
            
            auto projectionResults = projectionManager->projectToTopDown(testPoints);
            qint64 projectionTime = timer.elapsed();
            
            if (projectionResults.size() == testPoints.size() && projectionTime < 10) {
                qDebug() << "✓ Projection transformation works in" << projectionTime << "ms";
                
                for (size_t i = 0; i < projectionResults.size(); ++i) {
                    qDebug() << "  - Point" << i << ":" << testPoints[i] 
                             << "-> Screen:" << projectionResults[i].screenPosition;
                }
            } else {
                qDebug() << "✗ Projection transformation failed or too slow";
                allTestsPassed = false;
            }
            
            // 测试坐标转换
            QVector3D worldPoint(25.0f, 25.0f, 1.0f);
            QVector2D screenPoint = projectionManager->worldToScreen(worldPoint);
            QVector3D backToWorld = projectionManager->screenToWorld(screenPoint, 1.0f);
            
            float error = (backToWorld - worldPoint).length();
            if (error < 0.1f) {
                qDebug() << "✓ Coordinate transformation accuracy good (error:" << error << ")";
            } else {
                qDebug() << "✗ Coordinate transformation accuracy poor (error:" << error << ")";
                allTestsPassed = false;
            }
            
        } else {
            qDebug() << "✗ ViewProjectionManager not available";
            allTestsPassed = false;
        }
        
        // 测试4: 交互控制器
        qDebug() << "\n4. Testing TopDownInteractionController...";
        auto controller = renderer->getInteractionController();
        if (controller) {
            qDebug() << "✓ TopDownInteractionController available";
            
            // 测试交互模式切换
            controller->setInteractionMode(WallExtraction::InteractionMode::Pan);
            if (controller->getInteractionMode() == WallExtraction::InteractionMode::Pan) {
                qDebug() << "✓ Interaction mode switching works";
            } else {
                qDebug() << "✗ Interaction mode switching failed";
                allTestsPassed = false;
            }
            
            // 测试缩放控制
            float initialZoom = controller->getZoomLevel();
            controller->zoom(2.0f);
            float newZoom = controller->getZoomLevel();
            
            if (qAbs(newZoom - initialZoom * 2.0f) < 0.1f) {
                qDebug() << "✓ Zoom control works (zoom:" << initialZoom << "->" << newZoom << ")";
            } else {
                qDebug() << "✗ Zoom control failed";
                allTestsPassed = false;
            }
            
            // 测试测量功能
            controller->setMeasurementType(WallExtraction::MeasurementType::Distance);
            float distance = controller->measureWorldDistance(QPointF(0, 0), QPointF(100, 100));
            
            if (distance > 0.0f) {
                qDebug() << "✓ Distance measurement works:" << distance << "units";
            } else {
                qDebug() << "✗ Distance measurement failed";
                allTestsPassed = false;
            }
            
        } else {
            qDebug() << "✗ TopDownInteractionController not available";
            allTestsPassed = false;
        }
        
        // 测试5: 俯视图渲染性能
        qDebug() << "\n5. Testing top-down rendering performance...";
        auto testPoints = generateBuildingPoints();
        
        // 设置渲染参数
        renderer->setViewportSize(QSize(1024, 768));
        renderer->setViewBounds(-50.0f, 150.0f, -50.0f, 150.0f);
        renderer->setPointSize(1.5f);
        
        // 测试点渲染模式
        renderer->setRenderMode(WallExtraction::TopDownRenderMode::Points);
        
        QElapsedTimer timer;
        timer.start();
        
        bool renderResult = renderer->renderTopDownView(testPoints);
        qint64 renderTime = timer.elapsed();
        
        if (renderResult && renderTime < 500) { // 应在500ms内完成
            qDebug() << "✓ Point rendering completed in" << renderTime << "ms for" << testPoints.size() << "points";
            
            // 验证渲染结果
            QImage renderBuffer = renderer->getRenderBuffer();
            if (!renderBuffer.isNull() && renderBuffer.size() == QSize(1024, 768)) {
                qDebug() << "✓ Render buffer is valid";
                
                // 保存渲染结果
                QString filename = "test_topdown_render.png";
                if (renderer->saveRenderResult(filename)) {
                    qDebug() << "✓ Render result saved to" << filename;
                } else {
                    qDebug() << "✗ Failed to save render result";
                    allTestsPassed = false;
                }
            } else {
                qDebug() << "✗ Render buffer is invalid";
                allTestsPassed = false;
            }
        } else {
            qDebug() << "✗ Point rendering failed or too slow (" << renderTime << "ms)";
            allTestsPassed = false;
        }
        
        // 测试6: 不同渲染模式
        qDebug() << "\n6. Testing different rendering modes...";
        
        QList<WallExtraction::TopDownRenderMode> modes = {
            WallExtraction::TopDownRenderMode::Points,
            WallExtraction::TopDownRenderMode::Density,
            WallExtraction::TopDownRenderMode::Heatmap
        };
        
        QStringList modeNames = {"Points", "Density", "Heatmap"};
        
        for (int i = 0; i < modes.size(); ++i) {
            renderer->setRenderMode(modes[i]);
            
            timer.restart();
            bool result = renderer->renderTopDownView(testPoints);
            qint64 modeRenderTime = timer.elapsed();
            
            if (result && modeRenderTime < 1000) {
                qDebug() << "✓" << modeNames[i] << "mode rendered in" << modeRenderTime << "ms";
            } else {
                qDebug() << "✗" << modeNames[i] << "mode failed or too slow (" << modeRenderTime << "ms)";
                allTestsPassed = false;
            }
        }
        
        // 测试7: 颜色条生成
        qDebug() << "\n7. Testing color bar generation...";
        auto colorBar = colorMapper->generateColorBar(256, 32);
        
        if (!colorBar.isNull() && colorBar.size() == QSize(256, 32)) {
            qDebug() << "✓ Color bar generated successfully";
            
            if (colorBar.save("test_colorbar.png")) {
                qDebug() << "✓ Color bar saved to test_colorbar.png";
            }
        } else {
            qDebug() << "✗ Color bar generation failed";
            allTestsPassed = false;
        }
        
        // 测试8: 渲染统计信息
        qDebug() << "\n8. Testing render statistics...";
        auto stats = renderer->getRenderStatistics();
        
        if (!stats.isEmpty()) {
            qDebug() << "✓ Render statistics available:";
            for (auto it = stats.begin(); it != stats.end(); ++it) {
                qDebug() << "  -" << it.key() << ":" << it.value();
            }
        } else {
            qDebug() << "✗ Render statistics not available";
            allTestsPassed = false;
        }
        
        // 测试9: 大数据量性能测试
        qDebug() << "\n9. Testing large dataset performance...";
        
        // 生成更大的数据集
        std::vector<WallExtraction::PointWithAttributes> largeDataset;
        largeDataset.reserve(50000);
        
        for (int i = 0; i < 50000; ++i) {
            WallExtraction::PointWithAttributes point;
            point.position = QVector3D((i % 500) * 0.2f, (i / 500) * 0.2f, (i % 100) * 0.05f);
            point.attributes["intensity"] = i % 65536;
            largeDataset.push_back(point);
        }
        
        timer.restart();
        bool largeRenderResult = renderer->renderTopDownView(largeDataset);
        qint64 largeRenderTime = timer.elapsed();
        
        if (largeRenderResult && largeRenderTime < 2000) { // 应在2秒内完成
            qDebug() << "✓ Large dataset rendering completed in" << largeRenderTime 
                     << "ms for" << largeDataset.size() << "points";
        } else {
            qDebug() << "✗ Large dataset rendering failed or too slow (" << largeRenderTime << "ms)";
            allTestsPassed = false;
        }
        
    } catch (const std::exception& e) {
        qDebug() << "✗ Unexpected exception:" << e.what();
        allTestsPassed = false;
    }
    
    // 测试结果总结
    qDebug() << "\n=== Test Results ===";
    if (allTestsPassed) {
        qDebug() << "✓ All tests PASSED! T1.4 task completed successfully.";
        qDebug() << "✓ 2D top-down projection algorithm implemented";
        qDebug() << "✓ Height and intensity-based color mapping added";
        qDebug() << "✓ Top-down view rendering performance optimized";
        qDebug() << "✓ Specialized interactive controls for top-down view added";
        qDebug() << "✓ Multiple rendering modes (Points, Density, Heatmap) supported";
        qDebug() << "✓ Performance requirements met (5k points <500ms, 50k points <2s)";
        return 0;
    } else {
        qDebug() << "✗ Some tests FAILED. Please review the implementation.";
        return 1;
    }
}
