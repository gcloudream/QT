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

/**
 * @brief 渲染参数折叠功能Bug修复验证程序
 * 
 * 这个程序专门测试修复后的渲染参数折叠功能：
 * 1. 测试多次连续点击的稳定性
 * 2. 验证状态同步的正确性
 * 3. 检查动画完成回调的正确性
 * 4. 测试按钮状态与面板可见性的同步
 */
class RenderParamsBugfixTest : public QWidget
{
    Q_OBJECT

public:
    explicit RenderParamsBugfixTest(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_toggleButton(nullptr)
        , m_paramsContainer(nullptr)
        , m_animation(nullptr)
        , m_paramsVisible(false)
        , m_clickCount(0)
    {
        setupUI();
        setWindowTitle("渲染参数折叠功能 - Bug修复验证");
        setMinimumSize(500, 700);
        resize(550, 800);
        
        // 创建自动测试定时器
        m_autoTestTimer = new QTimer(this);
        connect(m_autoTestTimer, &QTimer::timeout, this, &RenderParamsBugfixTest::autoTest);
    }

private slots:
    void toggleRenderParams()
    {
        m_clickCount++;
        
        // 防止动画进行中的重复点击
        if (m_animation->state() == QAbstractAnimation::Running) {
            qDebug() << "Click" << m_clickCount << ": Animation is running, ignoring click";
            updateStatus("动画进行中，忽略点击");
            return;
        }

        qDebug() << "=== Click" << m_clickCount << ": toggleRenderParams called ===";
        qDebug() << "Current state - m_paramsVisible:" << m_paramsVisible;
        qDebug() << "Button checked state:" << m_toggleButton->isChecked();
        qDebug() << "Container visible:" << m_paramsContainer->isVisible();
        qDebug() << "Container height:" << m_paramsContainer->height();
        qDebug() << "Container maximumHeight:" << m_paramsContainer->maximumHeight();

        // 断开所有之前的finished信号连接，避免干扰
        disconnect(m_animation, &QPropertyAnimation::finished, nullptr, nullptr);

        if (m_paramsVisible) {
            // 隐藏参数面板
            qDebug() << "Starting hide animation...";
            updateStatus("开始隐藏动画...");
            
            // 立即更新按钮状态，提供即时反馈
            m_toggleButton->setText("显示渲染参数");
            m_toggleButton->setChecked(false);
            
            // 设置动画参数
            int currentHeight = m_paramsContainer->height();
            m_animation->setStartValue(currentHeight);
            m_animation->setEndValue(0);
            
            // 连接动画完成回调
            connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
                qDebug() << "Hide animation finished";
                m_paramsContainer->hide();
                m_paramsContainer->setMaximumHeight(0);
                m_paramsVisible = false;
                updateStatus("面板已隐藏");
                qDebug() << "Panel hidden, state updated to:" << m_paramsVisible;
            }, Qt::UniqueConnection);
            
            m_animation->start();
            
        } else {
            // 显示参数面板
            qDebug() << "Starting show animation...";
            updateStatus("开始显示动画...");
            
            // 立即更新按钮状态，提供即时反馈
            m_toggleButton->setText("隐藏渲染参数");
            m_toggleButton->setChecked(true);
            
            // 先显示容器
            m_paramsContainer->show();
            
            // 计算内容的理想高度
            int contentHeight = calculateHeight();
            qDebug() << "Calculated content height:" << contentHeight;
            
            // 设置最大高度以允许展开
            m_paramsContainer->setMaximumHeight(contentHeight);
            
            // 设置动画参数
            m_animation->setStartValue(0);
            m_animation->setEndValue(contentHeight);
            
            // 连接动画完成回调
            connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
                qDebug() << "Show animation finished";
                m_paramsContainer->setMaximumHeight(QWIDGETSIZE_MAX); // 允许自由调整大小
                m_paramsVisible = true;
                updateStatus("面板已显示");
                qDebug() << "Panel shown, state updated to:" << m_paramsVisible;
            }, Qt::UniqueConnection);
            
            m_animation->start();
        }

        qDebug() << "Animation started, duration:" << m_animation->duration() << "ms";
        qDebug() << "=== Click" << m_clickCount << ": toggleRenderParams end ===";
    }
    
    void startAutoTest()
    {
        m_autoTestTimer->start(1000); // 每秒点击一次
        updateStatus("自动测试开始...");
    }
    
    void stopAutoTest()
    {
        m_autoTestTimer->stop();
        updateStatus("自动测试停止");
    }
    
    void autoTest()
    {
        static int autoClickCount = 0;
        autoClickCount++;
        
        if (autoClickCount > 10) {
            stopAutoTest();
            updateStatus("自动测试完成 - 10次点击");
            return;
        }
        
        qDebug() << "Auto test click" << autoClickCount;
        toggleRenderParams();
    }
    
    void resetTest()
    {
        m_clickCount = 0;
        if (m_paramsVisible) {
            toggleRenderParams();
        }
        updateStatus("测试重置");
    }

