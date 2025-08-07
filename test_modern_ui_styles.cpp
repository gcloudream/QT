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
 * @brief 现代化UI样式测试程序
 * 
 * 这个程序展示了重新设计的阶段二UI组件的样式效果，包括：
 * 1. LineDrawingToolbar的现代化样式
 * 2. LinePropertyPanel的现代化样式
 * 3. 改进的按钮、标签和布局设计
 */
class ModernUIStyleTest : public QWidget
{
    Q_OBJECT

public:
    explicit ModernUIStyleTest(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setupUI();
        setWindowTitle("阶段二UI组件现代化样式测试");
        setMinimumSize(1000, 700);
    }

private:
    void setupUI()
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 16, 16, 16);
        mainLayout->setSpacing(16);
        
        // 标题
        QLabel* titleLabel = new QLabel("阶段二UI组件现代化样式预览");
        titleLabel->setStyleSheet(R"(
            QLabel {
                font-size: 20px;
                font-weight: 600;
                color: #2c3e50;
                padding: 12px;
                background-color: #ecf0f1;
                border-radius: 8px;
                margin-bottom: 8px;
            }
        )");
        mainLayout->addWidget(titleLabel);
        
        // 创建滚动区域
        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
        
        QWidget* scrollContent = new QWidget();
        QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
        scrollLayout->setSpacing(20);
        
        // 1. LineDrawingToolbar样式预览
        scrollLayout->addWidget(createToolbarPreview());
        
        // 2. LinePropertyPanel样式预览
        scrollLayout->addWidget(createPropertyPanelPreview());
        
        // 3. 设计说明
        scrollLayout->addWidget(createDesignNotes());
        
        scrollArea->setWidget(scrollContent);
        mainLayout->addWidget(scrollArea);
    }
    
    QWidget* createToolbarPreview()
    {
        QGroupBox* group = new QGroupBox("LineDrawingToolbar 改进后样式预览");
        group->setStyleSheet(getModernGroupBoxStyle());

        QVBoxLayout* layout = new QVBoxLayout(group);
        layout->setContentsMargins(12, 16, 12, 12);

        // 模拟改进后的工具栏
        QWidget* toolbar = new QWidget();
        toolbar->setStyleSheet(getModernToolbarStyle());
        toolbar->setMinimumHeight(120);

        // 主垂直布局
        QVBoxLayout* mainVerticalLayout = new QVBoxLayout(toolbar);
        mainVerticalLayout->setContentsMargins(16, 12, 16, 12);
        mainVerticalLayout->setSpacing(8);

        // 工具栏标题
        QLabel* toolbarTitle = createStyledLabel("线段绘制工具", "toolbar-title");
        mainVerticalLayout->addWidget(toolbarTitle);

        // 工具区域
        QWidget* toolsArea = new QWidget();
        QHBoxLayout* toolbarLayout = new QHBoxLayout(toolsArea);
        toolbarLayout->setContentsMargins(0, 0, 0, 0);
        toolbarLayout->setSpacing(20);

        // 绘制模式组 - 使用网格布局
        QGroupBox* drawingGroup = new QGroupBox("绘制模式");
        drawingGroup->setStyleSheet(getModernGroupBoxStyle());
        drawingGroup->setMinimumWidth(300);
        QGridLayout* drawingLayout = new QGridLayout(drawingGroup);
        drawingLayout->setContentsMargins(12, 16, 12, 12);
        drawingLayout->setSpacing(8);

        drawingLayout->addWidget(createStyledButton("无", "mode-button large", true), 0, 0);
        drawingLayout->addWidget(createStyledButton("单线段", "mode-button large"), 0, 1);
        drawingLayout->addWidget(createStyledButton("多段线", "mode-button large"), 0, 2);
        drawingLayout->addWidget(createStyledButton("选择", "mode-button large"), 1, 0);
        drawingLayout->addWidget(createStyledButton("编辑", "mode-button large"), 1, 1);

        // 工具组 - 使用垂直布局
        QGroupBox* toolsGroup = new QGroupBox("操作工具");
        toolsGroup->setStyleSheet(getModernGroupBoxStyle());
        toolsGroup->setMinimumWidth(200);
        QVBoxLayout* toolsMainLayout = new QVBoxLayout(toolsGroup);
        toolsMainLayout->setContentsMargins(12, 16, 12, 12);
        toolsMainLayout->setSpacing(6);

        QHBoxLayout* mainActionsLayout = new QHBoxLayout();
        mainActionsLayout->setSpacing(8);
        mainActionsLayout->addWidget(createStyledButton("保存", "tool-button success large"));
        mainActionsLayout->addWidget(createStyledButton("加载", "tool-button primary large"));

        QHBoxLayout* deleteActionsLayout = new QHBoxLayout();
        deleteActionsLayout->setSpacing(8);
        deleteActionsLayout->addWidget(createStyledButton("删除选中", "tool-button warning large"));
        deleteActionsLayout->addWidget(createStyledButton("清空所有", "tool-button danger large"));

        toolsMainLayout->addLayout(mainActionsLayout);
        toolsMainLayout->addLayout(deleteActionsLayout);

        // 状态组 - 改进的状态显示
        QGroupBox* statusGroup = new QGroupBox("状态信息");
        statusGroup->setStyleSheet(getModernGroupBoxStyle());
        statusGroup->setMinimumWidth(150);
        QVBoxLayout* statusMainLayout = new QVBoxLayout(statusGroup);
        statusMainLayout->setContentsMargins(12, 16, 12, 12);
        statusMainLayout->setSpacing(8);

        QWidget* statusContainer = new QWidget();
        statusContainer->setStyleSheet(R"(
            QWidget {
                background-color: #f8f9fa;
                border: 1px solid #e9ecef;
                border-radius: 4px;
                padding: 8px;
            }
        )");
        QVBoxLayout* statusLayout = new QVBoxLayout(statusContainer);
        statusLayout->setContentsMargins(8, 8, 8, 8);
        statusLayout->setSpacing(6);

        statusLayout->addWidget(createStyledLabel("线段数: 5", "status-label large"));
        statusLayout->addWidget(createStyledLabel("选中数: 2", "status-label large"));
        statusLayout->addWidget(createStyledLabel("模式: 无", "status-label large"));

        statusMainLayout->addWidget(statusContainer);

        toolbarLayout->addWidget(drawingGroup);
        toolbarLayout->addWidget(toolsGroup);
        toolbarLayout->addWidget(statusGroup);
        toolbarLayout->addStretch();

        mainVerticalLayout->addWidget(toolsArea);

        layout->addWidget(toolbar);

        return group;
    }
    
    QWidget* createPropertyPanelPreview()
    {
        QGroupBox* group = new QGroupBox("LinePropertyPanel 样式预览");
        group->setStyleSheet(getModernGroupBoxStyle());
        
        QVBoxLayout* layout = new QVBoxLayout(group);
        layout->setContentsMargins(12, 16, 12, 12);
        
        // 模拟属性面板
        QWidget* panel = new QWidget();
        panel->setStyleSheet(getModernPanelStyle());
        panel->setFixedHeight(300);
        
        QVBoxLayout* panelLayout = new QVBoxLayout(panel);
        panelLayout->setContentsMargins(8, 8, 8, 8);
        panelLayout->setSpacing(4);
        
        // 头部
        QWidget* header = new QWidget();
        header->setObjectName("HeaderWidget");
        header->setFixedHeight(36);
        header->setStyleSheet(R"(
            #HeaderWidget {
                background-color: #f8f9fa;
                border-bottom: 1px solid #e9ecef;
                border-top-left-radius: 8px;
                border-top-right-radius: 8px;
            }
        )");
        
        QHBoxLayout* headerLayout = new QHBoxLayout(header);
        headerLayout->setContentsMargins(12, 6, 12, 6);
        headerLayout->setSpacing(12);
        
        headerLayout->addWidget(createStyledLabel("线段属性", "title-label"));
        headerLayout->addStretch();
        headerLayout->addWidget(createStyledButton("折叠", "toggle-button"));
        
        // 标签页
        QTabWidget* tabWidget = new QTabWidget();
        tabWidget->setStyleSheet(getModernTabWidgetStyle());
        
        QWidget* infoTab = new QWidget();
        QVBoxLayout* infoLayout = new QVBoxLayout(infoTab);
        infoLayout->addWidget(createStyledLabel("线段ID: 001", "info-label"));
        infoLayout->addWidget(createStyledLabel("长度: 15.6m", "info-label"));
        infoLayout->addWidget(createStyledLabel("角度: 45°", "info-label"));
        infoLayout->addStretch();
        
        QWidget* listTab = new QWidget();
        QVBoxLayout* listLayout = new QVBoxLayout(listTab);
        listLayout->addWidget(createStyledLabel("线段列表 (5个)", "info-label"));
        listLayout->addStretch();
        
        tabWidget->addTab(infoTab, "线段信息");
        tabWidget->addTab(listTab, "线段列表");
        
        panelLayout->addWidget(header);
        panelLayout->addWidget(tabWidget);
        
        layout->addWidget(panel);
        
        return group;
    }
    
    QWidget* createDesignNotes()
    {
        QGroupBox* group = new QGroupBox("设计改进说明");
        group->setStyleSheet(getModernGroupBoxStyle());
        
        QVBoxLayout* layout = new QVBoxLayout(group);
        layout->setContentsMargins(12, 16, 12, 12);
        
        QLabel* notes = new QLabel(R"(
<b>主要改进点：</b><br>
• <b>字体可读性</b>：增加了按钮尺寸，调整字体大小为11px，确保文字完全可见<br>
• <b>布局优化</b>：增加了组件间距(16px)和内边距(8-12px)，避免拥挤感<br>
• <b>现代化设计</b>：采用扁平化设计，圆角边框(4-8px)，柔和的颜色搭配<br>
• <b>视觉层次</b>：使用不同的背景色和边框来区分功能区域<br>
• <b>交互反馈</b>：添加hover和pressed状态，提升用户体验<br>
• <b>颜色系统</b>：统一的颜色方案，主色调为蓝色系，辅以语义化颜色<br>
• <b>响应式设计</b>：最小尺寸设置，确保在不同分辨率下的显示效果
        )");
        notes->setWordWrap(true);
        notes->setStyleSheet(R"(
            QLabel {
                font-size: 12px;
                color: #34495e;
                background-color: #f8f9fa;
                padding: 12px;
                border-radius: 6px;
                border: 1px solid #e9ecef;
            }
        )");
        
        layout->addWidget(notes);
        
        return group;
    }
    
    // 样式方法（与实际组件中的样式保持一致）
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
                background-color: #ffffff;
                border: 1px solid #e0e0e0;
                border-radius: 8px;
                padding: 0px;
            }
        )";
    }
    
    QString getModernTabWidgetStyle() const
    {
        return R"(
            QTabWidget::pane {
                border: 1px solid #e0e0e0;
                border-radius: 4px;
                background-color: #ffffff;
                margin-top: 2px;
            }
            QTabBar::tab {
                background-color: #f8f9fa;
                border: 1px solid #e0e0e0;
                border-bottom: none;
                border-top-left-radius: 4px;
                border-top-right-radius: 4px;
                padding: 6px 12px;
                margin-right: 2px;
                font-size: 11px;
                color: #6c757d;
                min-width: 60px;
            }
            QTabBar::tab:selected {
                background-color: #ffffff;
                color: #495057;
                border-bottom: 1px solid #ffffff;
                font-weight: 500;
            }
            QTabBar::tab:hover:!selected {
                background-color: #e9ecef;
                color: #495057;
            }
        )";
    }
    
    QPushButton* createStyledButton(const QString& text, const QString& styleClass, bool checked = false)
    {
        QPushButton* button = new QPushButton(text);
        button->setCheckable(true);
        button->setChecked(checked);
        
        QString baseStyle = R"(
            QPushButton {
                font-size: 11px;
                font-weight: 500;
                padding: 6px 12px;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                background-color: #ffffff;
                color: #2c3e50;
                min-width: 60px;
                min-height: 24px;
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
            }
            QPushButton:checked:hover {
                background-color: #2980b9;
            }
        )";
        
        // 根据样式类添加特定颜色
        if (styleClass.contains("danger")) {
            baseStyle += R"(
                QPushButton { background-color: #e74c3c; color: white; border-color: #c0392b; }
                QPushButton:hover { background-color: #c0392b; }
            )";
        } else if (styleClass.contains("warning")) {
            baseStyle += R"(
                QPushButton { background-color: #f39c12; color: white; border-color: #e67e22; }
                QPushButton:hover { background-color: #e67e22; }
            )";
        } else if (styleClass.contains("success")) {
            baseStyle += R"(
                QPushButton { background-color: #27ae60; color: white; border-color: #229954; }
                QPushButton:hover { background-color: #229954; }
            )";
        } else if (styleClass.contains("primary")) {
            baseStyle += R"(
                QPushButton { background-color: #3498db; color: white; border-color: #2980b9; }
                QPushButton:hover { background-color: #2980b9; }
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
        return button;
    }
    
    QLabel* createStyledLabel(const QString& text, const QString& styleClass)
    {
        QLabel* label = new QLabel(text);
        
        QString style = R"(
            QLabel {
                color: #2c3e50;
                background-color: transparent;
            }
        )";
        
        if (styleClass.contains("title")) {
            style += R"(
                QLabel {
                    font-size: 13px;
                    font-weight: 600;
                    color: #34495e;
                }
            )";
        } else if (styleClass.contains("status")) {
            style += R"(
                QLabel {
                    font-size: 11px;
                    color: #34495e;
                    padding: 2px 4px;
                }
            )";
        } else {
            style += R"(
                QLabel {
                    font-size: 11px;
                    color: #7f8c8d;
                    padding: 2px 4px;
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
    
    ModernUIStyleTest window;
    window.show();
    
    return app.exec();
}

#include "test_modern_ui_styles.moc"
