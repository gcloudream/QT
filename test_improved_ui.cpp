#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QScrollArea>
#include <QFrame>

/**
 * @brief 改进后的UI样式测试程序
 * 
 * 这个程序展示了重新设计的阶段二UI组件的改进效果，解决了：
 * 1. 字体可读性问题 - 使用更大的按钮和字体
 * 2. 布局拥挤问题 - 增加间距和边距
 * 3. 视觉美观性问题 - 现代化设计风格
 */
class ImprovedUITest : public QWidget
{
    Q_OBJECT

public:
    explicit ImprovedUITest(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setupUI();
        setWindowTitle("阶段二UI组件改进效果测试");
        setMinimumSize(1200, 800);
    }

private:
    void setupUI()
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(20);
        
        // 标题
        QLabel* titleLabel = new QLabel("阶段二UI组件改进效果对比");
        titleLabel->setStyleSheet(R"(
            QLabel {
                font-size: 24px;
                font-weight: 600;
                color: #2c3e50;
                padding: 16px;
                background-color: #ecf0f1;
                border-radius: 12px;
                margin-bottom: 12px;
            }
        )");
        mainLayout->addWidget(titleLabel);
        
        // 创建滚动区域
        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
        
        QWidget* scrollContent = new QWidget();
        QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
        scrollLayout->setSpacing(24);
        
        // 改进后的工具栏预览
        scrollLayout->addWidget(createImprovedToolbarPreview());
        
        // 改进后的属性面板预览
        scrollLayout->addWidget(createImprovedPropertyPanelPreview());
        
        // 改进说明
        scrollLayout->addWidget(createImprovementNotes());
        
