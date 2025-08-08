#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QAbstractAnimation>
#include <QDebug>
#include <QTimer>
#include <QButtonGroup>

/**
 * @brief 线框绘制工具折叠功能测试程序
 * 
 * 这个程序测试Stage1DemoWidget中新增的线框绘制工具折叠功能：
 * 1. 模拟LineDrawingToolbar的UI布局
 * 2. 验证折叠/展开功能
 * 3. 测试与渲染参数面板的一致性设计
 * 4. 验证动画效果和用户交互
 */
class LineDrawingToggleTest : public QWidget
{
    Q_OBJECT

public:
    explicit LineDrawingToggleTest(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_toggleRenderParamsButton(nullptr)
        , m_renderParamsContainer(nullptr)
        , m_renderParamsAnimation(nullptr)
        , m_renderParamsVisible(false)
        , m_toggleLineDrawingButton(nullptr)
        , m_lineDrawingContainer(nullptr)
        , m_lineDrawingAnimation(nullptr)
        , m_lineDrawingVisible(false)
    {
        setupUI();
        setWindowTitle("线框绘制工具折叠功能测试");
        setMinimumSize(600, 800);
        resize(650, 900);
    }

private slots:
    void toggleRenderParams()
    {
        if (m_renderParamsAnimation->state() == QAbstractAnimation::Running) {
            return;
        }

        disconnect(m_renderParamsAnimation, &QPropertyAnimation::finished, nullptr, nullptr);

        if (m_renderParamsVisible) {
            m_renderParamsVisible = false;
            syncRenderParamsButtonState();
            
            int currentHeight = m_renderParamsContainer->height();
            if (currentHeight <= 0) currentHeight = 200;
            
            m_renderParamsAnimation->setStartValue(currentHeight);
            m_renderParamsAnimation->setEndValue(0);
            
            connect(m_renderParamsAnimation, &QPropertyAnimation::finished, this, [this]() {
                m_renderParamsContainer->hide();
                m_renderParamsContainer->setMaximumHeight(0);
            });
        } else {
            m_renderParamsVisible = true;
            syncRenderParamsButtonState();
            
            m_renderParamsContainer->show();
            int contentHeight = 200;
            m_renderParamsContainer->setMaximumHeight(contentHeight);
            
            m_renderParamsAnimation->setStartValue(0);
            m_renderParamsAnimation->setEndValue(contentHeight);
            
            connect(m_renderParamsAnimation, &QPropertyAnimation::finished, this, [this]() {
                m_renderParamsContainer->setMaximumHeight(QWIDGETSIZE_MAX);
            });
        }
        
        m_renderParamsAnimation->start();
    }
    
    void toggleLineDrawingTools()
    {
        if (m_lineDrawingAnimation->state() == QAbstractAnimation::Running) {
            return;
        }

        disconnect(m_lineDrawingAnimation, &QPropertyAnimation::finished, nullptr, nullptr);

        if (m_lineDrawingVisible) {
            m_lineDrawingVisible = false;
            syncLineDrawingButtonState();
            
            int currentHeight = m_lineDrawingContainer->height();
            if (currentHeight <= 0) currentHeight = 300;
            
            m_lineDrawingAnimation->setStartValue(currentHeight);
            m_lineDrawingAnimation->setEndValue(0);
            
            connect(m_lineDrawingAnimation, &QPropertyAnimation::finished, this, [this]() {
                m_lineDrawingContainer->hide();
                m_lineDrawingContainer->setMaximumHeight(0);
            });
        } else {
            m_lineDrawingVisible = true;
            syncLineDrawingButtonState();
            
            m_lineDrawingContainer->show();
            int contentHeight = 300;
            m_lineDrawingContainer->setMaximumHeight(contentHeight);
            
            m_lineDrawingAnimation->setStartValue(0);
            m_lineDrawingAnimation->setEndValue(contentHeight);
            
            connect(m_lineDrawingAnimation, &QPropertyAnimation::finished, this, [this]() {
                m_lineDrawingContainer->setMaximumHeight(QWIDGETSIZE_MAX);
            });
        }
        
        m_lineDrawingAnimation->start();
    }

private:
    void setupUI()
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 16, 16, 16);
        mainLayout->setSpacing(16);
        
        // 标题
        QLabel* titleLabel = new QLabel("线框绘制工具折叠功能测试");
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
            "新增功能：线框绘制工具可折叠设计\n\n"
            "设计特点：\n"
            "• 与渲染参数面板保持一致的设计风格\n"
            "• 相同的折叠/展开动画效果(300ms)\n"
            "• 统一的按钮状态变化(蓝色→红色)\n"
            "• 默认隐藏状态，界面更简洁\n\n"
            "测试方法：分别点击两个按钮，观察折叠效果的一致性"
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
        
