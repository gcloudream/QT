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
 * @brief 渲染参数折叠功能测试程序
 * 
 * 这个程序演示了"阶段一演示"界面中渲染参数的折叠功能：
 * 1. 默认状态：渲染参数面板隐藏
 * 2. 点击按钮：显示/隐藏参数面板
 * 3. 平滑动画：展开/收起动画效果
 * 4. 状态保持：记住用户偏好
 */
class RenderParamsToggleTest : public QWidget
{
    Q_OBJECT

public:
    explicit RenderParamsToggleTest(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_toggleButton(nullptr)
        , m_paramsContainer(nullptr)
        , m_animation(nullptr)
        , m_paramsVisible(false)
    {
        setupUI();
        setWindowTitle("渲染参数折叠功能测试");
        setMinimumSize(400, 600);
    }

private slots:
    void toggleRenderParams()
    {
        if (m_paramsVisible) {
            // 隐藏参数面板
            m_animation->setStartValue(m_paramsContainer->height());
            m_animation->setEndValue(0);
            
            // 断开之前的连接，避免重复连接
            disconnect(m_animation, &QPropertyAnimation::finished, 
                       m_paramsContainer, &QWidget::hide);
            connect(m_animation, &QPropertyAnimation::finished, 
                    m_paramsContainer, &QWidget::hide, Qt::UniqueConnection);
            m_animation->start();
            
            m_toggleButton->setText("显示渲染参数");
            m_toggleButton->setChecked(false);
            m_paramsVisible = false;
            
            qDebug() << "Hiding render parameters panel";
        } else {
            // 显示参数面板
            m_paramsContainer->show();
            
            // 计算内容的理想高度
            int contentHeight = m_paramsContainer->sizeHint().height();
            if (contentHeight < 300) contentHeight = 300;  // 最小高度
            
            m_animation->setStartValue(0);
            m_animation->setEndValue(contentHeight);
            m_animation->start();
            
            m_toggleButton->setText("隐藏渲染参数");
            m_toggleButton->setChecked(true);
            m_paramsVisible = true;
            
            qDebug() << "Showing render parameters panel with height:" << contentHeight;
        }
        
        qDebug() << "Render parameters visibility changed to:" << m_paramsVisible;
    }

private:
    void setupUI()
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 16, 16, 16);
        mainLayout->setSpacing(16);
        
        // 标题
        QLabel* titleLabel = new QLabel("渲染参数折叠功能演示");
        titleLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 18px;"
            "   font-weight: 600;"
            "   color: #2c3e50;"
            "   padding: 12px;"
            "   background-color: #ecf0f1;"
            "   border-radius: 8px;"
            "   margin-bottom: 8px;"
            "}"
        );
        mainLayout->addWidget(titleLabel);
        
        // 说明文字
        QLabel* descLabel = new QLabel(
            "点击下面的按钮来显示/隐藏渲染参数面板。\n"
            "面板默认隐藏，点击后会以平滑动画展开。"
        );
        descLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 12px;"
            "   color: #7f8c8d;"
            "   padding: 8px;"
            "   background-color: #f8f9fa;"
            "   border-radius: 4px;"
            "}"
        );
        descLabel->setWordWrap(true);
        mainLayout->addWidget(descLabel);
        
        // 创建切换按钮
        createToggleButton();
        mainLayout->addWidget(m_toggleButton);
        
        // 创建参数容器
        createParamsContainer();
        mainLayout->addWidget(m_paramsContainer);
        
        // 添加弹性空间
        mainLayout->addStretch();
        
        // 底部状态信息
        QLabel* statusLabel = new QLabel("状态：参数面板已隐藏");
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
        connect(this, &RenderParamsToggleTest::paramsVisibilityChanged,
                [statusLabel](bool visible) {
                    statusLabel->setText(visible ? "状态：参数面板已显示" : "状态：参数面板已隐藏");
                });
    }
    
    void createToggleButton()
    {
        m_toggleButton = new QPushButton("显示渲染参数", this);
        m_toggleButton->setCheckable(true);
        m_toggleButton->setChecked(false);
        m_toggleButton->setStyleSheet(
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
        
        connect(m_toggleButton, &QPushButton::clicked, 
                this, &RenderParamsToggleTest::toggleRenderParams);
    }
    
    void createParamsContainer()
    {
        m_paramsContainer = new QWidget(this);
        m_paramsContainer->setStyleSheet(
            "QWidget {"
            "   background-color: #f8f9fa;"
            "   border: 2px solid #dee2e6;"
            "   border-radius: 8px;"
            "   padding: 4px;"
            "}"
        );
        
        QVBoxLayout* containerLayout = new QVBoxLayout(m_paramsContainer);
        containerLayout->setContentsMargins(12, 12, 12, 12);
        containerLayout->setSpacing(16);
        
        // 标题
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
        
        // LOD控制
        QGroupBox* lodGroup = new QGroupBox("LOD细节层次");
        lodGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* lodLayout = new QVBoxLayout(lodGroup);
        
        QComboBox* lodCombo = new QComboBox();
        lodCombo->addItems({"均匀采样", "体素网格", "随机采样"});
        lodCombo->setStyleSheet(getComboBoxStyle());
        lodLayout->addWidget(lodCombo);
        
        QSlider* lodSlider = new QSlider(Qt::Horizontal);
        lodSlider->setRange(0, 3);
        lodSlider->setStyleSheet(getSliderStyle());
        lodLayout->addWidget(lodSlider);
        
        containerLayout->addWidget(lodGroup);
        
        // 颜色映射控制
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
        
        // 渲染设置
        QGroupBox* renderGroup = new QGroupBox("渲染设置");
        renderGroup->setStyleSheet(getGroupBoxStyle());
        QVBoxLayout* renderLayout = new QVBoxLayout(renderGroup);
        
        QComboBox* modeCombo = new QComboBox();
        modeCombo->addItems({"点云", "密度图", "等高线"});
        modeCombo->setStyleSheet(getComboBoxStyle());
        renderLayout->addWidget(modeCombo);
        
        containerLayout->addWidget(renderGroup);
        
        // 创建动画
        m_animation = new QPropertyAnimation(m_paramsContainer, "maximumHeight", this);
        m_animation->setDuration(300);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
        
        // 默认隐藏
        m_paramsContainer->setMaximumHeight(0);
        m_paramsContainer->hide();
        m_paramsVisible = false;
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
    QPushButton* m_toggleButton;
    QWidget* m_paramsContainer;
    QPropertyAnimation* m_animation;
    bool m_paramsVisible;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    RenderParamsToggleTest window;
    window.show();
    
    return app.exec();
}

#include "test_render_params_toggle.moc"
