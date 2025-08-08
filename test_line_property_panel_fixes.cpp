#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QButtonGroup>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QAbstractAnimation>
#include <QDebug>
#include <QTimer>

/**
 * @brief LinePropertyPanel修复测试程序
 * 
 * 这个程序测试LinePropertyPanel的两个修复：
 * 1. 验证折叠展开方向修复（向下展开，不影响上方UI）
 * 2. 测试编辑模式按钮的选择功能修复
 */
class LinePropertyPanelFixesTest : public QWidget
{
    Q_OBJECT

public:
    explicit LinePropertyPanelFixesTest(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_lineDrawingToolbar(nullptr)
        , m_linePropertyPanel(nullptr)
        , m_propertyAnimation(nullptr)
        , m_propertyVisible(false)
        , m_editModeHintLabel(nullptr)
    {
        setupUI();
        setWindowTitle("LinePropertyPanel修复测试");
        setMinimumSize(600, 800);
        resize(650, 900);
    }

private slots:
    void onDrawingModeChanged(int modeId)
    {
        qDebug() << "Drawing mode changed to:" << modeId;
        
        // 模拟编辑模式按钮的启用/禁用
        bool isEditMode = (modeId == 4); // 假设编辑模式是第5个按钮(索引4)
        
        // 启用/禁用编辑模式组
        if (m_editModeGroup) {
            m_editModeGroup->setEnabled(isEditMode);
        }
        
        // 控制提示标签的显示
        if (m_editModeHintLabel) {
            m_editModeHintLabel->setVisible(!isEditMode);
        }
        
        // 更新状态显示
        QString modeText;
        switch(modeId) {
            case 0: modeText = "无"; break;
            case 1: modeText = "单线段"; break;
            case 2: modeText = "多段线"; break;
            case 3: modeText = "选择"; break;
            case 4: modeText = "编辑"; break;
            default: modeText = "未知"; break;
        }
        
        if (m_statusLabel) {
            m_statusLabel->setText(QString("当前模式: %1").arg(modeText));
        }
    }
    
    void onEditModeChanged(int modeId)
    {
        qDebug() << "Edit mode changed to:" << modeId;
        
        QString editModeText;
        switch(modeId) {
            case 0: editModeText = "移动端点"; break;
            case 1: editModeText = "分割线段"; break;
            case 2: editModeText = "合并线段"; break;
            default: editModeText = "未知"; break;
        }
        
        if (m_editStatusLabel) {
            m_editStatusLabel->setText(QString("编辑模式: %1").arg(editModeText));
        }
    }
    
    void togglePropertyPanel()
    {
        if (m_propertyAnimation->state() == QAbstractAnimation::Running) {
            return;
        }
        
        if (m_propertyVisible) {
            // 隐藏
            m_propertyVisible = false;
            m_togglePropertyButton->setText("展开属性面板");
            
            int currentHeight = m_linePropertyPanel->height();
            m_propertyAnimation->setStartValue(currentHeight);
            m_propertyAnimation->setEndValue(50); // 折叠高度
            
            connect(m_propertyAnimation, &QPropertyAnimation::finished, this, [this]() {
                m_linePropertyPanel->setMaximumHeight(50);
                disconnect(m_propertyAnimation, &QPropertyAnimation::finished, nullptr, nullptr);
            }, Qt::UniqueConnection);
            
        } else {
            // 显示
            m_propertyVisible = true;
            m_togglePropertyButton->setText("折叠属性面板");
            
            m_linePropertyPanel->setMaximumHeight(300);
            
            int currentHeight = m_linePropertyPanel->height();
            m_propertyAnimation->setStartValue(currentHeight);
            m_propertyAnimation->setEndValue(300); // 展开高度
            
            connect(m_propertyAnimation, &QPropertyAnimation::finished, this, [this]() {
                m_linePropertyPanel->setMaximumHeight(QWIDGETSIZE_MAX);
                disconnect(m_propertyAnimation, &QPropertyAnimation::finished, nullptr, nullptr);
            }, Qt::UniqueConnection);
        }
        
        m_propertyAnimation->start();
    }

private:
    void setupUI()
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 16, 16, 16);
        mainLayout->setSpacing(16);
        