        // 模拟Stage1DemoWidget的控制面板布局
        QWidget* controlPanel = new QWidget();
        controlPanel->setStyleSheet(
            "QWidget {"
            "   background-color: #ffffff;"
            "   border: 1px solid #dee2e6;"
            "   border-radius: 8px;"
            "   padding: 8px;"
            "}"
        );
        
        QVBoxLayout* panelLayout = new QVBoxLayout(controlPanel);
        panelLayout->setContentsMargins(12, 12, 12, 12);
        panelLayout->setSpacing(16);
        
        // 渲染参数切换按钮
        createRenderParamsToggleButton();
        panelLayout->addWidget(m_toggleRenderParamsButton);
        
        // 渲染参数容器
        createRenderParamsContainer();
        panelLayout->addWidget(m_renderParamsContainer);
        
        // 线框绘制工具切换按钮
        createLineDrawingToggleButton();
        panelLayout->addWidget(m_toggleLineDrawingButton);
        
        // 线框绘制工具容器
        createLineDrawingContainer();
        panelLayout->addWidget(m_lineDrawingContainer);
        
        panelLayout->addStretch();
        
        mainLayout->addWidget(controlPanel);
        mainLayout->addStretch();
    }
    
    void createRenderParamsToggleButton()
    {
        m_toggleRenderParamsButton = new QPushButton("渲染参数", this);
        m_toggleRenderParamsButton->setCheckable(true);
        m_toggleRenderParamsButton->setChecked(false);
        m_toggleRenderParamsButton->setStyleSheet(getToggleButtonStyle());
        
        connect(m_toggleRenderParamsButton, &QPushButton::clicked,
                this, [this](bool checked) {
                    m_toggleRenderParamsButton->setChecked(m_renderParamsVisible);
                    toggleRenderParams();
                });
    }
    
    void createRenderParamsContainer()
    {
        m_renderParamsContainer = new QWidget(this);
        m_renderParamsContainer->setStyleSheet(getContainerStyle());
        
        QVBoxLayout* layout = new QVBoxLayout(m_renderParamsContainer);
        layout->setContentsMargins(12, 12, 12, 12);
        layout->setSpacing(12);
        
        QLabel* titleLabel = new QLabel("渲染参数设置");
        titleLabel->setStyleSheet(getTitleStyle());
        layout->addWidget(titleLabel);
        
        // 添加一些模拟内容
        QGroupBox* group = new QGroupBox("LOD控制");
        group->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* groupLayout = new QVBoxLayout(group);
        
        QComboBox* combo = new QComboBox();
        combo->addItems({"均匀采样", "体素网格", "随机采样"});
        combo->setStyleSheet(getComboBoxStyle());
        groupLayout->addWidget(combo);
        
        layout->addWidget(group);
        
        m_renderParamsAnimation = new QPropertyAnimation(m_renderParamsContainer, "maximumHeight", this);
        m_renderParamsAnimation->setDuration(300);
        m_renderParamsAnimation->setEasingCurve(QEasingCurve::InOutQuad);
        
        m_renderParamsContainer->setMaximumHeight(0);
        m_renderParamsContainer->hide();
        m_renderParamsVisible = false;
    }
    
    void createLineDrawingToggleButton()
    {
        m_toggleLineDrawingButton = new QPushButton("线框绘制工具", this);
        m_toggleLineDrawingButton->setCheckable(true);
        m_toggleLineDrawingButton->setChecked(false);
        m_toggleLineDrawingButton->setStyleSheet(getToggleButtonStyle());
        
        connect(m_toggleLineDrawingButton, &QPushButton::clicked,
                this, [this](bool checked) {
                    m_toggleLineDrawingButton->setChecked(m_lineDrawingVisible);
                    toggleLineDrawingTools();
                });
    }
    
    void createLineDrawingContainer()
    {
        m_lineDrawingContainer = new QWidget(this);
        m_lineDrawingContainer->setStyleSheet(getContainerStyle());
        
        QVBoxLayout* layout = new QVBoxLayout(m_lineDrawingContainer);
        layout->setContentsMargins(12, 12, 12, 12);
        layout->setSpacing(12);
        
        QLabel* titleLabel = new QLabel("线框绘制工具");
        titleLabel->setStyleSheet(getTitleStyle());
        layout->addWidget(titleLabel);
        
        // 模拟LineDrawingToolbar
        QGroupBox* drawingGroup = new QGroupBox("绘制模式");
        drawingGroup->setStyleSheet(getGroupBoxStyle());
        QHBoxLayout* drawingLayout = new QHBoxLayout(drawingGroup);
        
        QButtonGroup* buttonGroup = new QButtonGroup(this);
        QStringList modes = {"无", "单线段", "多段线", "选择", "编辑"};
        for (int i = 0; i < modes.size(); ++i) {
            QPushButton* btn = new QPushButton(modes[i]);
            btn->setCheckable(true);
            btn->setStyleSheet(getModeButtonStyle());
            if (i == 0) btn->setChecked(true);
            buttonGroup->addButton(btn, i);
            drawingLayout->addWidget(btn);
        }
        
        layout->addWidget(drawingGroup);
        
        // 模拟工具按钮
        QGroupBox* toolsGroup = new QGroupBox("操作工具");
        toolsGroup->setStyleSheet(getGroupBoxStyle());
        QHBoxLayout* toolsLayout = new QHBoxLayout(toolsGroup);
        
        QStringList tools = {"保存", "加载", "删除选中", "清空所有"};
        for (const QString& tool : tools) {
            QPushButton* btn = new QPushButton(tool);
            btn->setStyleSheet(getToolButtonStyle());
            toolsLayout->addWidget(btn);
        }
        
        layout->addWidget(toolsGroup);
        
        m_lineDrawingAnimation = new QPropertyAnimation(m_lineDrawingContainer, "maximumHeight", this);
        m_lineDrawingAnimation->setDuration(300);
        m_lineDrawingAnimation->setEasingCurve(QEasingCurve::InOutQuad);
        
        m_lineDrawingContainer->setMaximumHeight(0);
        m_lineDrawingContainer->hide();
        m_lineDrawingVisible = false;
    }
    
    void syncRenderParamsButtonState()
    {
        if (m_renderParamsVisible) {
            m_toggleRenderParamsButton->setText("隐藏参数");
            m_toggleRenderParamsButton->setChecked(true);
        } else {
            m_toggleRenderParamsButton->setText("渲染参数");
            m_toggleRenderParamsButton->setChecked(false);
        }
    }
    
    void syncLineDrawingButtonState()
    {
        if (m_lineDrawingVisible) {
            m_toggleLineDrawingButton->setText("隐藏工具");
            m_toggleLineDrawingButton->setChecked(true);
        } else {
            m_toggleLineDrawingButton->setText("线框绘制工具");
            m_toggleLineDrawingButton->setChecked(false);
        }
    }
    
    QString getToggleButtonStyle() const
    {
        return R"(
            QPushButton {
                padding: 12px 20px;
                font-size: 14px;
                font-weight: 600;
                background-color: #3498db;
                color: white;
                border: none;
                border-radius: 6px;
                min-height: 20px;
            }
            QPushButton:hover {
                background-color: #2980b9;
            }
            QPushButton:pressed {
                background-color: #21618c;
            }
            QPushButton:checked {
                background-color: #e74c3c;
            }
            QPushButton:checked:hover {
                background-color: #c0392b;
            }
        )";
    }
    
    QString getContainerStyle() const
    {
        return R"(
            QWidget {
                background-color: #f8f9fa;
                border: 2px solid #dee2e6;
                border-radius: 8px;
                padding: 4px;
            }
        )";
    }
    
    QString getTitleStyle() const
    {
        return R"(
            QLabel {
                font-weight: bold;
                font-size: 15px;
                color: #212529;
                padding: 8px 0;
                border-bottom: 2px solid #e9ecef;
                margin-bottom: 8px;
            }
        )";
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
    
    QString getComboBoxStyle() const
    {
        return R"(
            QComboBox {
                padding: 6px 10px;
                font-size: 12px;
                border: 2px solid #bdc3c7;
                border-radius: 4px;
                background-color: white;
                min-height: 20px;
            }
        )";
    }
    
    QString getModeButtonStyle() const
    {
        return R"(
            QPushButton {
                padding: 8px 12px;
                font-size: 12px;
                font-weight: 500;
                background-color: #ffffff;
                color: #2c3e50;
                border: 2px solid #bdc3c7;
                border-radius: 4px;
                min-width: 60px;
            }
            QPushButton:hover {
                background-color: #ecf0f1;
                border-color: #95a5a6;
            }
            QPushButton:checked {
                background-color: #3498db;
                color: white;
                border-color: #2980b9;
            }
        )";
    }
    
    QString getToolButtonStyle() const
    {
        return R"(
            QPushButton {
                padding: 8px 12px;
                font-size: 12px;
                font-weight: 500;
                background-color: #27ae60;
                color: white;
                border: none;
                border-radius: 4px;
                min-width: 60px;
            }
            QPushButton:hover {
                background-color: #229954;
            }
        )";
    }

private:
    // 渲染参数相关
    QPushButton* m_toggleRenderParamsButton;
    QWidget* m_renderParamsContainer;
    QPropertyAnimation* m_renderParamsAnimation;
    bool m_renderParamsVisible;
    
    // 线框绘制工具相关
    QPushButton* m_toggleLineDrawingButton;
    QWidget* m_lineDrawingContainer;
    QPropertyAnimation* m_lineDrawingAnimation;
    bool m_lineDrawingVisible;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    LineDrawingToggleTest window;
    window.show();
    
    return app.exec();
}

#include "test_line_drawing_toggle.moc"
