#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QButtonGroup>
#include <QScrollArea>
#include <QDebug>

/**
 * @brief LineDrawingToolbar UI优化测试程序
 * 
 * 这个程序测试LineDrawingToolbar的UI布局优化：
 * 1. 验证垂直布局的效果
 * 2. 测试紧凑按钮尺寸
 * 3. 验证QComboBox自适应尺寸
 * 4. 测试整体布局的紧凑性
 */
class LineDrawingUIOptimizationTest : public QWidget
{
    Q_OBJECT

public:
    explicit LineDrawingUIOptimizationTest(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setupUI();
        setWindowTitle("LineDrawingToolbar UI优化测试");
        setMinimumSize(400, 700);
        resize(450, 800);
    }

private:
    void setupUI()
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 16, 16, 16);
        mainLayout->setSpacing(16);
        
        // 标题
        QLabel* titleLabel = new QLabel("LineDrawingToolbar UI布局优化测试");
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
            "UI优化内容：\n\n"
            "✓ 所有按钮组改为垂直排列\n"
            "✓ 按钮尺寸优化为紧凑型(28px高度)\n"
            "✓ 字体大小调整为12px，确保中文清晰显示\n"
            "✓ 减少边距和间距，整体布局更紧凑\n"
            "✓ QComboBox自适应内容宽度\n"
            "✓ 状态信息显示区域优化\n\n"
            "对比效果：界面高度从120px增加到约400px，但宽度大幅减少"
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
        
        // 创建滚动区域以容纳优化后的LineDrawingToolbar
        QScrollArea* scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        
        // 模拟优化后的LineDrawingToolbar
        QWidget* optimizedToolbar = createOptimizedLineDrawingToolbar();
        scrollArea->setWidget(optimizedToolbar);
        
        mainLayout->addWidget(scrollArea);
        mainLayout->addStretch();
    }
    
    QWidget* createOptimizedLineDrawingToolbar()
    {
        QWidget* toolbar = new QWidget();
        toolbar->setStyleSheet(
            "QWidget {"
            "   background-color: #ffffff;"
            "   border: 2px solid #dee2e6;"
            "   border-radius: 8px;"
            "}"
        );
        
        QVBoxLayout* mainLayout = new QVBoxLayout(toolbar);
        mainLayout->setContentsMargins(16, 12, 16, 12);
        mainLayout->setSpacing(8);
        
        // 工具栏标题
        QLabel* titleLabel = new QLabel("线段绘制工具");
        titleLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   color: #2c3e50;"
            "   padding: 8px 0;"
            "   border-bottom: 2px solid #e9ecef;"
            "   margin-bottom: 8px;"
            "}"
        );
        mainLayout->addWidget(titleLabel);
        
        // 绘制模式组（垂直布局）
        QGroupBox* drawingModeGroup = new QGroupBox("绘制模式");
        drawingModeGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* drawingLayout = new QVBoxLayout(drawingModeGroup);
        drawingLayout->setContentsMargins(12, 16, 12, 12);
        drawingLayout->setSpacing(6);
        
        QButtonGroup* drawingButtonGroup = new QButtonGroup(this);
        QStringList drawingModes = {"无", "单线段", "多段线", "选择", "编辑"};
        for (int i = 0; i < drawingModes.size(); ++i) {
            QPushButton* btn = new QPushButton(drawingModes[i]);
            btn->setCheckable(true);
            btn->setStyleSheet(getCompactButtonStyle());
            btn->setMinimumSize(60, 28);
            btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            if (i == 0) btn->setChecked(true);
            drawingButtonGroup->addButton(btn, i);
            drawingLayout->addWidget(btn);
        }
        mainLayout->addWidget(drawingModeGroup);
        
        // 编辑模式组（垂直布局）
        QGroupBox* editModeGroup = new QGroupBox("编辑模式");
        editModeGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* editLayout = new QVBoxLayout(editModeGroup);
        editLayout->setContentsMargins(12, 16, 12, 12);
        editLayout->setSpacing(6);
        
        QButtonGroup* editButtonGroup = new QButtonGroup(this);
        QStringList editModes = {"移动端点", "分割线段", "合并线段"};
        for (int i = 0; i < editModes.size(); ++i) {
            QPushButton* btn = new QPushButton(editModes[i]);
            btn->setCheckable(true);
            btn->setStyleSheet(getCompactButtonStyle());
            btn->setMinimumSize(80, 28);
            btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            if (i == 0) btn->setChecked(true);
            editButtonGroup->addButton(btn, i);
            editLayout->addWidget(btn);
        }
        editModeGroup->setEnabled(false); // 默认禁用
        mainLayout->addWidget(editModeGroup);
        
        // 操作工具组（垂直布局）
        QGroupBox* toolsGroup = new QGroupBox("操作工具");
        toolsGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* toolsLayout = new QVBoxLayout(toolsGroup);
        toolsLayout->setContentsMargins(12, 16, 12, 12);
        toolsLayout->setSpacing(6);
        
        QStringList tools = {"保存", "加载", "删除选中", "清空所有"};
        QStringList toolColors = {"#27ae60", "#3498db", "#f39c12", "#e74c3c"};
        for (int i = 0; i < tools.size(); ++i) {
            QPushButton* btn = new QPushButton(tools[i]);
            btn->setStyleSheet(getToolButtonStyle(toolColors[i]));
            btn->setMinimumSize(tools[i].length() > 2 ? 80 : 60, 28);
            btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            toolsLayout->addWidget(btn);
        }
        mainLayout->addWidget(toolsGroup);
        
        // 状态信息组
        QGroupBox* statusGroup = new QGroupBox("状态信息");
        statusGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
        statusLayout->setContentsMargins(12, 16, 12, 12);
        statusLayout->setSpacing(6);
        
        QWidget* statusContainer = new QWidget();
        statusContainer->setStyleSheet(
            "QWidget {"
            "   background-color: #f8f9fa;"
            "   border: 1px solid #e9ecef;"
            "   border-radius: 6px;"
            "   padding: 8px;"
            "}"
        );
        
        QVBoxLayout* statusContainerLayout = new QVBoxLayout(statusContainer);
        statusContainerLayout->setContentsMargins(8, 8, 8, 8);
        statusContainerLayout->setSpacing(4);
        
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
                "   margin: 1px 0px;"
                "}"
            );
            statusContainerLayout->addWidget(label);
        }
        
        statusLayout->addWidget(statusContainer);
        mainLayout->addWidget(statusGroup);
        
        // 添加QComboBox自适应尺寸测试
        QGroupBox* comboTestGroup = new QGroupBox("下拉框自适应测试");
        comboTestGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* comboLayout = new QVBoxLayout(comboTestGroup);
        comboLayout->setContentsMargins(12, 16, 12, 12);
        comboLayout->setSpacing(6);
        
        // 短文本下拉框
        QComboBox* shortCombo = new QComboBox();
        shortCombo->addItems({"无", "有"});
        shortCombo->setStyleSheet(getAdaptiveComboBoxStyle());
        shortCombo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        comboLayout->addWidget(new QLabel("短文本:"));
        comboLayout->addWidget(shortCombo);
        
        // 长文本下拉框
        QComboBox* longCombo = new QComboBox();
        longCombo->addItems({"均匀采样策略", "体素网格采样", "随机采样算法", "自适应密度采样"});
        longCombo->setStyleSheet(getAdaptiveComboBoxStyle());
        longCombo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        comboLayout->addWidget(new QLabel("长文本:"));
        comboLayout->addWidget(longCombo);
        
        mainLayout->addWidget(comboTestGroup);
        
        mainLayout->addStretch(1);
        
        return toolbar;
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
                min-width: 50px;
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
                min-width: 50px;
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
    
    QString getAdaptiveComboBoxStyle() const
    {
        return R"(
            QComboBox {
                font-size: 12px;
                font-weight: 500;
                padding: 6px 10px;
                border: 2px solid #bdc3c7;
                border-radius: 6px;
                background-color: white;
                color: #2c3e50;
                min-height: 28px;
            }
            QComboBox:hover {
                border-color: #95a5a6;
            }
            QComboBox:focus {
                border-color: #3498db;
            }
            QComboBox::drop-down {
                border: none;
                width: 20px;
            }
            QComboBox::down-arrow {
                image: none;
                border-left: 5px solid transparent;
                border-right: 5px solid transparent;
                border-top: 5px solid #7f8c8d;
                margin-right: 5px;
            }
        )";
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    LineDrawingUIOptimizationTest window;
    window.show();
    
    return app.exec();
}

#include "test_line_drawing_ui_optimization.moc"
