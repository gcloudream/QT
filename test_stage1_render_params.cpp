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
#include <QDebug>

/**
 * @brief Stage1DemoWidget渲染参数折叠功能验证程序
 * 
 * 这个程序验证了Stage1DemoWidget中新增的渲染参数折叠功能：
 * 1. 模拟Stage1DemoWidget的控制面板布局
 * 2. 验证渲染参数的折叠/展开功能
 * 3. 测试动画效果和用户交互
 */
class Stage1RenderParamsTest : public QWidget
{
    Q_OBJECT

public:
    explicit Stage1RenderParamsTest(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_toggleRenderParamsButton(nullptr)
        , m_renderParamsContainer(nullptr)
        , m_renderParamsAnimation(nullptr)
        , m_renderParamsVisible(false)
    {
        setupUI();
        setWindowTitle("Stage1DemoWidget - 渲染参数折叠功能验证");
        setMinimumSize(400, 700);
        resize(450, 800);
    }

private slots:
    void toggleRenderParams()
    {
        if (m_renderParamsVisible) {
            // 隐藏参数面板
            m_renderParamsAnimation->setStartValue(m_renderParamsContainer->height());
            m_renderParamsAnimation->setEndValue(0);
            
            // 断开之前的连接，避免重复连接
            disconnect(m_renderParamsAnimation, &QPropertyAnimation::finished, 
                       m_renderParamsContainer, &QWidget::hide);
            connect(m_renderParamsAnimation, &QPropertyAnimation::finished, 
                    m_renderParamsContainer, &QWidget::hide, Qt::UniqueConnection);
            m_renderParamsAnimation->start();
            
            m_toggleRenderParamsButton->setText("渲染参数");
            m_toggleRenderParamsButton->setChecked(false);
            m_renderParamsVisible = false;
            
            qDebug() << "Hiding render parameters panel";
        } else {
            // 显示参数面板
            m_renderParamsContainer->show();
            
            // 计算内容的理想高度
            int contentHeight = 0;
            for (auto* child : m_renderParamsContainer->findChildren<QWidget*>()) {
                if (child->parent() == m_renderParamsContainer) {
                    contentHeight += child->sizeHint().height();
                }
            }
            contentHeight += 100;  // 添加标题和间距的高度
            if (contentHeight < 300) contentHeight = 300;
            
            m_renderParamsAnimation->setStartValue(0);
            m_renderParamsAnimation->setEndValue(contentHeight);
            m_renderParamsAnimation->start();
            
            m_toggleRenderParamsButton->setText("隐藏参数");
            m_toggleRenderParamsButton->setChecked(true);
            m_renderParamsVisible = true;
            
            qDebug() << "Showing render parameters panel with height:" << contentHeight;
        }
        
        // 保存状态偏好
        qDebug() << "Render parameters visibility changed to:" << m_renderParamsVisible;
    }

private:
    void setupUI()
    {
        // 模拟Stage1DemoWidget的控制面板布局
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(12, 12, 12, 12);
        mainLayout->setSpacing(16);
        
        // 标题
        QLabel* titleLabel = new QLabel("Stage1DemoWidget 控制面板模拟");
        titleLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 16px;"
            "   font-weight: 600;"
            "   color: #2c3e50;"
            "   padding: 12px;"
            "   background-color: #ecf0f1;"
            "   border-radius: 8px;"
            "   margin-bottom: 8px;"
            "}"
        );
        mainLayout->addWidget(titleLabel);
        
        // 创建渲染参数切换按钮
        createRenderParamsToggleButton();
        mainLayout->addWidget(m_toggleRenderParamsButton);
        
        // 创建可折叠的渲染参数容器
        createRenderParamsContainer();
        mainLayout->addWidget(m_renderParamsContainer);
        
        // 模拟其他控件（线段绘制等）
        QLabel* otherControlsLabel = new QLabel("其他控制区域");
        otherControlsLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 14px;"
            "   font-weight: 500;"
            "   color: #495057;"
            "   padding: 8px;"
            "   background-color: #f8f9fa;"
            "   border: 1px solid #dee2e6;"
            "   border-radius: 4px;"
            "}"
        );
        mainLayout->addWidget(otherControlsLabel);
        