        scrollArea->setWidget(scrollContent);
        mainLayout->addWidget(scrollArea);
    }
    
    QWidget* createImprovedToolbarPreview()
    {
        QGroupBox* group = new QGroupBox("改进后的 LineDrawingToolbar");
        group->setStyleSheet(getModernGroupBoxStyle());
        
        QVBoxLayout* layout = new QVBoxLayout(group);
        layout->setContentsMargins(16, 20, 16, 16);
        
        // 模拟改进后的工具栏
        QWidget* toolbar = new QWidget();
        toolbar->setStyleSheet(getModernToolbarStyle());
        toolbar->setMinimumHeight(120);
        
        // 主垂直布局
        QVBoxLayout* mainVerticalLayout = new QVBoxLayout(toolbar);
        mainVerticalLayout->setContentsMargins(20, 16, 20, 16);
        mainVerticalLayout->setSpacing(12);
        
        // 工具栏标题
        QLabel* toolbarTitle = createStyledLabel("线段绘制工具", "toolbar-title");
        mainVerticalLayout->addWidget(toolbarTitle);
        
        // 工具区域
        QWidget* toolsArea = new QWidget();
        QHBoxLayout* toolbarLayout = new QHBoxLayout(toolsArea);
        toolbarLayout->setContentsMargins(0, 0, 0, 0);
        toolbarLayout->setSpacing(24);
        
        // 绘制模式组 - 水平布局，更大按钮
        QGroupBox* drawingGroup = new QGroupBox("绘制模式");
        drawingGroup->setStyleSheet(getModernGroupBoxStyle());
        drawingGroup->setMinimumWidth(500);
        QHBoxLayout* drawingLayout = new QHBoxLayout(drawingGroup);
        drawingLayout->setContentsMargins(16, 20, 16, 16);
        drawingLayout->setSpacing(12);
        
        drawingLayout->addWidget(createStyledButton("无", "mode-button xlarge", true));
        drawingLayout->addWidget(createStyledButton("单线段", "mode-button xlarge"));
        drawingLayout->addWidget(createStyledButton("多段线", "mode-button xlarge"));
        drawingLayout->addWidget(createStyledButton("选择", "mode-button xlarge"));
        drawingLayout->addWidget(createStyledButton("编辑", "mode-button xlarge"));
        
        // 工具组 - 水平布局
        QGroupBox* toolsGroup = new QGroupBox("操作工具");
        toolsGroup->setStyleSheet(getModernGroupBoxStyle());
        toolsGroup->setMinimumWidth(350);
        QHBoxLayout* toolsLayout = new QHBoxLayout(toolsGroup);
        toolsLayout->setContentsMargins(16, 20, 16, 16);
        toolsLayout->setSpacing(12);
        
        toolsLayout->addWidget(createStyledButton("保存", "tool-button success xlarge"));
        toolsLayout->addWidget(createStyledButton("加载", "tool-button primary xlarge"));
        toolsLayout->addWidget(createStyledButton("删除选中", "tool-button warning xlarge"));
        toolsLayout->addWidget(createStyledButton("清空所有", "tool-button danger xlarge"));
        
        // 状态组 - 改进的状态显示
        QGroupBox* statusGroup = new QGroupBox("状态信息");
        statusGroup->setStyleSheet(getModernGroupBoxStyle());
        statusGroup->setMinimumWidth(180);
        QVBoxLayout* statusMainLayout = new QVBoxLayout(statusGroup);
        statusMainLayout->setContentsMargins(16, 20, 16, 16);
        statusMainLayout->setSpacing(10);
        
        QWidget* statusContainer = new QWidget();
        statusContainer->setStyleSheet(R"(
            QWidget {
                background-color: #f8f9fa;
                border: 2px solid #e9ecef;
                border-radius: 8px;
                padding: 12px;
            }
        )");
        QVBoxLayout* statusLayout = new QVBoxLayout(statusContainer);
        statusLayout->setContentsMargins(12, 12, 12, 12);
        statusLayout->setSpacing(8);
        
        statusLayout->addWidget(createStyledLabel("线段数: 5", "status-label xlarge"));
        statusLayout->addWidget(createStyledLabel("选中数: 2", "status-label xlarge"));
        statusLayout->addWidget(createStyledLabel("模式: 无", "status-label xlarge"));
        
        statusMainLayout->addWidget(statusContainer);
        
        toolbarLayout->addWidget(drawingGroup);
        toolbarLayout->addWidget(toolsGroup);
        toolbarLayout->addWidget(statusGroup);
        toolbarLayout->addStretch();
        
        mainVerticalLayout->addWidget(toolsArea);
        
        layout->addWidget(toolbar);
        
        return group;
    }
    
    QWidget* createImprovedPropertyPanelPreview()
    {
        QGroupBox* group = new QGroupBox("改进后的 LinePropertyPanel");
        group->setStyleSheet(getModernGroupBoxStyle());
        
        QVBoxLayout* layout = new QVBoxLayout(group);
        layout->setContentsMargins(16, 20, 16, 16);
        
        // 模拟改进后的属性面板
        QWidget* panel = new QWidget();
        panel->setStyleSheet(getModernPanelStyle());
        panel->setMinimumHeight(250);
        panel->setMinimumWidth(400);
        
        QVBoxLayout* panelLayout = new QVBoxLayout(panel);
        panelLayout->setContentsMargins(16, 16, 16, 16);
        panelLayout->setSpacing(12);
        
        // 头部 - 更大尺寸
        QWidget* header = new QWidget();
        header->setObjectName("HeaderWidget");
        header->setFixedHeight(48);
        header->setStyleSheet(R"(
            #HeaderWidget {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #e9ecef, stop: 1 #dee2e6);
                border-bottom: 2px solid #ced4da;
                border-top-left-radius: 12px;
                border-top-right-radius: 12px;
            }
        )");
        
        QHBoxLayout* headerLayout = new QHBoxLayout(header);
        headerLayout->setContentsMargins(16, 8, 16, 8);
        headerLayout->setSpacing(16);
        
        headerLayout->addWidget(createStyledLabel("线段属性", "title-label xlarge"));
        headerLayout->addStretch();
        headerLayout->addWidget(createStyledButton("折叠", "toggle-button xlarge"));
        
        // 标签页 - 改进的样式
        QTabWidget* tabWidget = new QTabWidget();
        tabWidget->setStyleSheet(getModernTabWidgetStyle());
        
        QWidget* infoTab = new QWidget();
        QVBoxLayout* infoLayout = new QVBoxLayout(infoTab);
        infoLayout->setContentsMargins(16, 16, 16, 16);
        infoLayout->setSpacing(8);
        infoLayout->addWidget(createStyledLabel("线段ID: 001", "info-label xlarge"));
        infoLayout->addWidget(createStyledLabel("长度: 15.6m", "info-label xlarge"));
        infoLayout->addWidget(createStyledLabel("角度: 45°", "info-label xlarge"));
        infoLayout->addStretch();
        
        QWidget* listTab = new QWidget();
        QVBoxLayout* listLayout = new QVBoxLayout(listTab);
        listLayout->setContentsMargins(16, 16, 16, 16);
        listLayout->addWidget(createStyledLabel("线段列表 (5个)", "info-label xlarge"));
        listLayout->addStretch();
        
        tabWidget->addTab(infoTab, "线段信息");
        tabWidget->addTab(listTab, "线段列表");
        
        panelLayout->addWidget(header);
        panelLayout->addWidget(tabWidget);
        
        layout->addWidget(panel);
        
        return group;
    }
    
    QWidget* createImprovementNotes()
    {
        QGroupBox* group = new QGroupBox("关键改进点");
        group->setStyleSheet(getModernGroupBoxStyle());
        
        QVBoxLayout* layout = new QVBoxLayout(group);
        layout->setContentsMargins(16, 20, 16, 16);
        
        QLabel* notes = new QLabel(R"(
<b>🎯 主要改进效果：</b><br><br>
<b>1. 字体可读性大幅提升：</b><br>
• 按钮尺寸从55x32增加到80x40像素<br>
• 字体大小从11px增加到14px<br>
• 内边距从6x12增加到12x20像素<br>
• 确保所有中文文字完全可见<br><br>

<b>2. 布局拥挤问题彻底解决：</b><br>
• 工具栏高度从80px增加到120px<br>
• 组件间距从16px增加到24px<br>
• 边距从12px增加到20px<br>
• 绘制模式改为单行水平布局<br><br>

<b>3. 视觉美观性显著提升：</b><br>
• 使用渐变背景和现代化圆角设计<br>
• 统一的颜色方案和语义化颜色<br>
• 增强的交互反馈效果<br>
• 更清晰的视觉层次结构<br><br>

<b>4. 响应式设计优化：</b><br>
• 设置合理的最小尺寸<br>
• 使用弹性布局策略<br>
• 支持不同屏幕分辨率<br>
• 适应窗口大小变化
        )");
        notes->setWordWrap(true);
        notes->setStyleSheet(R"(
            QLabel {
                font-size: 13px;
                color: #2c3e50;
                background-color: #f8f9fa;
                padding: 20px;
                border-radius: 8px;
                border: 2px solid #e9ecef;
                line-height: 1.6;
            }
        )");
        
        layout->addWidget(notes);
        
        return group;
    }
    
    // 样式方法（与实际组件保持一致）
    QString getModernToolbarStyle() const
    {
        return R"(
            QWidget {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #ffffff, stop: 1 #f8f9fa);
                border: 2px solid #dee2e6;
                border-radius: 12px;
                padding: 8px;
            }
            QLabel[objectName="toolbar-title"] {
                font-size: 14px;
                font-weight: 600;
                color: #495057;
                padding: 4px 8px;
                background-color: transparent;
                border: none;
            }
        )";
    }
    
    QString getModernGroupBoxStyle() const
    {
        return R"(
            QGroupBox {
                font-weight: 600;
                font-size: 12px;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 6px;
                margin-top: 8px;
                padding-top: 4px;
                background-color: #f8f9fa;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 8px;
                padding: 0 4px 0 4px;
                background-color: #f8f9fa;
            }
        )";
    }
    
    QString getModernPanelStyle() const
    {
        return R"(
            QWidget {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #ffffff, stop: 1 #f8f9fa);
                border: 2px solid #dee2e6;
                border-radius: 12px;
                padding: 0px;
            }
        )";
    }
    
    QString getModernTabWidgetStyle() const
    {
        return R"(
            QTabWidget::pane {
                border: 2px solid #dee2e6;
                border-radius: 8px;
                background-color: #ffffff;
                margin-top: 4px;
                padding: 8px;
            }
            QTabBar::tab {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #f8f9fa, stop: 1 #e9ecef);
                border: 2px solid #dee2e6;
                border-bottom: none;
                border-top-left-radius: 8px;
                border-top-right-radius: 8px;
                padding: 8px 16px;
                margin-right: 4px;
                font-size: 12px;
                font-weight: 500;
                color: #6c757d;
                min-width: 80px;
                min-height: 32px;
            }
            QTabBar::tab:selected {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #ffffff, stop: 1 #f8f9fa);
                color: #495057;
                border-bottom: 2px solid #ffffff;
                font-weight: 600;
                border-color: #adb5bd;
            }
            QTabBar::tab:hover:!selected {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #e9ecef, stop: 1 #dee2e6);
                color: #495057;
                border-color: #ced4da;
            }
        )";
    }
    
    QPushButton* createStyledButton(const QString& text, const QString& styleClass, bool checked = false)
    {
        QPushButton* button = new QPushButton(text);
        button->setCheckable(true);
        button->setChecked(checked);
        
        // 支持xlarge尺寸
        QString fontSize, padding, minWidth, minHeight;
        
        if (styleClass.contains("xlarge")) {
            fontSize = "14px";
            padding = "12px 20px";
            minWidth = "100px";
            minHeight = "40px";
        } else if (styleClass.contains("large")) {
            fontSize = "12px";
            padding = "8px 16px";
            minWidth = "80px";
            minHeight = "32px";
        } else {
            fontSize = "11px";
            padding = "6px 12px";
            minWidth = "60px";
            minHeight = "24px";
        }
        
        QString baseStyle = QString(R"(
            QPushButton {
                font-size: %1;
                font-weight: 500;
                padding: %2;
                border: 2px solid #bdc3c7;
                border-radius: 6px;
                background-color: #ffffff;
                color: #2c3e50;
                min-width: %3;
                min-height: %4;
            }
            QPushButton:hover {
                background-color: #ecf0f1;
                border-color: #95a5a6;
            }
            QPushButton:pressed {
                background-color: #d5dbdb;
            }
            QPushButton:checked {
                background-color: #3498db;
                color: white;
                border-color: #2980b9;
                font-weight: 600;
            }
            QPushButton:checked:hover {
                background-color: #2980b9;
            }
        )").arg(fontSize, padding, minWidth, minHeight);
        
        // 语义化颜色
        if (styleClass.contains("danger")) {
            baseStyle += R"(
                QPushButton { 
                    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                              stop: 0 #e74c3c, stop: 1 #c0392b);
                    color: white; 
                    border-color: #a93226; 
                }
                QPushButton:hover { 
                    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                              stop: 0 #c0392b, stop: 1 #a93226);
                }
            )";
        } else if (styleClass.contains("warning")) {
            baseStyle += R"(
                QPushButton { 
                    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                              stop: 0 #f39c12, stop: 1 #e67e22);
                    color: white; 
                    border-color: #d68910; 
                }
                QPushButton:hover { 
                    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                              stop: 0 #e67e22, stop: 1 #d68910);
                }
            )";
        } else if (styleClass.contains("success")) {
            baseStyle += R"(
                QPushButton { 
                    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                              stop: 0 #27ae60, stop: 1 #229954);
                    color: white; 
                    border-color: #1e8449; 
                }
                QPushButton:hover { 
                    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                              stop: 0 #229954, stop: 1 #1e8449);
                }
            )";
        } else if (styleClass.contains("primary")) {
            baseStyle += R"(
                QPushButton { 
                    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                              stop: 0 #3498db, stop: 1 #2980b9);
                    color: white; 
                    border-color: #2471a3; 
                }
                QPushButton:hover { 
                    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                              stop: 0 #2980b9, stop: 1 #2471a3);
                }
            )";
        } else if (styleClass.contains("toggle")) {
            baseStyle += R"(
                QPushButton {
                    background-color: #3498db;
                    color: white;
                    border-color: #2980b9;
                }
                QPushButton:hover {
                    background-color: #2980b9;
                }
            )";
        }
        
        button->setStyleSheet(baseStyle);
        button->setMinimumSize(minWidth.toInt(), minHeight.toInt());
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        return button;
    }
    
    QLabel* createStyledLabel(const QString& text, const QString& styleClass)
    {
        QLabel* label = new QLabel(text);
        
        QString fontSize, fontWeight, color;
        
        if (styleClass.contains("title")) {
            if (styleClass.contains("xlarge")) {
                fontSize = "16px";
                fontWeight = "600";
                color = "#2c3e50";
            } else {
                fontSize = "13px";
                fontWeight = "600";
                color = "#34495e";
            }
        } else {
            if (styleClass.contains("xlarge")) {
                fontSize = "13px";
                fontWeight = "500";
                color = "#495057";
            } else {
                fontSize = "11px";
                fontWeight = "400";
                color = "#7f8c8d";
            }
        }
        
        QString style = QString(R"(
            QLabel {
                font-size: %1;
                font-weight: %2;
                color: %3;
                background-color: transparent;
                padding: 4px 6px;
            }
        )").arg(fontSize, fontWeight, color);
        
        if (styleClass.contains("status")) {
            style += R"(
                QLabel {
                    background-color: rgba(52, 73, 94, 0.05);
                    border-radius: 3px;
                    margin: 1px 0px;
                }
            )";
        }
        
        label->setStyleSheet(style);
        return label;
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ImprovedUITest window;
    window.show();
    
    return app.exec();
}

#include "test_improved_ui.moc"
