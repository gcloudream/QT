#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QDebug>
#include <QTimer>
#include <memory>

// 包含必要的头文件
#include "src/wall_extraction/stage1_demo_widget.h"
#include "src/wall_extraction/wall_extraction_manager.h"
#include "src/wall_extraction/line_drawing_toolbar.h"
#include "src/wall_extraction/line_property_panel.h"

/**
 * @brief 测试阶段二UI组件可见性的简单应用程序
 * 
 * 这个测试程序专门用于验证阶段二的UI组件是否正确显示：
 * 1. LineDrawingToolbar (线段绘制工具栏)
 * 2. LinePropertyPanel (线段属性面板)
 * 3. 相关的控制按钮和功能
 */
class Stage2UITestWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Stage2UITestWidget(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_stage1DemoWidget(nullptr)
        , m_wallManager(nullptr)
    {
        setupUI();
        testStage2Components();
    }

private slots:
    void checkUIComponents()
    {
        qDebug() << "=== 阶段二UI组件检查 ===";
        
        if (!m_stage1DemoWidget) {
            qCritical() << "Stage1DemoWidget 未创建";
            return;
        }
        
        // 检查LineDrawingToolbar
        auto* toolbar = m_stage1DemoWidget->findChild<WallExtraction::LineDrawingToolbar*>();
        if (toolbar) {
            qDebug() << "✓ LineDrawingToolbar 已找到";
            qDebug() << "  - 可见性:" << toolbar->isVisible();
            qDebug() << "  - 启用状态:" << toolbar->isEnabled();
            qDebug() << "  - 大小:" << toolbar->size();
        } else {
            qWarning() << "✗ LineDrawingToolbar 未找到";
        }
        
        // 检查LinePropertyPanel
        auto* panel = m_stage1DemoWidget->findChild<WallExtraction::LinePropertyPanel*>();
        if (panel) {
            qDebug() << "✓ LinePropertyPanel 已找到";
            qDebug() << "  - 可见性:" << panel->isVisible();
            qDebug() << "  - 启用状态:" << panel->isEnabled();
            qDebug() << "  - 大小:" << panel->size();
        } else {
            qWarning() << "✗ LinePropertyPanel 未找到";
        }
        
        // 检查WallExtractionManager
        if (m_wallManager) {
            qDebug() << "✓ WallExtractionManager 已创建";
            auto* lineDrawingTool = m_wallManager->getLineDrawingTool();
            if (lineDrawingTool) {
                qDebug() << "✓ LineDrawingTool 已创建";
            } else {
                qWarning() << "✗ LineDrawingTool 未创建";
            }
        } else {
            qWarning() << "✗ WallExtractionManager 未创建";
        }
        
        qDebug() << "=== 检查完成 ===";
    }

private:
    void setupUI()
    {
        setWindowTitle("阶段二UI组件可见性测试");
        setMinimumSize(1200, 800);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // 创建标题
        QLabel* titleLabel = new QLabel("阶段二UI组件测试");
        titleLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 18px;"
            "   font-weight: bold;"
            "   color: #2c3e50;"
            "   padding: 10px;"
            "   background-color: #ecf0f1;"
            "   border-radius: 5px;"
            "   margin-bottom: 10px;"
            "}"
        );
        mainLayout->addWidget(titleLabel);
        
        // 创建TabWidget来模拟主界面
        QTabWidget* tabWidget = new QTabWidget(this);
        
        // 创建Stage1DemoWidget
        try {
            m_stage1DemoWidget = new Stage1DemoWidget(this);
            tabWidget->addTab(m_stage1DemoWidget, "阶段一演示");
            qDebug() << "Stage1DemoWidget 创建成功";
        } catch (const std::exception& e) {
            qCritical() << "创建Stage1DemoWidget失败:" << e.what();
            
            // 创建错误显示标签
            QLabel* errorLabel = new QLabel(QString("创建Stage1DemoWidget失败: %1").arg(e.what()));
            errorLabel->setStyleSheet("QLabel { color: red; font-size: 14px; padding: 20px; }");
            tabWidget->addTab(errorLabel, "错误");
        }
        
        mainLayout->addWidget(tabWidget);
        
        // 创建状态信息标签
        QLabel* statusLabel = new QLabel("检查控制台输出以查看详细的组件状态信息");
        statusLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 12px;"
            "   color: #7f8c8d;"
            "   padding: 5px;"
            "   background-color: #f8f9fa;"
            "   border-radius: 3px;"
            "}"
        );
        mainLayout->addWidget(statusLabel);
    }
    
    void testStage2Components()
    {
        // 延迟检查，确保所有组件都已初始化
        QTimer::singleShot(1000, this, &Stage2UITestWidget::checkUIComponents);
    }

private:
    Stage1DemoWidget* m_stage1DemoWidget;
    std::unique_ptr<WallExtraction::WallExtractionManager> m_wallManager;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "启动阶段二UI组件可见性测试...";
    
    Stage2UITestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "test_stage2_ui_visibility.moc"
