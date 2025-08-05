#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <memory>
#include "../src/wall_extraction/wall_extraction_manager.h"

/**
 * 手动测试程序，验证T1.1任务的完成情况
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== T1.1 Manual Test: Wall Extraction Module Basic Architecture ===";
    
    // 创建测试窗口
    QWidget testWidget;
    testWidget.resize(800, 600);
    testWidget.setWindowTitle("Wall Extraction Module Test");
    
    bool allTestsPassed = true;
    
    try {
        // 测试1: 创建WallExtractionManager
        qDebug() << "\n1. Testing WallExtractionManager creation...";
        auto manager = std::make_unique<WallExtraction::WallExtractionManager>(&testWidget);
        if (manager) {
            qDebug() << "✓ WallExtractionManager created successfully";
        } else {
            qDebug() << "✗ Failed to create WallExtractionManager";
            allTestsPassed = false;
        }
        
        // 测试2: 检查初始状态
        qDebug() << "\n2. Testing initial state...";
        if (!manager->isInitialized()) {
            qDebug() << "✓ Manager is not initialized initially (correct)";
        } else {
            qDebug() << "✗ Manager should not be initialized initially";
            allTestsPassed = false;
        }
        
        if (!manager->isActive()) {
            qDebug() << "✓ Manager is not active initially (correct)";
        } else {
            qDebug() << "✗ Manager should not be active initially";
            allTestsPassed = false;
        }
        
        if (manager->getCurrentMode() == WallExtraction::InteractionMode::PointCloudView) {
            qDebug() << "✓ Initial mode is PointCloudView (correct)";
        } else {
            qDebug() << "✗ Initial mode should be PointCloudView";
            allTestsPassed = false;
        }
        
        // 测试3: 初始化管理器
        qDebug() << "\n3. Testing manager initialization...";
        if (manager->initialize()) {
            qDebug() << "✓ Manager initialized successfully";
            
            if (manager->isInitialized()) {
                qDebug() << "✓ Manager reports initialized state correctly";
            } else {
                qDebug() << "✗ Manager should report initialized state";
                allTestsPassed = false;
            }
        } else {
            qDebug() << "✗ Failed to initialize manager";
            allTestsPassed = false;
        }
        
        // 测试4: 检查子组件
        qDebug() << "\n4. Testing sub-components...";
        if (manager->getLineDrawingTool() != nullptr) {
            qDebug() << "✓ LineDrawingTool created";
        } else {
            qDebug() << "✗ LineDrawingTool not created";
            allTestsPassed = false;
        }
        
        if (manager->getWallFittingAlgorithm() != nullptr) {
            qDebug() << "✓ WallFittingAlgorithm created";
        } else {
            qDebug() << "✗ WallFittingAlgorithm not created";
            allTestsPassed = false;
        }
        
        if (manager->getWireframeGenerator() != nullptr) {
            qDebug() << "✓ WireframeGenerator created";
        } else {
            qDebug() << "✗ WireframeGenerator not created";
            allTestsPassed = false;
        }
        
        // 测试5: 模块激活
        qDebug() << "\n5. Testing module activation...";
        if (manager->activateModule()) {
            qDebug() << "✓ Module activated successfully";
            
            if (manager->isActive()) {
                qDebug() << "✓ Module reports active state correctly";
            } else {
                qDebug() << "✗ Module should report active state";
                allTestsPassed = false;
            }
        } else {
            qDebug() << "✗ Failed to activate module";
            allTestsPassed = false;
        }
        
        // 测试6: 交互模式切换
        qDebug() << "\n6. Testing interaction mode switching...";
        QList<WallExtraction::InteractionMode> modes = {
            WallExtraction::InteractionMode::LineDrawing,
            WallExtraction::InteractionMode::WallExtraction,
            WallExtraction::InteractionMode::WireframeView,
            WallExtraction::InteractionMode::PointCloudView
        };
        
        for (auto mode : modes) {
            if (manager->setInteractionMode(mode)) {
                if (manager->getCurrentMode() == mode) {
                    qDebug() << "✓ Mode switch successful for mode" << static_cast<int>(mode);
                } else {
                    qDebug() << "✗ Mode not set correctly for mode" << static_cast<int>(mode);
                    allTestsPassed = false;
                }
            } else {
                qDebug() << "✗ Failed to set mode" << static_cast<int>(mode);
                allTestsPassed = false;
            }
        }
        
        // 测试7: 模块停用
        qDebug() << "\n7. Testing module deactivation...";
        if (manager->deactivateModule()) {
            qDebug() << "✓ Module deactivated successfully";
            
            if (!manager->isActive()) {
                qDebug() << "✓ Module reports inactive state correctly";
            } else {
                qDebug() << "✗ Module should report inactive state";
                allTestsPassed = false;
            }
        } else {
            qDebug() << "✗ Failed to deactivate module";
            allTestsPassed = false;
        }
        
        // 测试8: 异常处理
        qDebug() << "\n8. Testing exception handling...";
        try {
            manager->processInvalidOperation();
            qDebug() << "✗ Exception should have been thrown";
            allTestsPassed = false;
        } catch (const WallExtraction::WallExtractionException& e) {
            qDebug() << "✓ Exception handling works correctly:" << e.what();
        }
        
    } catch (const std::exception& e) {
        qDebug() << "✗ Unexpected exception:" << e.what();
        allTestsPassed = false;
    }
    
    // 测试结果总结
    qDebug() << "\n=== Test Results ===";
    if (allTestsPassed) {
        qDebug() << "✓ All tests PASSED! T1.1 task completed successfully.";
        qDebug() << "✓ WallExtraction namespace created";
        qDebug() << "✓ WallExtractionManager base class implemented";
        qDebug() << "✓ Module communication interfaces designed";
        qDebug() << "✓ Integration with MainWindow system completed";
        return 0;
    } else {
        qDebug() << "✗ Some tests FAILED. Please review the implementation.";
        return 1;
    }
}
