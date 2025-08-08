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
 * @brief 渲染参数折叠功能切换问题修复测试
 * 
 * 专门测试修复后的切换功能：
 * 1. 第一次点击能正常显示
 * 2. 第二次点击能正常隐藏
 * 3. 多次切换都能正常工作
 * 4. 状态同步正确
 */
class ToggleFixTest : public QWidget
{
    Q_OBJECT

public:
    explicit ToggleFixTest(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_toggleButton(nullptr)
        , m_paramsContainer(nullptr)
        , m_animation(nullptr)
        , m_paramsVisible(false)
        , m_toggleCount(0)
    {
        setupUI();
        setWindowTitle("渲染参数切换问题修复测试");
        setMinimumSize(500, 600);
        resize(550, 700);
    }

private slots:
    void toggleRenderParams()
    {
        m_toggleCount++;
        
        // 防止动画进行中的重复点击
        if (m_animation->state() == QAbstractAnimation::Running) {
            qDebug() << "Animation is running, ignoring click";
            updateStatus("动画进行中，忽略点击");
            return;
        }

        qDebug() << "=== Toggle" << m_toggleCount << ": toggleRenderParams called ===";
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
            
            // 立即更新状态变量，确保状态一致性
            m_paramsVisible = false;

            // 同步按钮状态
            syncButtonState();

            // 设置动画参数
            int currentHeight = m_paramsContainer->height();
            qDebug() << "Current height for hide animation:" << currentHeight;
            
            // 确保有有效的起始高度
            if (currentHeight <= 0) {
                currentHeight = calculateHeight();
                qDebug() << "Using calculated height as start value:" << currentHeight;
            }
            
            m_animation->setStartValue(currentHeight);
            m_animation->setEndValue(0);

            // 连接动画完成回调
            connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
                qDebug() << "Hide animation finished";
                m_paramsContainer->hide();
                m_paramsContainer->setMaximumHeight(0);
                updateStatus("面板已隐藏");
                qDebug() << "Panel hidden completely";
            }, Qt::UniqueConnection);

            m_animation->start();

        } else {
            // 显示参数面板
            qDebug() << "Starting show animation...";
            updateStatus("开始显示动画...");
            
            // 立即更新状态变量，确保状态一致性
            m_paramsVisible = true;

            // 同步按钮状态
            syncButtonState();

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
                updateStatus("面板已显示");
                qDebug() << "Panel shown completely";
            }, Qt::UniqueConnection);

            m_animation->start();
        }

        qDebug() << "Animation started, duration:" << m_animation->duration() << "ms";
        qDebug() << "New state - m_paramsVisible:" << m_paramsVisible;
        qDebug() << "=== Toggle" << m_toggleCount << ": toggleRenderParams end ===";
        
        // 更新计数器
        m_countLabel->setText(QString("切换次数: %1").arg(m_toggleCount));
    }
    
    void resetTest()
    {
        m_toggleCount = 0;
        if (m_paramsVisible) {
            toggleRenderParams();
        }
        m_countLabel->setText("切换次数: 0");
        updateStatus("测试重置");
    }

private:
    void setupUI()
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 16, 16, 16);
        mainLayout->setSpacing(16);
        
        // 标题
        QLabel* titleLabel = new QLabel("渲染参数切换问题修复测试");
        titleLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 18px;"
            "   font-weight: 600;"
            "   color: #2c3e50;"
            "   padding: 12px;"
            "   background-color: #e8f8f5;"
            "   border: 2px solid #1abc9c;"
            "   border-radius: 8px;"
            "   margin-bottom: 8px;"
            "}"
        );
        mainLayout->addWidget(titleLabel);
        
        // 问题描述
        QLabel* problemLabel = new QLabel(
            "修复的问题：点击渲染参数按钮后不能折叠隐藏\n\n"
            "修复方案：\n"
            "1. 状态变量在动画开始前立即更新\n"
            "2. 改进按钮状态同步逻辑\n"
            "3. 添加状态同步辅助方法\n"
            "4. 改进按钮点击事件处理\n\n"
            "测试方法：多次点击按钮，验证能否正常显示/隐藏切换"
        );
        problemLabel->setStyleSheet(
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
        problemLabel->setWordWrap(true);
        mainLayout->addWidget(problemLabel);
        
        // 控制按钮
        QHBoxLayout* controlLayout = new QHBoxLayout();
        
        QPushButton* resetBtn = new QPushButton("重置测试");
        resetBtn->setStyleSheet(getButtonStyle("#6c757d"));
        connect(resetBtn, &QPushButton::clicked, this, &ToggleFixTest::resetTest);
        controlLayout->addWidget(resetBtn);
        
        controlLayout->addStretch();
        mainLayout->addLayout(controlLayout);
        
        // 切换计数器
        m_countLabel = new QLabel("切换次数: 0");
        m_countLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   color: #495057;"
            "   padding: 8px;"
            "   background-color: #e9ecef;"
            "   border-radius: 4px;"
            "}"
        );
        mainLayout->addWidget(m_countLabel);
        
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
        m_toggleButton = new QPushButton("渲染参数", this);
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
        
        // 使用修复后的连接方式
        connect(m_toggleButton, &QPushButton::clicked,
                this, [this](bool checked) {
                    qDebug() << "Button clicked, checked state:" << checked;
                    qDebug() << "Current m_paramsVisible:" << m_paramsVisible;
                    
                    // 重置按钮状态为当前逻辑状态，避免状态不一致
                    m_toggleButton->setChecked(m_paramsVisible);
                    
                    // 调用切换方法
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
        for (int i = 0; i < 2; i++) {
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
    
    void syncButtonState()
    {
        qDebug() << "Syncing button state - m_paramsVisible:" << m_paramsVisible;
        
        if (m_paramsVisible) {
            m_toggleButton->setText("隐藏参数");
            m_toggleButton->setChecked(true);
        } else {
            m_toggleButton->setText("渲染参数");
            m_toggleButton->setChecked(false);
        }
        
        qDebug() << "Button state synced - text:" << m_toggleButton->text()
                 << "checked:" << m_toggleButton->isChecked();
    }
    
    int calculateHeight()
    {
        int height = m_paramsContainer->sizeHint().height();
        return qMax(height, 200);
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
    int m_toggleCount;
    
    QLabel* m_statusLabel;
    QLabel* m_countLabel;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ToggleFixTest window;
    window.show();
    
    return app.exec();
}

#include "test_toggle_fix.moc"