private:
    void setupUI()
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 16, 16, 16);
        mainLayout->setSpacing(16);
        
        // 标题
        QLabel* titleLabel = new QLabel("渲染参数折叠功能 - Bug修复验证");
        titleLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 18px;"
            "   font-weight: 600;"
            "   color: #2c3e50;"
            "   padding: 12px;"
            "   background-color: #e8f5e8;"
            "   border: 2px solid #27ae60;"
            "   border-radius: 8px;"
            "   margin-bottom: 8px;"
            "}"
        );
        mainLayout->addWidget(titleLabel);
        
        // 测试说明
        QLabel* descLabel = new QLabel(
            "修复的问题：\n"
            "1. 状态同步问题 - 现在状态在动画完成后更新\n"
            "2. 多次点击问题 - 添加了动画状态检查\n"
            "3. 回调连接问题 - 每次都断开重连，避免重复\n"
            "4. 高度计算问题 - 改进了高度计算逻辑\n\n"
            "测试方法：多次快速点击按钮，观察是否稳定切换"
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
        
        // 测试控制按钮
        QHBoxLayout* controlLayout = new QHBoxLayout();
        
        QPushButton* autoTestBtn = new QPushButton("开始自动测试");
        autoTestBtn->setStyleSheet(getButtonStyle("#28a745"));
        connect(autoTestBtn, &QPushButton::clicked, this, &RenderParamsBugfixTest::startAutoTest);
        controlLayout->addWidget(autoTestBtn);
        
        QPushButton* stopTestBtn = new QPushButton("停止测试");
        stopTestBtn->setStyleSheet(getButtonStyle("#dc3545"));
        connect(stopTestBtn, &QPushButton::clicked, this, &RenderParamsBugfixTest::stopAutoTest);
        controlLayout->addWidget(stopTestBtn);
        
        QPushButton* resetBtn = new QPushButton("重置");
        resetBtn->setStyleSheet(getButtonStyle("#6c757d"));
        connect(resetBtn, &QPushButton::clicked, this, &RenderParamsBugfixTest::resetTest);
        controlLayout->addWidget(resetBtn);
        
        mainLayout->addLayout(controlLayout);
        
        // 点击计数器
        m_clickCountLabel = new QLabel("点击次数: 0");
        m_clickCountLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   color: #495057;"
            "   padding: 8px;"
            "   background-color: #e9ecef;"
            "   border-radius: 4px;"
            "}"
        );
        mainLayout->addWidget(m_clickCountLabel);
        
        // 创建切换按钮
        createToggleButton();
        mainLayout->addWidget(m_toggleButton);
        
        // 创建参数容器
        createParamsContainer();
        mainLayout->addWidget(m_paramsContainer);
        
        // 状态显示
        m_statusLabel = new QLabel("状态：参数面板已隐藏（默认状态）");
        m_statusLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 12px;"
            "   color: #6c757d;"
            "   padding: 8px;"
            "   background-color: #f8f9fa;"
            "   border: 1px solid #dee2e6;"
            "   border-radius: 4px;"
            "}"
        );
        mainLayout->addWidget(m_statusLabel);
        
        // 底部弹性空间
        mainLayout->addStretch();
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
        
        connect(m_toggleButton, &QPushButton::clicked, this, [this]() {
            m_clickCountLabel->setText(QString("点击次数: %1").arg(m_clickCount + 1));
            toggleRenderParams();
        });
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
        
        // 添加一些测试内容
        for (int i = 0; i < 3; i++) {
            QGroupBox* group = new QGroupBox(QString("参数组 %1").arg(i + 1));
            group->setStyleSheet(getGroupBoxStyle());
            QVBoxLayout* groupLayout = new QVBoxLayout(group);
            
            QComboBox* combo = new QComboBox();
            combo->addItems({"选项1", "选项2", "选项3"});
            combo->setStyleSheet(getComboBoxStyle());
            groupLayout->addWidget(combo);
            
            QSlider* slider = new QSlider(Qt::Horizontal);
            slider->setRange(0, 100);
            slider->setStyleSheet(getSliderStyle());
            groupLayout->addWidget(slider);
            
            containerLayout->addWidget(group);
        }
        
        // 创建动画
        m_animation = new QPropertyAnimation(m_paramsContainer, "maximumHeight", this);
        m_animation->setDuration(300);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
        
        // 默认隐藏
        m_paramsContainer->setMaximumHeight(0);
        m_paramsContainer->hide();
        m_paramsVisible = false;
    }
    
    int calculateHeight()
    {
        int height = m_paramsContainer->sizeHint().height();
        return qMax(height, 300);
    }
    
    void updateStatus(const QString& status)
    {
        m_statusLabel->setText(QString("状态：%1").arg(status));
    }
    
    QString getButtonStyle(const QString& color) const
    {
        return QString(
            "QPushButton {"
            "   padding: 8px 16px;"
            "   font-size: 12px;"
            "   font-weight: 500;"
            "   background-color: %1;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "   opacity: 0.8;"
            "}"
        ).arg(color);
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

private:
    QPushButton* m_toggleButton;
    QWidget* m_paramsContainer;
    QPropertyAnimation* m_animation;
    bool m_paramsVisible;
    int m_clickCount;
    
    QTimer* m_autoTestTimer;
    QLabel* m_statusLabel;
    QLabel* m_clickCountLabel;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    RenderParamsBugfixTest window;
    window.show();
    
    return app.exec();
}

#include "test_render_params_bugfix.moc"