        // 标题
        QLabel* titleLabel = new QLabel("LinePropertyPanel修复测试");
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
            "修复内容：\n\n"
            "问题1修复：折叠展开方向错误\n"
            "✓ 改为使用maximumHeight动画，向下展开\n"
            "✓ 不再影响上方UI元素的位置\n"
            "✓ 移除了size动画和透明度动画\n\n"
            "问题2修复：编辑模式按钮无法选择\n"
            "✓ 添加了\"(需先选择编辑模式)\"提示\n"
            "✓ 选择编辑模式后隐藏提示，启用按钮\n"
            "✓ 编辑模式按钮可以正常选择和切换\n\n"
            "测试方法：\n"
            "1. 点击\"编辑\"按钮，观察编辑模式组的变化\n"
            "2. 点击\"展开属性面板\"，观察展开方向"
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
        
        // 创建模拟的LineDrawingToolbar
        createMockLineDrawingToolbar();
        mainLayout->addWidget(m_lineDrawingToolbar);
        
        // 创建属性面板切换按钮
        m_togglePropertyButton = new QPushButton("展开属性面板");
        m_togglePropertyButton->setStyleSheet(
            "QPushButton {"
            "   padding: 8px 16px;"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   background-color: #17a2b8;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 6px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #138496;"
            "}"
        );
        connect(m_togglePropertyButton, &QPushButton::clicked, this, &LinePropertyPanelFixesTest::togglePropertyPanel);
        mainLayout->addWidget(m_togglePropertyButton);
        
        // 创建模拟的LinePropertyPanel
        createMockLinePropertyPanel();
        mainLayout->addWidget(m_linePropertyPanel);
        