        // 底部弹性空间
        mainLayout->addStretch();
        
        // 状态信息
        QLabel* statusLabel = new QLabel("状态：渲染参数面板已隐藏（默认状态）");
        statusLabel->setObjectName("statusLabel");
        statusLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 11px;"
            "   color: #6c757d;"
            "   padding: 4px 8px;"
            "   background-color: #e9ecef;"
            "   border-radius: 3px;"
            "}"
        );
        mainLayout->addWidget(statusLabel);
        
        // 连接状态更新
        connect(this, &Stage1RenderParamsTest::paramsVisibilityChanged,
                [statusLabel](bool visible) {
                    statusLabel->setText(visible ? 
                        "状态：渲染参数面板已显示" : 
                        "状态：渲染参数面板已隐藏");
                });
    }
    
    void createRenderParamsToggleButton()
    {
        m_toggleRenderParamsButton = new QPushButton("渲染参数", this);
        m_toggleRenderParamsButton->setCheckable(true);
        m_toggleRenderParamsButton->setChecked(false);  // 默认隐藏
        m_toggleRenderParamsButton->setStyleSheet(
            "QPushButton {"
            "   padding: 12px 20px;"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   background-color: #3498db;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 6px;"
            "   min-height: 20px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #2980b9;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #21618c;"
            "}"
            "QPushButton:checked {"
            "   background-color: #e74c3c;"
            "}"
            "QPushButton:checked:hover {"
            "   background-color: #c0392b;"
            "}"
        );
        
        // 连接信号
        connect(m_toggleRenderParamsButton, &QPushButton::clicked, 
                this, &Stage1RenderParamsTest::toggleRenderParams);
    }
    
    void createRenderParamsContainer()
    {
        m_renderParamsContainer = new QWidget(this);
        m_renderParamsContainer->setStyleSheet(
            "QWidget {"
            "   background-color: #f8f9fa;"
            "   border: 2px solid #dee2e6;"
            "   border-radius: 8px;"
            "   padding: 4px;"
            "}"
        );
        
        QVBoxLayout* containerLayout = new QVBoxLayout(m_renderParamsContainer);
        containerLayout->setContentsMargins(8, 8, 8, 8);
        containerLayout->setSpacing(12);
        
        // 添加标题
        QLabel* titleLabel = new QLabel("渲染参数设置");
        titleLabel->setStyleSheet(
            "QLabel {"
            "   font-weight: bold;"
            "   font-size: 15px;"
            "   color: #212529;"
            "   padding: 8px 0;"
            "   border-bottom: 2px solid #e9ecef;"
            "   margin-bottom: 8px;"
            "}"
        );
        containerLayout->addWidget(titleLabel);
        
        // 模拟LOD控制区域
        QGroupBox* lodGroup = new QGroupBox("LOD细节层次");
        lodGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* lodLayout = new QVBoxLayout(lodGroup);
        
        QComboBox* lodCombo = new QComboBox();
        lodCombo->addItems({"均匀采样", "体素网格", "随机采样", "重要性采样"});
        lodCombo->setStyleSheet(getComboBoxStyle());
        lodLayout->addWidget(lodCombo);
        
        QSlider* lodSlider = new QSlider(Qt::Horizontal);
        lodSlider->setRange(0, 3);
        lodSlider->setStyleSheet(getSliderStyle());
        lodLayout->addWidget(lodSlider);
        
        containerLayout->addWidget(lodGroup);
        
        // 模拟颜色映射控制区域
        QGroupBox* colorGroup = new QGroupBox("颜色映射");
        colorGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* colorLayout = new QVBoxLayout(colorGroup);
        
        QComboBox* colorCombo = new QComboBox();
        colorCombo->addItems({"高度", "强度", "分类", "RGB"});
        colorCombo->setStyleSheet(getComboBoxStyle());
        colorLayout->addWidget(colorCombo);
        
        QHBoxLayout* rangeLayout = new QHBoxLayout();
        QDoubleSpinBox* minSpin = new QDoubleSpinBox();
        minSpin->setRange(-1000, 1000);
        minSpin->setStyleSheet(getSpinBoxStyle());
        QDoubleSpinBox* maxSpin = new QDoubleSpinBox();
        maxSpin->setRange(-1000, 1000);
        maxSpin->setValue(100);
        maxSpin->setStyleSheet(getSpinBoxStyle());
        
        rangeLayout->addWidget(new QLabel("范围:"));
        rangeLayout->addWidget(minSpin);
        rangeLayout->addWidget(new QLabel("~"));
        rangeLayout->addWidget(maxSpin);
        colorLayout->addLayout(rangeLayout);
        
        containerLayout->addWidget(colorGroup);
        
        // 模拟渲染设置控制区域
        QGroupBox* renderGroup = new QGroupBox("渲染设置");
        renderGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* renderLayout = new QVBoxLayout(renderGroup);
        
        QComboBox* modeCombo = new QComboBox();
        modeCombo->addItems({"点云", "密度图", "等高线", "热力图"});
        modeCombo->setStyleSheet(getComboBoxStyle());
        renderLayout->addWidget(modeCombo);
        
        QHBoxLayout* sizeLayout = new QHBoxLayout();
        QDoubleSpinBox* sizeSpin = new QDoubleSpinBox();
        sizeSpin->setRange(0.1, 10.0);
        sizeSpin->setValue(2.0);
        sizeSpin->setSingleStep(0.1);
        sizeSpin->setStyleSheet(getSpinBoxStyle());
        
        sizeLayout->addWidget(new QLabel("点大小:"));
        sizeLayout->addWidget(sizeSpin);
        sizeLayout->addWidget(new QLabel("px"));
        renderLayout->addLayout(sizeLayout);
        
        containerLayout->addWidget(renderGroup);
        
        // 创建动画
        m_renderParamsAnimation = new QPropertyAnimation(m_renderParamsContainer, "maximumHeight", this);
        m_renderParamsAnimation->setDuration(300);  // 300ms动画
        m_renderParamsAnimation->setEasingCurve(QEasingCurve::InOutQuad);
        
        // 默认隐藏
        m_renderParamsContainer->setMaximumHeight(0);
        m_renderParamsContainer->hide();
        m_renderParamsVisible = false;
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
            QComboBox:focus {
                border-color: #3498db;
            }
        )";
    }
    
    QString getSpinBoxStyle() const
    {
        return R"(
            QDoubleSpinBox {
                padding: 6px 8px;
                font-size: 12px;
                border: 2px solid #bdc3c7;
                border-radius: 4px;
                background-color: white;
                min-width: 80px;
            }
            QDoubleSpinBox:focus {
                border-color: #3498db;
            }
        )";
    }
    
    QString getSliderStyle() const
    {
        return R"(
            QSlider::groove:horizontal {
                border: 1px solid #bdc3c7;
                height: 6px;
                background: #ecf0f1;
                border-radius: 3px;
            }
            QSlider::handle:horizontal {
                background: #3498db;
                border: 2px solid #2980b9;
                width: 16px;
                height: 16px;
                margin: -6px 0;
                border-radius: 8px;
            }
        )";
    }

signals:
    void paramsVisibilityChanged(bool visible);

private:
    QPushButton* m_toggleRenderParamsButton;
    QWidget* m_renderParamsContainer;
    QPropertyAnimation* m_renderParamsAnimation;
    bool m_renderParamsVisible;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    Stage1RenderParamsTest window;
    window.show();
    
    return app.exec();
}

#include "test_stage1_render_params.moc"
