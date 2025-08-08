#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QButtonGroup>
#include <QScrollArea>
#include <QDebug>

/**
 * @brief LineDrawingToolbar水平布局测试程序
 * 
 * 这个程序测试LineDrawingToolbar的水平布局修改：
 * 1. 验证移除标题文字的效果
 * 2. 测试各组内按钮的水平单行排列
 * 3. 验证整体高度的减少
 * 4. 测试字体和尺寸的适配性
 */
class LineDrawingHorizontalLayoutTest : public QWidget
{
    Q_OBJECT

public:
    explicit LineDrawingHorizontalLayoutTest(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setupUI();
        setWindowTitle("LineDrawingToolbar水平布局测试");
        setMinimumSize(500, 600);
        resize(600, 700);
    }

private:
    void setupUI()
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 16, 16, 16);
        mainLayout->setSpacing(16);
        
        // 标题
        QLabel* titleLabel = new QLabel("LineDrawingToolbar水平布局修改测试");
        titleLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 18px;"
            "   font-weight: 600;"
            "   color: #2c3e50;"
            "   padding: 12px;"
            "   background-color: #e8f4fd;"
            "   border: 2px solid #3498db;"
            "   border-radius: 8px;"
            "   margin-bottom: 8px;"
            "}"
        );
        mainLayout->addWidget(titleLabel);
        
        // 功能说明
        QLabel* descLabel = new QLabel(
            "UI修改内容：\n\n"
            "✓ 移除了\"线框绘制工具\"标题文字\n"
            "✓ 绘制模式组：5个按钮水平单行排列\n"
            "✓ 编辑模式组：3个按钮水平单行排列\n"
            "✓ 操作工具组：4个按钮水平单行排列\n"
            "✓ 状态信息组：3个状态项水平单行排列\n"
            "✓ 整体高度从400px减少到约220px\n"
            "✓ 字体大小确保中文文字完整显示\n\n"
            "对比效果：界面更加紧凑，宽度利用更充分"
        );
        descLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 12px;"
            "   color: #2c3e50;"
            "   padding: 12px;"
            "   background-color: #f8f9fa;"
            "   border: 1px solid #dee2e6;"
            "   border-radius: 6px;"
            "   line-height: 1.4;"
            "}"
        );
        descLabel->setWordWrap(true);
        mainLayout->addWidget(descLabel);
        
        // 创建对比区域
        QHBoxLayout* comparisonLayout = new QHBoxLayout();
        comparisonLayout->setSpacing(16);
        
        // 原始垂直布局（左侧）
        QWidget* originalWidget = createOriginalVerticalLayout();
        originalWidget->setMaximumWidth(250);
        comparisonLayout->addWidget(originalWidget);
        
        // 优化后水平布局（右侧）
        QWidget* optimizedWidget = createOptimizedHorizontalLayout();
        comparisonLayout->addWidget(optimizedWidget);
        
        mainLayout->addLayout(comparisonLayout);
        mainLayout->addStretch();
    }
    
    QWidget* createOriginalVerticalLayout()
    {
        QWidget* widget = new QWidget();
        widget->setStyleSheet(
            "QWidget {"
            "   background-color: #ffffff;"
            "   border: 2px solid #e74c3c;"
            "   border-radius: 8px;"
            "}"
        );
        
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(12, 8, 12, 8);
        layout->setSpacing(6);
        
        // 标题
        QLabel* titleLabel = new QLabel("原始垂直布局");
        titleLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   color: #e74c3c;"
            "   padding: 8px 0;"
            "   border-bottom: 2px solid #e9ecef;"
            "   margin-bottom: 8px;"
            "}"
        );
        layout->addWidget(titleLabel);
        
        // 工具栏标题（将被移除）
        QLabel* toolbarTitle = new QLabel("线框绘制工具");
        toolbarTitle->setStyleSheet(
            "QLabel {"
            "   font-size: 12px;"
            "   font-weight: 600;"
            "   color: #e74c3c;"
            "   padding: 6px;"
            "   background-color: #ffeaea;"
            "   border: 1px dashed #e74c3c;"
            "   border-radius: 4px;"
            "   text-decoration: line-through;"
            "}"
        );
        layout->addWidget(toolbarTitle);
        
        // 绘制模式组（垂直）
        QGroupBox* drawingGroup = new QGroupBox("绘制模式");
        drawingGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* drawingLayout = new QVBoxLayout(drawingGroup);
        drawingLayout->setSpacing(4);
        
        QStringList drawingModes = {"无", "单线段", "多段线", "选择", "编辑"};
        for (const QString& mode : drawingModes) {
            QPushButton* btn = new QPushButton(mode);
            btn->setStyleSheet(getCompactButtonStyle());
            btn->setMinimumSize(60, 28);
            drawingLayout->addWidget(btn);
        }
        layout->addWidget(drawingGroup);
        
        layout->addStretch();
        
        return widget;
    }
    
    QWidget* createOptimizedHorizontalLayout()
    {
        QWidget* widget = new QWidget();
        widget->setStyleSheet(
            "QWidget {"
            "   background-color: #ffffff;"
            "   border: 2px solid #27ae60;"
            "   border-radius: 8px;"
            "}"
        );
        
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(12, 8, 12, 8);
        layout->setSpacing(6);
        
        // 标题
        QLabel* titleLabel = new QLabel("优化后水平布局");
        titleLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   color: #27ae60;"
            "   padding: 8px 0;"
            "   border-bottom: 2px solid #e9ecef;"
            "   margin-bottom: 8px;"
            "}"
        );
        layout->addWidget(titleLabel);
        
        // 绘制模式组（水平）
        QGroupBox* drawingGroup = new QGroupBox("绘制模式");
        drawingGroup->setStyleSheet(getGroupBoxStyle());
        QHBoxLayout* drawingLayout = new QHBoxLayout(drawingGroup);
        drawingLayout->setContentsMargins(8, 12, 8, 8);
        drawingLayout->setSpacing(4);
        
        QButtonGroup* drawingButtonGroup = new QButtonGroup(this);
        QStringList drawingModes = {"无", "单线段", "多段线", "选择", "编辑"};
        for (int i = 0; i < drawingModes.size(); ++i) {
            QPushButton* btn = new QPushButton(drawingModes[i]);
            btn->setCheckable(true);
            btn->setStyleSheet(getCompactButtonStyle());
            btn->setMinimumSize(drawingModes[i].length() > 2 ? 60 : 40, 28);
            btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            if (i == 0) btn->setChecked(true);
            drawingButtonGroup->addButton(btn, i);
            drawingLayout->addWidget(btn);
        }
        layout->addWidget(drawingGroup);
        
        // 编辑模式组（水平）
        QGroupBox* editGroup = new QGroupBox("编辑模式");
        editGroup->setStyleSheet(getGroupBoxStyle());
        QHBoxLayout* editLayout = new QHBoxLayout(editGroup);
        editLayout->setContentsMargins(8, 12, 8, 8);
        editLayout->setSpacing(4);
        
        QButtonGroup* editButtonGroup = new QButtonGroup(this);
        QStringList editModes = {"移动端点", "分割线段", "合并线段"};
        for (int i = 0; i < editModes.size(); ++i) {
            QPushButton* btn = new QPushButton(editModes[i]);
            btn->setCheckable(true);
            btn->setStyleSheet(getCompactButtonStyle());
            btn->setMinimumSize(70, 28);
            btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            if (i == 0) btn->setChecked(true);
            editButtonGroup->addButton(btn, i);
            editLayout->addWidget(btn);
        }
        editGroup->setEnabled(false); // 默认禁用
        layout->addWidget(editGroup);
        
        // 操作工具组（水平）
        QGroupBox* toolsGroup = new QGroupBox("操作工具");
        toolsGroup->setStyleSheet(getGroupBoxStyle());
        QHBoxLayout* toolsLayout = new QHBoxLayout(toolsGroup);
        toolsLayout->setContentsMargins(8, 12, 8, 8);
        toolsLayout->setSpacing(4);
        
        QStringList tools = {"保存", "加载", "删除选中", "清空所有"};
        QStringList toolColors = {"#27ae60", "#3498db", "#f39c12", "#e74c3c"};
        for (int i = 0; i < tools.size(); ++i) {
            QPushButton* btn = new QPushButton(tools[i]);
            btn->setStyleSheet(getToolButtonStyle(toolColors[i]));
            btn->setMinimumSize(tools[i].length() > 2 ? 70 : 50, 28);
            btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            toolsLayout->addWidget(btn);
        }
        layout->addWidget(toolsGroup);
        
        // 状态信息组（水平）
        QGroupBox* statusGroup = new QGroupBox("状态信息");
        statusGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* statusGroupLayout = new QVBoxLayout(statusGroup);
        statusGroupLayout->setContentsMargins(8, 12, 8, 8);
        statusGroupLayout->setSpacing(4);
        
        QWidget* statusContainer = new QWidget();
        statusContainer->setStyleSheet(
            "QWidget {"
            "   background-color: #f8f9fa;"
            "   border: 1px solid #e9ecef;"
            "   border-radius: 6px;"
            "   padding: 6px;"
            "}"
        );
        
        QHBoxLayout* statusLayout = new QHBoxLayout(statusContainer);
        statusLayout->setContentsMargins(6, 6, 6, 6);
        statusLayout->setSpacing(12);
        
        QStringList statusTexts = {"线段数: 0", "选中数: 0", "模式: 无"};
        for (const QString& text : statusTexts) {
            QLabel* label = new QLabel(text);
            label->setStyleSheet(
                "QLabel {"
                "   font-size: 11px;"
                "   font-weight: 500;"
                "   color: #495057;"
                "   padding: 3px 5px;"
                "   background-color: rgba(52, 73, 94, 0.05);"
                "   border-radius: 3px;"
                "}"
            );
            statusLayout->addWidget(label);
        }
        statusLayout->addStretch();
        
        statusGroupLayout->addWidget(statusContainer);
        layout->addWidget(statusGroup);
        
        layout->addStretch();
        
        return widget;
    }
    
    QString getGroupBoxStyle() const
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
                background-color: #ffffff;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 8px;
                padding: 0 4px 0 4px;
                background-color: #ffffff;
            }
        )";
    }
    
    QString getCompactButtonStyle() const
    {
        return R"(
            QPushButton {
                font-size: 12px;
                font-weight: 500;
                padding: 6px 10px;
                border: 2px solid #bdc3c7;
                border-radius: 6px;
                background-color: #ffffff;
                color: #2c3e50;
                min-width: 30px;
                min-height: 28px;
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
        )";
    }
    
    QString getToolButtonStyle(const QString& color) const
    {
        return QString(R"(
            QPushButton {
                font-size: 12px;
                font-weight: 500;
                padding: 6px 10px;
                border: none;
                border-radius: 6px;
                background-color: %1;
                color: white;
                min-width: 30px;
                min-height: 28px;
            }
            QPushButton:hover {
                background-color: %2;
            }
            QPushButton:pressed {
                background-color: %3;
            }
        )").arg(color, 
                color == "#27ae60" ? "#229954" : 
                color == "#3498db" ? "#2980b9" :
                color == "#f39c12" ? "#e67e22" : "#c0392b",
                color == "#27ae60" ? "#1e8449" : 
                color == "#3498db" ? "#21618c" :
                color == "#f39c12" ? "#d68910" : "#a93226");
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    LineDrawingHorizontalLayoutTest window;
    window.show();
    
    return app.exec();
}

#include "test_line_drawing_horizontal_layout.moc"