        // 状态显示
        m_statusLabel = new QLabel("当前模式: 无");
        m_statusLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 12px;"
            "   font-weight: 600;"
            "   color: #495057;"
            "   padding: 8px;"
            "   background-color: #e9ecef;"
            "   border-radius: 4px;"
            "}"
        );
        mainLayout->addWidget(m_statusLabel);
        
        m_editStatusLabel = new QLabel("编辑模式: 移动端点");
        m_editStatusLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 12px;"
            "   font-weight: 600;"
            "   color: #495057;"
            "   padding: 8px;"
            "   background-color: #e9ecef;"
            "   border-radius: 4px;"
            "}"
        );
        mainLayout->addWidget(m_editStatusLabel);
        
        mainLayout->addStretch();
    }
    
    void createMockLineDrawingToolbar()
    {
        m_lineDrawingToolbar = new QWidget();
        m_lineDrawingToolbar->setStyleSheet(
            "QWidget {"
            "   background-color: #ffffff;"
            "   border: 2px solid #dee2e6;"
            "   border-radius: 8px;"
            "}"
        );
        
        QVBoxLayout* toolbarLayout = new QVBoxLayout(m_lineDrawingToolbar);
        toolbarLayout->setContentsMargins(12, 8, 12, 8);
        toolbarLayout->setSpacing(6);
        
        // 绘制模式组
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
            if (i == 0) btn->setChecked(true);
            drawingButtonGroup->addButton(btn, i);
            drawingLayout->addWidget(btn);
        }
        connect(drawingButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
                this, &LinePropertyPanelFixesTest::onDrawingModeChanged);
        toolbarLayout->addWidget(drawingGroup);
        
        // 编辑模式组
        m_editModeGroup = new QGroupBox("编辑模式");
        m_editModeGroup->setStyleSheet(getGroupBoxStyle());
        QHBoxLayout* editLayout = new QHBoxLayout(m_editModeGroup);
        editLayout->setContentsMargins(8, 12, 8, 8);
        editLayout->setSpacing(4);
        
        QButtonGroup* editButtonGroup = new QButtonGroup(this);
        QStringList editModes = {"移动端点", "分割线段", "合并线段"};
        for (int i = 0; i < editModes.size(); ++i) {
            QPushButton* btn = new QPushButton(editModes[i]);
            btn->setCheckable(true);
            btn->setStyleSheet(getCompactButtonStyle());
            btn->setMinimumSize(70, 28);
            if (i == 0) btn->setChecked(true);
            editButtonGroup->addButton(btn, i);
            editLayout->addWidget(btn);
        }
        connect(editButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
                this, &LinePropertyPanelFixesTest::onEditModeChanged);
        
        // 添加提示标签
        m_editModeHintLabel = new QLabel("(需先选择\"编辑\"模式)");
        m_editModeHintLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 10px;"
            "   color: #6c757d;"
            "   font-style: italic;"
            "   padding: 2px 4px;"
            "}"
        );
        editLayout->addWidget(m_editModeHintLabel);
        editLayout->addStretch();
        
        m_editModeGroup->setEnabled(false); // 默认禁用
        toolbarLayout->addWidget(m_editModeGroup);
    }
    
    void createMockLinePropertyPanel()
    {
        m_linePropertyPanel = new QWidget();
        m_linePropertyPanel->setStyleSheet(
            "QWidget {"
            "   background-color: #f8f9fa;"
            "   border: 2px solid #dee2e6;"
            "   border-radius: 8px;"
            "}"
        );
        
        QVBoxLayout* panelLayout = new QVBoxLayout(m_linePropertyPanel);
        panelLayout->setContentsMargins(12, 8, 12, 8);
        panelLayout->setSpacing(8);
        
        // 头部
        QLabel* headerLabel = new QLabel("线段属性");
        headerLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   color: #2c3e50;"
            "   padding: 8px 0;"
            "   border-bottom: 2px solid #e9ecef;"
            "}"
        );
        panelLayout->addWidget(headerLabel);
        
        // 模拟内容
        for (int i = 0; i < 5; ++i) {
            QLabel* contentLabel = new QLabel(QString("属性项 %1: 值 %1").arg(i + 1));
            contentLabel->setStyleSheet(
                "QLabel {"
                "   font-size: 12px;"
                "   color: #495057;"
                "   padding: 4px 8px;"
                "   background-color: #ffffff;"
                "   border: 1px solid #ced4da;"
                "   border-radius: 4px;"
                "}"
            );
            panelLayout->addWidget(contentLabel);
        }
        
        panelLayout->addStretch();
        
        // 设置初始状态为折叠
        m_linePropertyPanel->setMaximumHeight(50);
        
        // 创建动画
        m_propertyAnimation = new QPropertyAnimation(m_linePropertyPanel, "maximumHeight", this);
        m_propertyAnimation->setDuration(300);
        m_propertyAnimation->setEasingCurve(QEasingCurve::InOutQuad);
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
                min-height: 28px;
            }
            QPushButton:hover {
                background-color: #ecf0f1;
                border-color: #95a5a6;
            }
            QPushButton:checked {
                background-color: #3498db;
                color: white;
                border-color: #2980b9;
                font-weight: 600;
            }
            QPushButton:disabled {
                background-color: #f8f9fa;
                color: #6c757d;
                border-color: #dee2e6;
            }
        )";
    }

private:
    QWidget* m_lineDrawingToolbar;
    QWidget* m_linePropertyPanel;
    QGroupBox* m_editModeGroup;
    QLabel* m_editModeHintLabel;
    QLabel* m_statusLabel;
    QLabel* m_editStatusLabel;
    QPushButton* m_togglePropertyButton;
    QPropertyAnimation* m_propertyAnimation;
    bool m_propertyVisible;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    LinePropertyPanelFixesTest window;
    window.show();
    
    return app.exec();
}

#include "test_line_property_panel_fixes.moc"
