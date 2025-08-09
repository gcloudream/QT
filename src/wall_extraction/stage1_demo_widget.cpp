#include "stage1_demo_widget.h"
#include "wall_extraction_manager.h"
#include "top_down_view_renderer.h"
#include "view_projection_manager.h"
#include "color_mapping_manager.h"
#include "point_cloud_lod_manager.h"
#include "point_cloud_memory_manager.h"
#include "spatial_index.h"
#include "line_drawing_toolbar.h"
#include "las_reader.h"
#include "point_cloud_processor.h"
#include "../../pcdreader.h"
#include <QDebug>
#include <QMessageBox>
#include <QApplication>
#include <QPixmap>
#include <QtMath>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QAbstractAnimation>
#include <cmath>

Stage1DemoWidget::Stage1DemoWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_lineDrawingToolbar(nullptr)
    , m_toggleRenderParamsButton(nullptr)
    , m_renderParamsContainer(nullptr)
    , m_renderParamsAnimation(nullptr)
    , m_renderParamsVisible(false)
    , m_toggleLineDrawingButton(nullptr)
    , m_lineDrawingContainer(nullptr)
    , m_lineDrawingAnimation(nullptr)
    , m_lineDrawingVisible(false)
    , m_isClearing(false)
    , m_updateTimer(new QTimer(this))
    , m_performanceTimer(new QElapsedTimer())
{
    // 创建核心组件
    m_wallManager = std::make_unique<WallExtraction::WallExtractionManager>(this);
    m_renderer = std::make_unique<WallExtraction::TopDownViewRenderer>(this);
    m_colorMapper = std::make_unique<WallExtraction::ColorMappingManager>(this);
    m_lodManager = std::make_unique<WallExtraction::PointCloudLODManager>(this);
    m_memoryManager = std::make_unique<WallExtraction::PointCloudMemoryManager>(this);
    m_spatialIndex = std::make_unique<WallExtraction::SpatialIndex>(this);

    // 初始化WallExtractionManager（这是关键步骤！）
    if (!m_wallManager->initialize()) {
        qCritical() << "Failed to initialize WallExtractionManager";
        // 继续执行，但阶段二功能将不可用
    } else {
        qDebug() << "WallExtractionManager initialized successfully";
    }

    // 设置UI
    setupUI();

    // 连接信号
    connectSignals();

    // 应用初始响应式样式
    updateResponsiveStyles();

    // 设置响应式属性
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(800, 600);

    // 定时器连接已删除
    connect(m_renderer.get(), &WallExtraction::TopDownViewRenderer::renderingCompleted,
            this, [this](qint64 time) {
                m_stats.lastRenderTime = time;
                m_stats.fps = 1000.0 / qMax(time, qint64(1));
                // 统计更新已删除
            });

    // 启动定时器
    m_updateTimer->start(1000); // 每秒更新一次统计信息

    qDebug() << "Stage1DemoWidget created successfully with responsive design";
}

Stage1DemoWidget::~Stage1DemoWidget()
{
    qDebug() << "Stage1DemoWidget destroyed";
}

// ==================== 渲染参数折叠功能实现 ====================

void Stage1DemoWidget::createRenderParamsToggleButton()
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

    // 连接信号 - 使用clicked信号，并在槽函数中处理状态
    connect(m_toggleRenderParamsButton, &QPushButton::clicked,
            this, [this](bool checked) {
                qDebug() << "Button clicked, checked state:" << checked;
                qDebug() << "Current m_renderParamsVisible:" << m_renderParamsVisible;

                // 重置按钮状态为当前逻辑状态，避免状态不一致
                m_toggleRenderParamsButton->setChecked(m_renderParamsVisible);

                // 调用切换方法
                toggleRenderParams();
            });

    qDebug() << "Render params toggle button created with initial state:"
             << m_toggleRenderParamsButton->isChecked();
}

void Stage1DemoWidget::createRenderParamsContainer()
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

    // LOD控制区域
    createCompactLODControl();
    containerLayout->addWidget(m_compactLODWidget);

    // 颜色映射控制区域
    createCompactColorControl();
    containerLayout->addWidget(m_compactColorWidget);

    // 渲染模式控制区域
    createCompactRenderControl();
    containerLayout->addWidget(m_compactRenderWidget);

    // 创建动画
    m_renderParamsAnimation = new QPropertyAnimation(m_renderParamsContainer, "maximumHeight", this);
    m_renderParamsAnimation->setDuration(300);  // 300ms动画
    m_renderParamsAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // 默认隐藏
    m_renderParamsContainer->setMaximumHeight(0);
    m_renderParamsContainer->hide();
    m_renderParamsVisible = false;
}

void Stage1DemoWidget::toggleRenderParams()
{
    // 防止动画进行中的重复点击
    if (m_renderParamsAnimation->state() == QAbstractAnimation::Running) {
        qDebug() << "Animation is running, ignoring click";
        return;
    }

    qDebug() << "=== toggleRenderParams called ===";
    qDebug() << "Current state - m_renderParamsVisible:" << m_renderParamsVisible;
    qDebug() << "Button checked state:" << m_toggleRenderParamsButton->isChecked();
    qDebug() << "Container visible:" << m_renderParamsContainer->isVisible();
    qDebug() << "Container height:" << m_renderParamsContainer->height();
    qDebug() << "Container maximumHeight:" << m_renderParamsContainer->maximumHeight();

    // 断开所有之前的finished信号连接，避免干扰
    disconnect(m_renderParamsAnimation, &QPropertyAnimation::finished, nullptr, nullptr);

    if (m_renderParamsVisible) {
        // 隐藏参数面板
        qDebug() << "Starting hide animation...";

        // 立即更新状态变量，确保状态一致性
        m_renderParamsVisible = false;

        // 同步按钮状态
        syncRenderParamsButtonState();

        // 设置动画参数
        int currentHeight = m_renderParamsContainer->height();
        qDebug() << "Current height for hide animation:" << currentHeight;

        // 确保有有效的起始高度
        if (currentHeight <= 0) {
            currentHeight = calculateRenderParamsHeight();
            qDebug() << "Using calculated height as start value:" << currentHeight;
        }

        m_renderParamsAnimation->setStartValue(currentHeight);
        m_renderParamsAnimation->setEndValue(0);

        // 连接动画完成回调
        connect(m_renderParamsAnimation, &QPropertyAnimation::finished, this, [this]() {
            qDebug() << "Hide animation finished";
            m_renderParamsContainer->hide();
            m_renderParamsContainer->setMaximumHeight(0);
            qDebug() << "Panel hidden completely";
        }, Qt::UniqueConnection);

        m_renderParamsAnimation->start();

    } else {
        // 显示参数面板
        qDebug() << "Starting show animation...";

        // 立即更新状态变量，确保状态一致性
        m_renderParamsVisible = true;

        // 同步按钮状态
        syncRenderParamsButtonState();

        // 先显示容器
        m_renderParamsContainer->show();

        // 计算内容的理想高度
        int contentHeight = calculateRenderParamsHeight();
        qDebug() << "Calculated content height:" << contentHeight;

        // 设置最大高度以允许展开
        m_renderParamsContainer->setMaximumHeight(contentHeight);

        // 设置动画参数
        m_renderParamsAnimation->setStartValue(0);
        m_renderParamsAnimation->setEndValue(contentHeight);

        // 连接动画完成回调
        connect(m_renderParamsAnimation, &QPropertyAnimation::finished, this, [this]() {
            qDebug() << "Show animation finished";
            m_renderParamsContainer->setMaximumHeight(QWIDGETSIZE_MAX); // 允许自由调整大小
            qDebug() << "Panel shown completely";
        }, Qt::UniqueConnection);

        m_renderParamsAnimation->start();
    }

    qDebug() << "Animation started, duration:" << m_renderParamsAnimation->duration() << "ms";
    qDebug() << "New state - m_renderParamsVisible:" << m_renderParamsVisible;
    qDebug() << "=== toggleRenderParams end ===";
}

int Stage1DemoWidget::calculateRenderParamsHeight()
{
    int contentHeight = 0;

    // 计算各个子组件的高度
    if (m_compactLODWidget) {
        int lodHeight = m_compactLODWidget->sizeHint().height();
        contentHeight += lodHeight;
        qDebug() << "LOD widget height:" << lodHeight;
    }

    if (m_compactColorWidget) {
        int colorHeight = m_compactColorWidget->sizeHint().height();
        contentHeight += colorHeight;
        qDebug() << "Color widget height:" << colorHeight;
    }

    if (m_compactRenderWidget) {
        int renderHeight = m_compactRenderWidget->sizeHint().height();
        contentHeight += renderHeight;
        qDebug() << "Render widget height:" << renderHeight;
    }

    // 添加标题、边距和间距
    contentHeight += 60;  // 标题高度
    contentHeight += 16;  // 容器内边距 (8*2)
    contentHeight += 36;  // 组件间距 (12*3)

    // 确保最小高度
    contentHeight = qMax(contentHeight, 200);

    qDebug() << "Total calculated height:" << contentHeight;
    return contentHeight;
}

void Stage1DemoWidget::syncRenderParamsButtonState()
{
    qDebug() << "Syncing button state - m_renderParamsVisible:" << m_renderParamsVisible;

    if (m_renderParamsVisible) {
        m_toggleRenderParamsButton->setText("隐藏参数");
        m_toggleRenderParamsButton->setChecked(true);
    } else {
        m_toggleRenderParamsButton->setText("渲染参数");
        m_toggleRenderParamsButton->setChecked(false);
    }

    qDebug() << "Button state synced - text:" << m_toggleRenderParamsButton->text()
             << "checked:" << m_toggleRenderParamsButton->isChecked();
}

// ==================== 线框绘制工具折叠功能实现 ====================

void Stage1DemoWidget::createLineDrawingToggleButton()
{
    m_toggleLineDrawingButton = new QPushButton("线框绘制工具", this);
    m_toggleLineDrawingButton->setCheckable(true);
    m_toggleLineDrawingButton->setChecked(false);  // 默认隐藏
    m_toggleLineDrawingButton->setStyleSheet(
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

    // 连接信号 - 使用与渲染参数相同的处理方式
    connect(m_toggleLineDrawingButton, &QPushButton::clicked,
            this, [this](bool checked) {
                qDebug() << "Line drawing button clicked, checked state:" << checked;
                qDebug() << "Current m_lineDrawingVisible:" << m_lineDrawingVisible;

                // 重置按钮状态为当前逻辑状态，避免状态不一致
                m_toggleLineDrawingButton->setChecked(m_lineDrawingVisible);

                // 调用切换方法
                toggleLineDrawingTools();
            });

    qDebug() << "Line drawing toggle button created with initial state:"
             << m_toggleLineDrawingButton->isChecked();
}

void Stage1DemoWidget::createLineDrawingContainer()
{
    m_lineDrawingContainer = new QWidget(this);
    m_lineDrawingContainer->setStyleSheet(
        "QWidget {"
        "   background-color: #f8f9fa;"
        "   border: 2px solid #dee2e6;"
        "   border-radius: 8px;"
        "   padding: 4px;"
        "}"
    );

    QVBoxLayout* containerLayout = new QVBoxLayout(m_lineDrawingContainer);
    containerLayout->setContentsMargins(8, 8, 8, 8);
    containerLayout->setSpacing(12);

    // 添加标题
    QLabel* titleLabel = new QLabel("线框绘制工具");
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

    // 注意：LineDrawingToolbar将在createLineDrawingControls调用后
    // 通过addLineDrawingControlsToContainer方法添加到容器中

    // 创建动画
    m_lineDrawingAnimation = new QPropertyAnimation(m_lineDrawingContainer, "maximumHeight", this);
    m_lineDrawingAnimation->setDuration(300);  // 300ms动画，与渲染参数一致
    m_lineDrawingAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // 默认隐藏
    m_lineDrawingContainer->setMaximumHeight(0);
    m_lineDrawingContainer->hide();
    m_lineDrawingVisible = false;

    qDebug() << "Line drawing container created and hidden by default";
}

void Stage1DemoWidget::toggleLineDrawingTools()
{
    // 防止动画进行中的重复点击
    if (m_lineDrawingAnimation->state() == QAbstractAnimation::Running) {
        qDebug() << "Line drawing animation is running, ignoring click";
        return;
    }

    qDebug() << "=== toggleLineDrawingTools called ===";
    qDebug() << "Current state - m_lineDrawingVisible:" << m_lineDrawingVisible;
    qDebug() << "Button checked state:" << m_toggleLineDrawingButton->isChecked();
    qDebug() << "Container visible:" << m_lineDrawingContainer->isVisible();
    qDebug() << "Container height:" << m_lineDrawingContainer->height();
    qDebug() << "Container maximumHeight:" << m_lineDrawingContainer->maximumHeight();

    // 断开所有之前的finished信号连接，避免干扰
    disconnect(m_lineDrawingAnimation, &QPropertyAnimation::finished, nullptr, nullptr);

    if (m_lineDrawingVisible) {
        // 隐藏线框绘制工具面板
        qDebug() << "Starting hide line drawing tools animation...";

        // 立即更新状态变量，确保状态一致性
        m_lineDrawingVisible = false;

        // 同步按钮状态
        syncLineDrawingButtonState();

        // 设置动画参数
        int currentHeight = m_lineDrawingContainer->height();
        qDebug() << "Current height for hide animation:" << currentHeight;

        // 确保有有效的起始高度
        if (currentHeight <= 0) {
            currentHeight = calculateLineDrawingHeight();
            qDebug() << "Using calculated height as start value:" << currentHeight;
        }

        m_lineDrawingAnimation->setStartValue(currentHeight);
        m_lineDrawingAnimation->setEndValue(0);

        // 连接动画完成回调
        connect(m_lineDrawingAnimation, &QPropertyAnimation::finished, this, [this]() {
            qDebug() << "Hide line drawing tools animation finished";
            m_lineDrawingContainer->hide();
            m_lineDrawingContainer->setMaximumHeight(0);
            qDebug() << "Line drawing tools panel hidden completely";
        }, Qt::UniqueConnection);

        m_lineDrawingAnimation->start();

    } else {
        // 显示线框绘制工具面板
        qDebug() << "Starting show line drawing tools animation...";

        // 立即更新状态变量，确保状态一致性
        m_lineDrawingVisible = true;

        // 同步按钮状态
        syncLineDrawingButtonState();

        // 先显示容器
        m_lineDrawingContainer->show();

        // 计算内容的理想高度
        int contentHeight = calculateLineDrawingHeight();
        qDebug() << "Calculated line drawing content height:" << contentHeight;

        // 设置最大高度以允许展开
        m_lineDrawingContainer->setMaximumHeight(contentHeight);

        // 设置动画参数
        m_lineDrawingAnimation->setStartValue(0);
        m_lineDrawingAnimation->setEndValue(contentHeight);

        // 连接动画完成回调
        connect(m_lineDrawingAnimation, &QPropertyAnimation::finished, this, [this]() {
            qDebug() << "Show line drawing tools animation finished";
            m_lineDrawingContainer->setMaximumHeight(QWIDGETSIZE_MAX); // 允许自由调整大小
            qDebug() << "Line drawing tools panel shown completely";
        }, Qt::UniqueConnection);

        m_lineDrawingAnimation->start();
    }

    qDebug() << "Line drawing animation started, duration:" << m_lineDrawingAnimation->duration() << "ms";
    qDebug() << "New state - m_lineDrawingVisible:" << m_lineDrawingVisible;
    qDebug() << "=== toggleLineDrawingTools end ===";
}

int Stage1DemoWidget::calculateLineDrawingHeight()
{
    int contentHeight = 0;

    // 计算LineDrawingToolbar的高度（现在是水平布局组合，高度会减少）
    if (m_lineDrawingToolbar) {
        // 重新估算：移除标题 + 绘制模式组(28+边距=50) + 编辑模式组(28+边距=50) +
        //          工具组(28+边距=50) + 状态组(28+边距=50) + 间距
        int estimatedToolbarHeight = 50 + 50 + 50 + 50 + 20; // 约220px

        // 使用实际高度或估算高度的较大值
        int actualHeight = m_lineDrawingToolbar->sizeHint().height();
        int toolbarHeight = qMax(actualHeight, estimatedToolbarHeight);

        contentHeight += toolbarHeight;
        qDebug() << "LineDrawingToolbar height (actual/estimated):" << actualHeight << "/" << estimatedToolbarHeight << "-> using:" << toolbarHeight;
    }

    // LinePropertyPanel已移除，不再计算其高度

    // 添加容器标题、边距和间距（移除了工具栏标题）
    contentHeight += 40;  // 容器标题高度
    contentHeight += 16;  // 容器内边距 (8*2)
    contentHeight += 12;  // 组件间距

    // 由于改为水平布局，减少最小高度
    contentHeight = qMax(contentHeight, 280);

    qDebug() << "Total calculated line drawing height:" << contentHeight;
    return contentHeight;
}

void Stage1DemoWidget::syncLineDrawingButtonState()
{
    qDebug() << "Syncing line drawing button state - m_lineDrawingVisible:" << m_lineDrawingVisible;

    if (m_lineDrawingVisible) {
        m_toggleLineDrawingButton->setText("隐藏工具");
        m_toggleLineDrawingButton->setChecked(true);
    } else {
        m_toggleLineDrawingButton->setText("线框绘制工具");
        m_toggleLineDrawingButton->setChecked(false);
    }

    qDebug() << "Line drawing button state synced - text:" << m_toggleLineDrawingButton->text()
             << "checked:" << m_toggleLineDrawingButton->isChecked();
}

void Stage1DemoWidget::addLineDrawingControlsToContainer()
{
    if (!m_lineDrawingContainer) {
        qWarning() << "Line drawing container not created yet";
        return;
    }

    QVBoxLayout* containerLayout = qobject_cast<QVBoxLayout*>(m_lineDrawingContainer->layout());
    if (!containerLayout) {
        qWarning() << "Line drawing container layout not found";
        return;
    }

    // 将LineDrawingToolbar添加到容器中
    if (m_lineDrawingToolbar) {
        containerLayout->addWidget(m_lineDrawingToolbar);
        qDebug() << "LineDrawingToolbar added to container";
    }
}

void Stage1DemoWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    qDebug() << "=== Stage1DemoWidget::resizeEvent ===";
    qDebug() << "Old size:" << event->oldSize();
    qDebug() << "New size:" << event->size();
    qDebug() << "Parent size:" << (parentWidget() ? parentWidget()->size() : QSize(-1, -1));

    // 根据窗口大小动态调整控制面板宽度
    if (m_controlPanel) {
        int windowWidth = event->size().width();
        int panelWidth;

        if (windowWidth < 1200) {
            // 小窗口：控制面板占35%，最小320px
            panelWidth = qMax(320, (windowWidth * 35) / 100);
        } else if (windowWidth < 1600) {
            // 中等窗口：控制面板占35%，最小380px
            panelWidth = qMax(380, (windowWidth * 35) / 100);
        } else {
            // 大窗口：控制面板占35%，最大500px
            panelWidth = qMin(500, (windowWidth * 35) / 100);
        }

        // 计算实际占比
        double actualRatio = (double)panelWidth / windowWidth * 100.0;
        double displayAreaWidth = windowWidth - panelWidth - 8; // 减去间距
        double displayRatio = displayAreaWidth / windowWidth * 100.0;

        qDebug() << "=== Control Panel Width Adjustment ===";
        qDebug() << "Window width:" << windowWidth << "px";
        qDebug() << "Control panel width:" << panelWidth << "px (" << QString::number(actualRatio, 'f', 1) << "%)";
        qDebug() << "Display area width:" << displayAreaWidth << "px (" << QString::number(displayRatio, 'f', 1) << "%)";
        qDebug() << "Previous panel width:" << m_controlPanel->width() << "px";

        m_controlPanel->setMaximumWidth(panelWidth);
        m_controlPanel->setMinimumWidth(qMin(320, panelWidth));

        // 强制布局更新
        m_controlPanel->updateGeometry();
        if (m_contentLayout) {
            m_contentLayout->invalidate();
            m_contentLayout->activate();
        }
    }

    // 通知渲染器更新viewport
    if (m_renderer && m_renderDisplayLabel) {
        QSize displaySize = m_renderDisplayLabel->size();
        if (displaySize.isValid() && displaySize.width() > 0 && displaySize.height() > 0) {
            qDebug() << "Updating renderer viewport to:" << displaySize;
            m_renderer->setViewportSize(displaySize);
        }
    }

    // 更新响应式样式
    updateResponsiveStyles();

    // 强制整体布局更新
    if (m_mainLayout) {
        m_mainLayout->invalidate();
        m_mainLayout->activate();
    }

    // 确保所有子控件都更新
    update();
}

void Stage1DemoWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    qDebug() << "Stage1DemoWidget::showEvent called - Widget size:" << size();

    // 初始化响应式布局
    QTimer::singleShot(100, this, [this]() {
        // 延迟执行，确保布局已经完成
        qDebug() << "=== Initial layout setup ===";
        if (m_controlPanel) {
            qDebug() << "Initial control panel size:" << m_controlPanel->size();
            qDebug() << "Control panel max width:" << m_controlPanel->maximumWidth();
            qDebug() << "Control panel min width:" << m_controlPanel->minimumWidth();
        }
        if (m_renderDisplayLabel) {
            qDebug() << "Initial display area size:" << m_renderDisplayLabel->size();
        }

        // 计算初始宽度占比
        if (m_controlPanel && width() > 0) {
            double panelRatio = (double)m_controlPanel->width() / width() * 100.0;
            qDebug() << "Initial control panel width ratio:" << QString::number(panelRatio, 'f', 1) << "%";
        }

        // 验证字体修复效果
        qDebug() << "=== Font Display Fix Verification ===";
        if (m_colorSchemeCombo) {
            qDebug() << "Color scheme combo font:" << m_colorSchemeCombo->font();
        }
        if (m_minValueSpin && m_maxValueSpin) {
            qDebug() << "Value range spinboxes font:" << m_minValueSpin->font();
        }
        if (m_pointSizeSpin) {
            qDebug() << "Point size spinbox font:" << m_pointSizeSpin->font();
        }
        qDebug() << "Font display verification completed";

        // 验证输入框尺寸优化效果
        qDebug() << "=== Input Box Size Optimization Verification ===";
        if (m_minValueSpin) {
            qDebug() << "Min value spinbox size:" << m_minValueSpin->size();
            qDebug() << "Min value spinbox min/max width:" << m_minValueSpin->minimumWidth() << "/" << m_minValueSpin->maximumWidth();
        }
        if (m_maxValueSpin) {
            qDebug() << "Max value spinbox size:" << m_maxValueSpin->size();
            qDebug() << "Max value spinbox min/max width:" << m_maxValueSpin->minimumWidth() << "/" << m_maxValueSpin->maximumWidth();
        }
        if (m_pointSizeSpin) {
            qDebug() << "Point size spinbox size:" << m_pointSizeSpin->size();
            qDebug() << "Point size spinbox min/max width:" << m_pointSizeSpin->minimumWidth() << "/" << m_pointSizeSpin->maximumWidth();
        }
        if (m_renderModeCombo) {
            qDebug() << "Render mode combo size:" << m_renderModeCombo->size();
        }
        qDebug() << "Input box size optimization verification completed";

        // 强制调整到父容器大小
        if (parentWidget()) {
            QSize parentSize = parentWidget()->size();
            qDebug() << "Parent container size:" << parentSize;
            if (parentSize.isValid() && parentSize.width() > 0 && parentSize.height() > 0) {
                resize(parentSize);
                qDebug() << "Resized to parent size:" << parentSize;
            }
        }

        // 强制触发一次完整的布局更新
        forceLayoutUpdate();
    });
}

// ==================== 鼠标事件处理（用于线段绘制） ====================

void Stage1DemoWidget::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "=== Mouse Press Event ===";
    qDebug() << "Event position:" << event->pos();
    qDebug() << "Event button:" << event->button();

    // 检查渲染显示标签是否存在
    if (!m_renderDisplayLabel) {
        qDebug() << "ERROR: m_renderDisplayLabel is null";
        QWidget::mousePressEvent(event);
        return;
    }

    qDebug() << "Render display label geometry:" << m_renderDisplayLabel->geometry();
    qDebug() << "Event in render area:" << m_renderDisplayLabel->geometry().contains(event->pos());

    // 检查鼠标事件是否在渲染显示区域内
    if (m_renderDisplayLabel->geometry().contains(event->pos())) {
        qDebug() << "Mouse event is in render display area";

        // 将窗口坐标转换为label内部坐标
        QPoint labelLocal = event->pos() - m_renderDisplayLabel->geometry().topLeft();
        qDebug() << "Label local position:" << labelLocal;

        QPointF viewportPos;
        if (!mapLabelPosToViewport(labelLocal, viewportPos)) {
            qDebug() << "Click is outside the rendered pixmap area, ignoring";
            return;
        }

        qDebug() << "Successfully mapped to viewport position:" << viewportPos;

        // 检查WallManager和LineDrawingTool
        if (!m_wallManager) {
            qDebug() << "ERROR: m_wallManager is null";
            QWidget::mousePressEvent(event);
            return;
        }

        if (!m_wallManager->getLineDrawingTool()) {
            qDebug() << "ERROR: LineDrawingTool is null";
            QWidget::mousePressEvent(event);
            return;
        }

        qDebug() << "LineDrawingTool is available";
        qDebug() << "Current drawing mode:" << static_cast<int>(m_wallManager->getLineDrawingTool()->getDrawingMode());

        // 使用label->viewport映射后的坐标作为事件位置
        QPoint relativePos = QPoint(static_cast<int>(viewportPos.x()), static_cast<int>(viewportPos.y()));
        qDebug() << "Viewport-mapped relative position:" << relativePos;

        // 创建新的鼠标事件，使用渲染视口相对坐标
        QMouseEvent relativeEvent(event->type(), relativePos, event->globalPosition(),
                                event->button(), event->buttons(), event->modifiers());

        qDebug() << "Calling LineDrawingTool::handleMousePressEvent";
        bool handled = m_wallManager->getLineDrawingTool()->handleMousePressEvent(&relativeEvent);
        qDebug() << "LineDrawingTool handled event:" << handled;

        if (handled) {
            qDebug() << "Mouse press event handled by LineDrawingTool at:" << relativePos;
            // 触发渲染更新
            updateRenderView();
            return;
        } else {
            qDebug() << "LineDrawingTool did not handle the event";
        }
    } else {
        qDebug() << "Mouse event is outside render display area";
    }

    // 如果没有被LineDrawingTool处理，调用父类方法
    QWidget::mousePressEvent(event);
}

void Stage1DemoWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 检查鼠标事件是否在渲染显示区域内
    if (m_renderDisplayLabel && m_renderDisplayLabel->geometry().contains(event->pos())) {
        // 将事件传递给LineDrawingTool
        if (m_wallManager && m_wallManager->getLineDrawingTool()) {
            // 将窗口坐标转换为label内部坐标，并映射到渲染视口坐标
            QPoint labelLocal = event->pos() - m_renderDisplayLabel->geometry().topLeft();
            QPointF viewportPos;
            if (!mapLabelPosToViewport(labelLocal, viewportPos)) {
                return;
            }

            QPoint relativePos = QPoint(static_cast<int>(viewportPos.x()), static_cast<int>(viewportPos.y()));

            // 创建新的鼠标事件，使用相对坐标
            QMouseEvent relativeEvent(event->type(), relativePos, event->globalPosition(),
                                    event->button(), event->buttons(), event->modifiers());

            bool handled = m_wallManager->getLineDrawingTool()->handleMouseMoveEvent(&relativeEvent);
            if (handled) {
                // 触发渲染更新（用于实时预览）
                updateRenderView();
                return;
            }
        }
    }

    // 如果没有被LineDrawingTool处理，调用父类方法
    QWidget::mouseMoveEvent(event);
}

void Stage1DemoWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // 检查鼠标事件是否在渲染显示区域内
    if (m_renderDisplayLabel && m_renderDisplayLabel->geometry().contains(event->pos())) {
        // 将事件传递给LineDrawingTool
        if (m_wallManager && m_wallManager->getLineDrawingTool()) {
            // 将窗口坐标转换为label内部坐标，并映射到渲染视口坐标
            QPoint labelLocal = event->pos() - m_renderDisplayLabel->geometry().topLeft();
            QPointF viewportPos;
            if (!mapLabelPosToViewport(labelLocal, viewportPos)) {
                return;
            }

            QPoint relativePos = QPoint(static_cast<int>(viewportPos.x()), static_cast<int>(viewportPos.y()));

            // 创建新的鼠标事件，使用相对坐标
            QMouseEvent relativeEvent(event->type(), relativePos, event->globalPosition(),
                                    event->button(), event->buttons(), event->modifiers());

            bool handled = m_wallManager->getLineDrawingTool()->handleMouseReleaseEvent(&relativeEvent);
            if (handled) {
                qDebug() << "Mouse release event handled by LineDrawingTool at:" << relativePos;
                // 触发渲染更新
                updateRenderView();
                return;
            }
        }
    }

    // 如果没有被LineDrawingTool处理，调用父类方法
    QWidget::mouseReleaseEvent(event);
}

void Stage1DemoWidget::forceLayoutUpdate()
{
    qDebug() << "=== Force layout update called ===";
    qDebug() << "Current widget size:" << size();
    qDebug() << "Parent widget size:" << (parentWidget() ? parentWidget()->size() : QSize(-1, -1));

    // 强制更新所有布局
    if (m_mainLayout) {
        qDebug() << "Updating main layout";
        m_mainLayout->invalidate();
        m_mainLayout->activate();
    }
    if (m_contentLayout) {
        qDebug() << "Updating content layout";
        m_contentLayout->invalidate();
        m_contentLayout->activate();
    }
    if (m_displayLayout) {
        qDebug() << "Updating display layout";
        m_displayLayout->invalidate();
        m_displayLayout->activate();
    }

    // 更新所有控件的几何信息
    if (m_controlPanel) {
        qDebug() << "Control panel size before update:" << m_controlPanel->size();
        m_controlPanel->updateGeometry();
        qDebug() << "Control panel size after update:" << m_controlPanel->size();
    }
    if (m_renderDisplayLabel) {
        qDebug() << "Display label size before update:" << m_renderDisplayLabel->size();
        m_renderDisplayLabel->updateGeometry();
        qDebug() << "Display label size after update:" << m_renderDisplayLabel->size();
    }

    // 强制整个widget调整到父容器大小
    if (parentWidget()) {
        QSize parentSize = parentWidget()->size();
        qDebug() << "Adjusting to parent size:" << parentSize;
        resize(parentSize);
    }

    // 重新应用样式
    updateResponsiveStyles();

    // 强制重绘
    update();

    qDebug() << "=== Layout update completed ===";
}

void Stage1DemoWidget::setupUI()
{
    qDebug() << "Setting up responsive UI layout";

    // 创建全新的布局架构：顶部工具栏 + 主内容区域 + 底部状态栏
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);  // 减少边距
    m_mainLayout->setSpacing(2);                   // 减少间距

    // 创建顶部工具栏
    createTopToolbar();
    m_mainLayout->addWidget(m_topToolbar, 0);      // 不拉伸

    // 创建主内容区域（左侧控制面板 + 右侧显示区域）
    m_contentLayout = new QHBoxLayout();
    m_contentLayout->setSpacing(8);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    // 创建紧凑的左侧控制面板
    createControlPanel();
    m_contentLayout->addWidget(m_controlPanel, 0); // 控制面板不拉伸

    // 创建右侧主显示区域
    m_displayLayout = new QVBoxLayout();
    m_displayLayout->setSpacing(4);
    m_displayLayout->setContentsMargins(0, 0, 0, 0);
    createDisplayArea();
    m_contentLayout->addLayout(m_displayLayout, 1); // 显示区域占据剩余空间并拉伸

    m_mainLayout->addLayout(m_contentLayout, 1);    // 主内容区域拉伸

    // 创建底部状态栏
    createStatusBar();
    m_mainLayout->addWidget(m_statusBar, 0);        // 不拉伸

    setLayout(m_mainLayout);

    qDebug() << "Responsive UI layout setup completed";
}

void Stage1DemoWidget::createTopToolbar()
{
    m_topToolbar = new QWidget();
    // 使用最小高度而不是固定高度，支持响应式设计
    m_topToolbar->setMinimumHeight(50);
    m_topToolbar->setMaximumHeight(80);
    m_topToolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_topToolbar->setStyleSheet(
        "QWidget {"
        "   background-color: #f8f9fa;"
        "   border-bottom: 2px solid #dee2e6;"
        "}"
    );

    QHBoxLayout* toolbarLayout = new QHBoxLayout(m_topToolbar);
    toolbarLayout->setContentsMargins(16, 8, 16, 8);
    toolbarLayout->setSpacing(12);

    // 数据操作按钮组
    QLabel* dataLabel = new QLabel("数据操作:");
    dataLabel->setStyleSheet(
        "QLabel {"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   color: #495057;"
        "   margin-right: 8px;"
        "}"
    );
    toolbarLayout->addWidget(dataLabel);

    // 创建文件操作按钮（重用现有按钮）
    createFileControlButtons();
    toolbarLayout->addWidget(m_loadFileButton);
    toolbarLayout->addWidget(m_generateDataButton);
    toolbarLayout->addWidget(m_clearDataButton);

    // 分隔线
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setStyleSheet("QFrame { color: #dee2e6; }");
    toolbarLayout->addWidget(separator);

    // 主要操作按钮
    QLabel* actionLabel = new QLabel("渲染操作:");
    actionLabel->setStyleSheet(
        "QLabel {"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   color: #495057;"
        "   margin-right: 8px;"
        "}"
    );
    toolbarLayout->addWidget(actionLabel);

    // 创建渲染按钮（重用现有按钮）
    createRenderButtons();
    toolbarLayout->addWidget(m_renderButton);
    toolbarLayout->addWidget(m_saveImageButton);

    // 右侧弹性空间
    toolbarLayout->addStretch();
}

void Stage1DemoWidget::createControlPanel()
{
    m_controlPanel = new QWidget();
    // 使用最小和最大宽度支持响应式设计（增加到35%占比）
    m_controlPanel->setMinimumWidth(320);
    m_controlPanel->setMaximumWidth(500);
    m_controlPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_controlPanel->setStyleSheet(
        "QWidget {"
        "   background-color: #ffffff;"
        "   border-right: 2px solid #dee2e6;"
        "}"
    );

    QVBoxLayout* panelLayout = new QVBoxLayout(m_controlPanel);
    panelLayout->setContentsMargins(12, 12, 12, 12);
    panelLayout->setSpacing(16);

    // 创建渲染参数切换按钮
    createRenderParamsToggleButton();
    panelLayout->addWidget(m_toggleRenderParamsButton);

    // 创建可折叠的渲染参数容器
    createRenderParamsContainer();
    panelLayout->addWidget(m_renderParamsContainer);

    // 创建线框绘制工具切换按钮
    createLineDrawingToggleButton();
    panelLayout->addWidget(m_toggleLineDrawingButton);

    // 创建可折叠的线框绘制工具容器
    createLineDrawingContainer();
    panelLayout->addWidget(m_lineDrawingContainer);

    // 线段绘制控制区域（现在在折叠容器中创建）
    createLineDrawingControls();

    // 底部弹性空间
    panelLayout->addStretch();
}

void Stage1DemoWidget::createStatusBar()
{
    m_statusBar = new QWidget();
    // 使用最小高度支持响应式设计
    m_statusBar->setMinimumHeight(28);
    m_statusBar->setMaximumHeight(40);
    m_statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_statusBar->setStyleSheet(
        "QWidget {"
        "   background-color: #f8f9fa;"
        "   border-top: 1px solid #dee2e6;"
        "}"
    );

    QHBoxLayout* statusLayout = new QHBoxLayout(m_statusBar);
    statusLayout->setContentsMargins(16, 4, 16, 4);
    statusLayout->setSpacing(16);

    // 文件信息标签（重用现有标签）
    m_fileInfoLabel = new QLabel("未加载文件");
    m_fileInfoLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 12px;"
        "   color: #6c757d;"
        "}"
    );
    statusLayout->addWidget(m_fileInfoLabel);

    // 分隔符
    QLabel* separator = new QLabel("|");
    separator->setStyleSheet("QLabel { color: #dee2e6; }");
    statusLayout->addWidget(separator);

    // LOD信息标签（重用现有标签）
    m_lodInfoLabel = new QLabel("LOD未生成");
    m_lodInfoLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 12px;"
        "   color: #6c757d;"
        "}"
    );
    statusLayout->addWidget(m_lodInfoLabel);

    // 右侧弹性空间
    statusLayout->addStretch();
}

void Stage1DemoWidget::createFileControlButtons()
{
    // 创建文件操作按钮，使用工具栏样式
    QString toolbarButtonStyle =
        "QPushButton {"
        "   padding: 8px 16px;"
        "   font-size: 12px;"
        "   font-weight: 500;"
        "   background-color: #007bff;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   min-height: 16px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #0056b3;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004085;"
        "}";

    m_loadFileButton = new QPushButton("加载点云");
    m_loadFileButton->setStyleSheet(toolbarButtonStyle);

    m_generateDataButton = new QPushButton("生成测试数据");
    m_generateDataButton->setStyleSheet(toolbarButtonStyle);

    m_clearDataButton = new QPushButton("清除数据");
    m_clearDataButton->setStyleSheet(toolbarButtonStyle.replace("#007bff", "#dc3545").replace("#0056b3", "#c82333").replace("#004085", "#a02622"));
}

void Stage1DemoWidget::createRenderButtons()
{
    // 创建渲染操作按钮
    QString primaryButtonStyle =
        "QPushButton {"
        "   padding: 10px 20px;"
        "   font-size: 13px;"
        "   font-weight: bold;"
        "   background-color: #28a745;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   min-height: 18px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #218838;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1e7e34;"
        "}";

    QString secondaryButtonStyle =
        "QPushButton {"
        "   padding: 8px 16px;"
        "   font-size: 12px;"
        "   font-weight: 500;"
        "   background-color: #6c757d;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   min-height: 16px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #5a6268;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #495057;"
        "}";

    m_renderButton = new QPushButton("渲染俯视图");
    m_renderButton->setStyleSheet(primaryButtonStyle);

    m_saveImageButton = new QPushButton("保存图像");
    m_saveImageButton->setStyleSheet(secondaryButtonStyle);
}

void Stage1DemoWidget::createCompactLODControl()
{
    m_compactLODWidget = new QWidget();
    m_compactLODWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #f8f9fa;"
        "   border: 1px solid #dee2e6;"
        "   border-radius: 6px;"
        "   padding: 8px;"
        "}"
    );

    QVBoxLayout* lodLayout = new QVBoxLayout(m_compactLODWidget);
    lodLayout->setContentsMargins(12, 12, 12, 12);
    lodLayout->setSpacing(10);

    // LOD标题
    QLabel* lodTitle = new QLabel("LOD细节层次");
    lodTitle->setStyleSheet(
        "QLabel {"
        "   font-weight: bold;"
        "   font-size: 13px;"
        "   color: #495057;"
        "   margin-bottom: 4px;"
        "}"
    );
    lodLayout->addWidget(lodTitle);

    // LOD策略
    QHBoxLayout* strategyLayout = new QHBoxLayout();
    strategyLayout->setSpacing(8);

    QLabel* strategyLabel = new QLabel("策略:");
    strategyLabel->setStyleSheet("QLabel { font-size: 12px; color: #6c757d; min-width: 40px; }");
    strategyLayout->addWidget(strategyLabel);

    m_lodStrategyCombo = new QComboBox();
    m_lodStrategyCombo->addItems({"均匀采样", "体素网格", "随机采样", "重要性采样"});
    m_lodStrategyCombo->setStyleSheet(
        "QComboBox {"
        "   padding: 4px 8px;"
        "   font-size: 11px;"
        "   border: 1px solid #ced4da;"
        "   border-radius: 3px;"
        "   background-color: white;"
        "}"
    );
    strategyLayout->addWidget(m_lodStrategyCombo);
    lodLayout->addLayout(strategyLayout);

    // LOD级别
    QHBoxLayout* levelLayout = new QHBoxLayout();
    levelLayout->setSpacing(8);

    QLabel* levelLabel = new QLabel("级别:");
    levelLabel->setStyleSheet("QLabel { font-size: 12px; color: #6c757d; min-width: 40px; }");
    levelLayout->addWidget(levelLabel);

    m_lodLevelSlider = new QSlider(Qt::Horizontal);
    m_lodLevelSlider->setRange(0, 3);
    m_lodLevelSlider->setValue(0);
    m_lodLevelSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "   border: 1px solid #ced4da;"
        "   height: 4px;"
        "   background: #e9ecef;"
        "   border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "   background: #007bff;"
        "   border: 1px solid #0056b3;"
        "   width: 12px;"
        "   height: 12px;"
        "   margin: -4px 0;"
        "   border-radius: 6px;"
        "}"
    );
    levelLayout->addWidget(m_lodLevelSlider);

    m_lodLevelLabel = new QLabel("级别 0");
    m_lodLevelLabel->setStyleSheet("QLabel { font-size: 11px; color: #6c757d; min-width: 50px; }");
    levelLayout->addWidget(m_lodLevelLabel);
    lodLayout->addLayout(levelLayout);

    // LOD生成按钮
    m_generateLODButton = new QPushButton("生成LOD");
    m_generateLODButton->setStyleSheet(
        "QPushButton {"
        "   padding: 6px 12px;"
        "   font-size: 11px;"
        "   font-weight: 500;"
        "   background-color: #17a2b8;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #138496;"
        "}"
    );
    lodLayout->addWidget(m_generateLODButton);
}

void Stage1DemoWidget::createCompactColorControl()
{
    m_compactColorWidget = new QWidget();
    m_compactColorWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #f8f9fa;"
        "   border: 1px solid #dee2e6;"
        "   border-radius: 6px;"
        "   padding: 8px;"
        "}"
    );

    QVBoxLayout* colorLayout = new QVBoxLayout(m_compactColorWidget);
    colorLayout->setContentsMargins(12, 12, 12, 12);
    colorLayout->setSpacing(10);

    // 颜色映射标题
    QLabel* colorTitle = new QLabel("颜色映射");
    colorTitle->setStyleSheet(
        "QLabel {"
        "   font-weight: bold;"
        "   font-size: 13px;"
        "   color: #495057;"
        "   margin-bottom: 4px;"
        "}"
    );
    colorLayout->addWidget(colorTitle);

    // 颜色方案
    QHBoxLayout* schemeLayout = new QHBoxLayout();
    schemeLayout->setSpacing(8);

    QLabel* schemeLabel = new QLabel("方案:");
    schemeLabel->setStyleSheet(
        "QLabel { "
        "   font-size: 13px; "
        "   font-weight: 500; "
        "   color: #2c3e50; "
        "   min-width: 40px; "
        "   background-color: transparent; "
        "}"
    );
    schemeLayout->addWidget(schemeLabel);

    m_colorSchemeCombo = new QComboBox();
    m_colorSchemeCombo->addItems({"高度", "强度", "分类", "RGB"});
    m_colorSchemeCombo->setStyleSheet(
        "QComboBox {"
        "   padding: 6px 10px;"
        "   font-size: 12px;"
        "   font-weight: 500;"
        "   color: #2c3e50;"
        "   background-color: white;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 4px;"
        "}"
        "QComboBox:focus {"
        "   border-color: #3498db;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "}"
        "QComboBox::down-arrow {"
        "   image: none;"
        "   border: none;"
        "}"
    );
    schemeLayout->addWidget(m_colorSchemeCombo);
    colorLayout->addLayout(schemeLayout);

    // 数值范围
    QHBoxLayout* rangeLayout = new QHBoxLayout();
    rangeLayout->setSpacing(10);

    QLabel* rangeLabel = new QLabel("范围:");
    rangeLabel->setStyleSheet(
        "QLabel { "
        "   font-size: 13px; "
        "   font-weight: 500; "
        "   color: #2c3e50; "
        "   min-width: 40px; "
        "   background-color: transparent; "
        "}"
    );
    rangeLayout->addWidget(rangeLabel);

    m_minValueSpin = new QDoubleSpinBox();
    m_minValueSpin->setRange(-1000, 1000);
    m_minValueSpin->setValue(0);
    m_minValueSpin->setMinimumWidth(90);
    m_minValueSpin->setMaximumWidth(110);
    m_minValueSpin->setStyleSheet(
        "QDoubleSpinBox {"
        "   padding: 6px 8px;"
        "   font-size: 12px;"
        "   font-weight: 500;"
        "   color: #2c3e50;"
        "   background-color: white;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   min-height: 24px;"
        "}"
        "QDoubleSpinBox:focus {"
        "   border-color: #3498db;"
        "   background-color: #f8f9fa;"
        "}"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
        "   width: 16px;"
        "   border: none;"
        "}"
    );
    rangeLayout->addWidget(m_minValueSpin);

    QLabel* toLabel = new QLabel("~");
    toLabel->setStyleSheet(
        "QLabel { "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   color: #2c3e50; "
        "   background-color: transparent; "
        "}"
    );
    rangeLayout->addWidget(toLabel);

    m_maxValueSpin = new QDoubleSpinBox();
    m_maxValueSpin->setRange(-1000, 1000);
    m_maxValueSpin->setValue(100);
    m_maxValueSpin->setMinimumWidth(90);
    m_maxValueSpin->setMaximumWidth(110);
    m_maxValueSpin->setStyleSheet(
        "QDoubleSpinBox {"
        "   padding: 6px 8px;"
        "   font-size: 12px;"
        "   font-weight: 500;"
        "   color: #2c3e50;"
        "   background-color: white;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   min-height: 24px;"
        "}"
        "QDoubleSpinBox:focus {"
        "   border-color: #3498db;"
        "   background-color: #f8f9fa;"
        "}"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
        "   width: 16px;"
        "   border: none;"
        "}"
    );
    rangeLayout->addWidget(m_maxValueSpin);

    rangeLayout->addStretch();
    colorLayout->addLayout(rangeLayout);

    // 颜色条生成按钮
    m_generateColorBarButton = new QPushButton("生成颜色条");
    m_generateColorBarButton->setStyleSheet(
        "QPushButton {"
        "   padding: 6px 12px;"
        "   font-size: 11px;"
        "   font-weight: 500;"
        "   background-color: #fd7e14;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e8650e;"
        "}"
    );
    colorLayout->addWidget(m_generateColorBarButton);

    // 颜色条显示
    m_colorBarLabel = new QLabel();
    m_colorBarLabel->setMinimumHeight(16);
    m_colorBarLabel->setMaximumHeight(24);
    m_colorBarLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_colorBarLabel->setStyleSheet(
        "QLabel {"
        "   border: 1px solid #ced4da;"
        "   background-color: white;"
        "   border-radius: 3px;"
        "}"
    );
    colorLayout->addWidget(m_colorBarLabel);
}

void Stage1DemoWidget::createCompactRenderControl()
{
    m_compactRenderWidget = new QWidget();
    m_compactRenderWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #f8f9fa;"
        "   border: 1px solid #dee2e6;"
        "   border-radius: 6px;"
        "   padding: 8px;"
        "}"
    );

    QVBoxLayout* renderLayout = new QVBoxLayout(m_compactRenderWidget);
    renderLayout->setContentsMargins(12, 12, 12, 12);
    renderLayout->setSpacing(10);

    // 渲染控制标题
    QLabel* renderTitle = new QLabel("渲染设置");
    renderTitle->setStyleSheet(
        "QLabel {"
        "   font-weight: bold;"
        "   font-size: 13px;"
        "   color: #495057;"
        "   margin-bottom: 4px;"
        "}"
    );
    renderLayout->addWidget(renderTitle);

    // 渲染模式
    QHBoxLayout* modeLayout = new QHBoxLayout();
    modeLayout->setSpacing(8);

    QLabel* modeLabel = new QLabel("模式:");
    modeLabel->setStyleSheet(
        "QLabel { "
        "   font-size: 13px; "
        "   font-weight: 500; "
        "   color: #2c3e50; "
        "   min-width: 50px; "
        "   background-color: transparent; "
        "}"
    );
    modeLayout->addWidget(modeLabel);

    m_renderModeCombo = new QComboBox();
    m_renderModeCombo->addItems({"点云", "密度图", "等高线", "热力图"});
    m_renderModeCombo->setStyleSheet(
        "QComboBox {"
        "   padding: 6px 10px;"
        "   font-size: 12px;"
        "   font-weight: 500;"
        "   color: #2c3e50;"
        "   background-color: white;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   min-width: 100px;"
        "   min-height: 24px;"
        "}"
        "QComboBox:focus {"
        "   border-color: #3498db;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "   width: 20px;"
        "}"
        "QComboBox::down-arrow {"
        "   image: none;"
        "   border: none;"
        "}"
    );
    modeLayout->addWidget(m_renderModeCombo);
    renderLayout->addLayout(modeLayout);

    // 点大小
    QHBoxLayout* sizeLayout = new QHBoxLayout();
    sizeLayout->setSpacing(10);

    QLabel* sizeLabel = new QLabel("点大小:");
    sizeLabel->setStyleSheet(
        "QLabel { "
        "   font-size: 13px; "
        "   font-weight: 500; "
        "   color: #2c3e50; "
        "   min-width: 50px; "
        "   background-color: transparent; "
        "}"
    );
    sizeLayout->addWidget(sizeLabel);

    m_pointSizeSpin = new QDoubleSpinBox();
    m_pointSizeSpin->setRange(0.1, 10.0);
    m_pointSizeSpin->setValue(2.0);
    m_pointSizeSpin->setSingleStep(0.1);
    m_pointSizeSpin->setMinimumWidth(85);
    m_pointSizeSpin->setMaximumWidth(100);
    m_pointSizeSpin->setStyleSheet(
        "QDoubleSpinBox {"
        "   padding: 6px 8px;"
        "   font-size: 12px;"
        "   font-weight: 500;"
        "   color: #2c3e50;"
        "   background-color: white;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   min-height: 24px;"
        "}"
        "QDoubleSpinBox:focus {"
        "   border-color: #3498db;"
        "   background-color: #f8f9fa;"
        "}"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
        "   width: 16px;"
        "   border: none;"
        "}"
    );
    sizeLayout->addWidget(m_pointSizeSpin);

    QLabel* unitLabel = new QLabel("px");
    unitLabel->setStyleSheet(
        "QLabel { "
        "   font-size: 12px; "
        "   font-weight: 500; "
        "   color: #495057; "
        "   background-color: transparent; "
        "}"
    );
    sizeLayout->addWidget(unitLabel);

    sizeLayout->addStretch();
    renderLayout->addLayout(sizeLayout);
}

void Stage1DemoWidget::createLineDrawingControls()
{
    qDebug() << "Creating line drawing controls...";

    // 确保WallExtractionManager已初始化
    if (!m_wallManager) {
        qCritical() << "WallExtractionManager is null -阶段二功能不可用";
        return;
    }

    if (!m_wallManager->isInitialized()) {
        qCritical() << "WallExtractionManager not initialized - 阶段二功能不可用";
        return;
    }

    if (!m_wallManager->getLineDrawingTool()) {
        qCritical() << "LineDrawingTool not available - 阶段二功能不可用";
        return;
    }

    qDebug() << "WallExtractionManager and LineDrawingTool are ready";

    try {
        // 创建线段绘制工具栏
        m_lineDrawingToolbar = new WallExtraction::LineDrawingToolbar(this);

        // 初始化LineDrawingTool
        if (m_wallManager->getLineDrawingTool()) {
            bool initSuccess = m_wallManager->getLineDrawingTool()->initialize();
            if (initSuccess) {
                qDebug() << "LineDrawingTool initialized successfully";

                // 设置基于渲染器边界的简化坐标转换函数
                qDebug() << "Setting up LineDrawingTool with renderer view bounds:" << m_currentViewBounds;
                qDebug() << "Current viewport size:" << m_currentViewportSize;

                // 设置screenToWorld转换函数（直接使用渲染器的投影管理器，确保完全一致）
                auto screenToWorldFunc = [this](const QVector2D& screenPoint) -> QVector3D {
                    qDebug() << "=== screenToWorldFunc (Manual) ===";
                    qDebug() << "Input screenPoint:" << screenPoint;

                    // 使用手动转换，与绘制时保持一致
                    if (m_currentViewBounds.isEmpty() || !m_currentViewportSize.isValid()) {
                        qWarning() << "Invalid view bounds or viewport size";
                        return QVector3D(screenPoint.x(), screenPoint.y(), 0.0f);
                    }

                    // 像素坐标 -> 标准化坐标 -> 世界坐标
                    float normalizedX = screenPoint.x() / m_currentViewportSize.width();
                    float normalizedY = screenPoint.y() / m_currentViewportSize.height();

                    float worldX = m_currentViewBounds.left() + normalizedX * m_currentViewBounds.width();
                    float worldY = m_currentViewBounds.top() + normalizedY * m_currentViewBounds.height();

                    QVector3D result(worldX, worldY, 0.0f);
                    qDebug() << "Manual conversion: pixels" << screenPoint << "-> normalized" << normalizedX << normalizedY << "-> world" << result;
                    return result;
                };

                // 设置worldToScreen转换函数（直接使用渲染器的投影管理器，确保完全一致）
                auto worldToScreenFunc = [this](const QVector3D& worldPoint) -> QVector2D {
                    qDebug() << "=== worldToScreenFunc (Manual) ===";
                    qDebug() << "Input worldPoint:" << worldPoint;

                    // 使用手动转换，与绘制时保持一致
                    if (m_currentViewBounds.isEmpty() || !m_currentViewportSize.isValid()) {
                        qWarning() << "Invalid view bounds or viewport size";
                        return QVector2D(worldPoint.x(), worldPoint.y());
                    }

                    // 世界坐标 -> 标准化坐标 -> 像素坐标
                    float normalizedX = (worldPoint.x() - m_currentViewBounds.left()) / m_currentViewBounds.width();
                    float normalizedY = (worldPoint.y() - m_currentViewBounds.top()) / m_currentViewBounds.height();

                    QVector2D result(normalizedX * m_currentViewportSize.width(),
                                    normalizedY * m_currentViewportSize.height());

                    qDebug() << "Manual conversion: world" << worldPoint << "-> normalized" << normalizedX << normalizedY << "-> pixels" << result;
                    return result;
                };

                m_wallManager->getLineDrawingTool()->setCoordinateConverter(screenToWorldFunc, worldToScreenFunc);
                qDebug() << "LineDrawingTool coordinate converters set to use direct renderer coordinate system";
            } else {
                qWarning() << "Failed to initialize LineDrawingTool";
            }
        }

        m_lineDrawingToolbar->setLineDrawingTool(m_wallManager->getLineDrawingTool());

        // 将控件添加到折叠容器中
        addLineDrawingControlsToContainer();

        qDebug() << "Line drawing controls created successfully";

    } catch (const std::exception& e) {
        qCritical() << "Failed to create line drawing controls:" << e.what();
        m_lineDrawingToolbar = nullptr;
    }
}

QString Stage1DemoWidget::getResponsiveButtonStyle(const QString& baseColor, bool isPrimary)
{
    // 根据DPI和窗口大小计算响应式尺寸
    qreal dpiScale = qApp->devicePixelRatio();
    int baseFontSize = isPrimary ? 13 : 12;
    int fontSize = qMax(10, int(baseFontSize * dpiScale));

    QString hoverColor = baseColor;
    hoverColor.replace("ff", "cc"); // 简单的颜色变暗效果

    return QString(
        "QPushButton {"
        "   padding: %1em %2em;"
        "   font-size: %3px;"
        "   font-weight: %4;"
        "   background-color: %5;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 0.3em;"
        "   min-height: %6px;"
        "}"
        "QPushButton:hover {"
        "   background-color: %7;"
        "}"
        "QPushButton:pressed {"
        "   background-color: %8;"
        "}"
    ).arg(isPrimary ? "0.8" : "0.6")
     .arg(isPrimary ? "1.2" : "1.0")
     .arg(fontSize)
     .arg(isPrimary ? "bold" : "500")
     .arg(baseColor)
     .arg(isPrimary ? 24 : 20)
     .arg(hoverColor)
     .arg(hoverColor.replace("cc", "aa"));
}

QString Stage1DemoWidget::getResponsiveGroupBoxStyle()
{
    qreal dpiScale = qApp->devicePixelRatio();
    int fontSize = qMax(12, int(14 * dpiScale));

    return QString(
        "QGroupBox {"
        "   font-weight: bold;"
        "   font-size: %1px;"
        "   color: #2c3e50;"
        "   border: 0.15em solid #bdc3c7;"
        "   border-radius: 0.4em;"
        "   margin-top: 0.5em;"
        "   padding-top: 0.5em;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 0.8em;"
        "   padding: 0 0.5em 0 0.5em;"
        "   background-color: white;"
        "}"
    ).arg(fontSize);
}

QString Stage1DemoWidget::getResponsiveLabelStyle(int level)
{
    qreal dpiScale = qApp->devicePixelRatio();
    int baseFontSize = (level == 1) ? 16 : (level == 2) ? 13 : 12;
    int fontSize = qMax(10, int(baseFontSize * dpiScale));
    QString fontWeight = (level == 1) ? "bold" : (level == 2) ? "500" : "normal";

    return QString(
        "QLabel {"
        "   font-size: %1px;"
        "   font-weight: %2;"
        "   color: %3;"
        "}"
    ).arg(fontSize)
     .arg(fontWeight)
     .arg(level == 1 ? "#212529" : level == 2 ? "#495057" : "#6c757d");
}

void Stage1DemoWidget::updateResponsiveStyles()
{
    qDebug() << "Updating responsive styles...";
    qreal dpiScale = qApp->devicePixelRatio();
    qDebug() << "Current DPI scale:" << dpiScale;

    // 在窗口大小变化时更新样式
    if (m_topToolbar) {
        m_topToolbar->setStyleSheet(
            "QWidget {"
            "   background-color: #f8f9fa;"
            "   border-bottom: 0.15em solid #dee2e6;"
            "}"
        );
        qDebug() << "Updated toolbar style";
    }

    if (m_controlPanel) {
        m_controlPanel->setStyleSheet(
            "QWidget {"
            "   background-color: #ffffff;"
            "   border-right: 0.15em solid #dee2e6;"
            "}"
        );
        qDebug() << "Updated control panel style";
    }

    if (m_statusBar) {
        m_statusBar->setStyleSheet(
            "QWidget {"
            "   background-color: #f8f9fa;"
            "   border-top: 0.1em solid #dee2e6;"
            "}"
        );
        qDebug() << "Updated status bar style";
    }

    // 更新按钮样式
    if (m_renderButton) {
        m_renderButton->setStyleSheet(getResponsiveButtonStyle("#28a745", true));
        qDebug() << "Updated renderButton style";
    }
    if (m_generateDataButton) {
        m_generateDataButton->setStyleSheet(getResponsiveButtonStyle("#007bff", false));
        qDebug() << "Updated generateDataButton style";
    }
    if (m_clearDataButton) {
        m_clearDataButton->setStyleSheet(getResponsiveButtonStyle("#dc3545", false));
        qDebug() << "Updated clearDataButton style";
    }

    // 更新其他控制按钮样式
    if (m_loadFileButton) {
        m_loadFileButton->setStyleSheet(getResponsiveButtonStyle("#17a2b8", false));
    }
    if (m_saveImageButton) {
        m_saveImageButton->setStyleSheet(getResponsiveButtonStyle("#6c757d", false));
    }
    if (m_generateLODButton) {
        m_generateLODButton->setStyleSheet(getResponsiveButtonStyle("#fd7e14", false));
    }
    if (m_generateColorBarButton) {
        m_generateColorBarButton->setStyleSheet(getResponsiveButtonStyle("#20c997", false));
    }

    qDebug() << "All responsive styles updated successfully";
}

void Stage1DemoWidget::createFileControlGroup()
{
    m_fileControlGroup = new QGroupBox("文件操作");
    m_fileControlGroup->setStyleSheet(
        "QGroupBox {"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   color: #2c3e50;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 6px;"
        "   margin-top: 8px;"
        "   padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 12px;"
        "   padding: 0 8px 0 8px;"
        "   background-color: white;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(m_fileControlGroup);
    layout->setSpacing(12);                              // 增加控件间距
    layout->setContentsMargins(16, 20, 16, 16);         // 增加内边距

    m_loadFileButton = new QPushButton("加载点云");
    m_generateDataButton = new QPushButton("生成测试数据");
    m_clearDataButton = new QPushButton("清除数据");
    m_fileInfoLabel = new QLabel("未加载文件");

    // 优化按钮样式 - 增大字体和内边距
    QString buttonStyle =
        "QPushButton {"
        "   padding: 10px 16px;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   min-height: 20px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #21618c;"
        "}";

    m_loadFileButton->setStyleSheet(buttonStyle);
    m_generateDataButton->setStyleSheet(buttonStyle);
    m_clearDataButton->setStyleSheet(buttonStyle);

    // 优化信息标签样式 - 增大字体和改善对比度
    m_fileInfoLabel->setStyleSheet(
        "QLabel {"
        "   color: #34495e;"
        "   font-size: 12px;"
        "   font-style: italic;"
        "   padding: 8px;"
        "   background-color: #ecf0f1;"
        "   border-radius: 3px;"
        "}"
    );
    m_fileInfoLabel->setWordWrap(true);
    m_fileInfoLabel->setMinimumHeight(40);               // 确保标签有足够高度

    layout->addWidget(m_loadFileButton);
    layout->addWidget(m_generateDataButton);
    layout->addWidget(m_clearDataButton);
    layout->addWidget(m_fileInfoLabel);

    connect(m_loadFileButton, &QPushButton::clicked, this, &Stage1DemoWidget::loadPointCloudFile);
    connect(m_generateDataButton, &QPushButton::clicked, this, &Stage1DemoWidget::generateTestData);
    connect(m_clearDataButton, &QPushButton::clicked, this, &Stage1DemoWidget::clearPointCloud);
}

void Stage1DemoWidget::createLODControlGroup()
{
    m_lodControlGroup = new QGroupBox("LOD细节层次系统");
    m_lodControlGroup->setStyleSheet(
        "QGroupBox {"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   color: #2c3e50;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 6px;"
        "   margin-top: 8px;"
        "   padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 12px;"
        "   padding: 0 8px 0 8px;"
        "   background-color: white;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(m_lodControlGroup);
    layout->setSpacing(14);                              // 增加控件间距
    layout->setContentsMargins(16, 20, 16, 16);         // 增加内边距

    // LOD策略选择
    QLabel* strategyLabel = new QLabel("LOD策略:");
    strategyLabel->setStyleSheet(
        "QLabel {"
        "   font-weight: 500;"
        "   font-size: 13px;"
        "   color: #2c3e50;"
        "   margin-bottom: 4px;"
        "}"
    );
    layout->addWidget(strategyLabel);

    m_lodStrategyCombo = new QComboBox();
    m_lodStrategyCombo->addItems({"均匀采样", "体素网格", "随机采样", "重要性采样"});
    m_lodStrategyCombo->setStyleSheet(
        "QComboBox {"
        "   padding: 8px 12px;"
        "   font-size: 12px;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "   min-height: 20px;"
        "}"
        "QComboBox:focus {"
        "   border-color: #3498db;"
        "}"
        "QComboBox::drop-down {"
        "   width: 20px;"
        "   border: none;"
        "}"
    );
    layout->addWidget(m_lodStrategyCombo);

    // 添加分隔空间
    layout->addSpacing(8);

    // LOD级别控制
    QLabel* levelLabel = new QLabel("LOD级别:");
    levelLabel->setStyleSheet(
        "QLabel {"
        "   font-weight: 500;"
        "   font-size: 13px;"
        "   color: #2c3e50;"
        "   margin-bottom: 4px;"
        "}"
    );
    layout->addWidget(levelLabel);

    m_lodLevelSlider = new QSlider(Qt::Horizontal);
    m_lodLevelSlider->setRange(0, 3);
    m_lodLevelSlider->setValue(0);
    m_lodLevelSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "   border: 1px solid #bdc3c7;"
        "   height: 6px;"
        "   background: #ecf0f1;"
        "   border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "   background: #3498db;"
        "   border: 2px solid #2980b9;"
        "   width: 16px;"
        "   height: 16px;"
        "   margin: -6px 0;"
        "   border-radius: 8px;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "   background: #2980b9;"
        "}"
    );
    m_lodLevelSlider->setMinimumHeight(30);              // 增加滑块操作区域
    layout->addWidget(m_lodLevelSlider);

    m_lodLevelLabel = new QLabel("级别 0 (100%)");
    m_lodLevelLabel->setStyleSheet(
        "QLabel {"
        "   color: #7f8c8d;"
        "   font-size: 12px;"
        "   font-style: italic;"
        "   padding: 4px;"
        "   background-color: #f8f9fa;"
        "   border-radius: 3px;"
        "}"
    );
    m_lodLevelLabel->setAlignment(Qt::AlignCenter);
    m_lodLevelLabel->setMinimumHeight(28);               // 确保标签有足够高度
    layout->addWidget(m_lodLevelLabel);

    // 添加分隔空间
    layout->addSpacing(8);

    m_generateLODButton = new QPushButton("生成LOD级别");
    m_generateLODButton->setStyleSheet(
        "QPushButton {"
        "   padding: 10px 16px;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "   background-color: #27ae60;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   min-height: 20px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #229954;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1e8449;"
        "}"
    );
    layout->addWidget(m_generateLODButton);

    m_lodInfoLabel = new QLabel("LOD未生成");
    m_lodInfoLabel->setStyleSheet(
        "QLabel {"
        "   color: #34495e;"
        "   font-size: 12px;"
        "   font-style: italic;"
        "   padding: 8px;"
        "   background-color: #ecf0f1;"
        "   border-radius: 3px;"
        "}"
    );
    m_lodInfoLabel->setWordWrap(true);
    m_lodInfoLabel->setMinimumHeight(40);                // 确保标签有足够高度
    layout->addWidget(m_lodInfoLabel);
    
    connect(m_lodLevelSlider, &QSlider::valueChanged, this, &Stage1DemoWidget::onLODLevelChanged);
    connect(m_lodStrategyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Stage1DemoWidget::onLODStrategyChanged);
    connect(m_generateLODButton, &QPushButton::clicked, this, &Stage1DemoWidget::generateLODLevels);
}

void Stage1DemoWidget::createColorMappingGroup()
{
    m_colorMappingGroup = new QGroupBox("颜色映射");
    m_colorMappingGroup->setStyleSheet(
        "QGroupBox {"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   color: #2c3e50;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 6px;"
        "   margin-top: 8px;"
        "   padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 12px;"
        "   padding: 0 8px 0 8px;"
        "   background-color: white;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(m_colorMappingGroup);
    layout->setSpacing(14);                              // 增加控件间距
    layout->setContentsMargins(16, 20, 16, 16);         // 增加内边距

    // 颜色方案选择
    QLabel* schemeLabel = new QLabel("颜色方案:");
    schemeLabel->setStyleSheet(
        "QLabel {"
        "   font-weight: 500;"
        "   font-size: 13px;"
        "   color: #2c3e50;"
        "   margin-bottom: 4px;"
        "}"
    );
    layout->addWidget(schemeLabel);

    m_colorSchemeCombo = new QComboBox();
    m_colorSchemeCombo->addItems({"高度", "强度", "分类", "RGB"});
    m_colorSchemeCombo->setStyleSheet(
        "QComboBox {"
        "   padding: 8px 12px;"
        "   font-size: 12px;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "   min-height: 20px;"
        "}"
        "QComboBox:focus {"
        "   border-color: #3498db;"
        "}"
    );
    layout->addWidget(m_colorSchemeCombo);

    // 添加分隔空间
    layout->addSpacing(8);

    // 值范围控制 - 优化布局和间距
    QLabel* rangeLabel = new QLabel("数值范围:");
    rangeLabel->setStyleSheet(
        "QLabel {"
        "   font-weight: 500;"
        "   font-size: 13px;"
        "   color: #2c3e50;"
        "   margin-bottom: 4px;"
        "}"
    );
    layout->addWidget(rangeLabel);

    QHBoxLayout* rangeLayout = new QHBoxLayout();
    rangeLayout->setSpacing(12);                         // 增加水平间距

    QLabel* minLabel = new QLabel("最小:");
    minLabel->setStyleSheet(
        "QLabel {"
        "   font-weight: 500;"
        "   font-size: 12px;"
        "   color: #2c3e50;"
        "   min-width: 35px;"
        "}"
    );
    rangeLayout->addWidget(minLabel);

    m_minValueSpin = new QDoubleSpinBox();
    m_minValueSpin->setRange(-1000, 1000);
    m_minValueSpin->setValue(0);
    m_minValueSpin->setMinimumWidth(90);                 // 增加最小宽度确保数字完全可见
    m_minValueSpin->setStyleSheet(
        "QDoubleSpinBox {"
        "   padding: 6px 8px;"
        "   font-size: 12px;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "}"
        "QDoubleSpinBox:focus {"
        "   border-color: #3498db;"
        "}"
    );
    rangeLayout->addWidget(m_minValueSpin);

    // 添加弹性空间
    rangeLayout->addSpacing(16);

    QLabel* maxLabel = new QLabel("最大:");
    maxLabel->setStyleSheet(
        "QLabel {"
        "   font-weight: 500;"
        "   font-size: 12px;"
        "   color: #2c3e50;"
        "   min-width: 35px;"
        "}"
    );
    rangeLayout->addWidget(maxLabel);

    m_maxValueSpin = new QDoubleSpinBox();
    m_maxValueSpin->setRange(-1000, 1000);
    m_maxValueSpin->setValue(100);
    m_maxValueSpin->setMinimumWidth(90);                 // 增加最小宽度确保数字完全可见
    m_maxValueSpin->setStyleSheet(
        "QDoubleSpinBox {"
        "   padding: 6px 8px;"
        "   font-size: 12px;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "}"
        "QDoubleSpinBox:focus {"
        "   border-color: #3498db;"
        "}"
    );
    rangeLayout->addWidget(m_maxValueSpin);

    rangeLayout->addStretch();                           // 右侧弹性空间
    layout->addLayout(rangeLayout);

    // 添加分隔空间
    layout->addSpacing(8);

    m_generateColorBarButton = new QPushButton("生成颜色条");
    m_generateColorBarButton->setStyleSheet(
        "QPushButton {"
        "   padding: 10px 16px;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "   background-color: #e74c3c;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   min-height: 20px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c0392b;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #a93226;"
        "}"
    );
    layout->addWidget(m_generateColorBarButton);

    m_colorBarLabel = new QLabel();
    m_colorBarLabel->setMinimumHeight(24);
    m_colorBarLabel->setMaximumHeight(40);
    m_colorBarLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_colorBarLabel->setStyleSheet(
        "QLabel {"
        "   border: 2px solid #bdc3c7;"
        "   background-color: #f8f9fa;"
        "   border-radius: 4px;"
        "}"
    );
    layout->addWidget(m_colorBarLabel);
    
    connect(m_colorSchemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Stage1DemoWidget::onColorSchemeChanged);
    connect(m_minValueSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &Stage1DemoWidget::onColorRangeChanged);
    connect(m_maxValueSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &Stage1DemoWidget::onColorRangeChanged);
    connect(m_generateColorBarButton, &QPushButton::clicked, this, &Stage1DemoWidget::generateColorBar);
}

void Stage1DemoWidget::createRenderControlGroup()
{
    m_renderControlGroup = new QGroupBox("俯视图渲染");
    m_renderControlGroup->setStyleSheet(
        "QGroupBox {"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   color: #2c3e50;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 6px;"
        "   margin-top: 8px;"
        "   padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 12px;"
        "   padding: 0 8px 0 8px;"
        "   background-color: white;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(m_renderControlGroup);
    layout->setSpacing(14);                              // 增加控件间距
    layout->setContentsMargins(16, 20, 16, 16);         // 增加内边距

    // 渲染模式选择
    QLabel* modeLabel = new QLabel("渲染模式:");
    modeLabel->setStyleSheet(
        "QLabel {"
        "   font-weight: 500;"
        "   font-size: 13px;"
        "   color: #2c3e50;"
        "   margin-bottom: 4px;"
        "}"
    );
    layout->addWidget(modeLabel);

    m_renderModeCombo = new QComboBox();
    m_renderModeCombo->addItems({"点云", "密度图", "等高线", "热力图"});
    m_renderModeCombo->setStyleSheet(
        "QComboBox {"
        "   padding: 8px 12px;"
        "   font-size: 12px;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "   min-height: 20px;"
        "}"
        "QComboBox:focus {"
        "   border-color: #3498db;"
        "}"
    );
    layout->addWidget(m_renderModeCombo);

    // 添加分隔空间
    layout->addSpacing(8);

    // 点大小控制 - 优化布局和间距
    QLabel* sizeLabel = new QLabel("点大小:");
    sizeLabel->setStyleSheet(
        "QLabel {"
        "   font-weight: 500;"
        "   font-size: 13px;"
        "   color: #2c3e50;"
        "   margin-bottom: 4px;"
        "}"
    );
    layout->addWidget(sizeLabel);

    QHBoxLayout* sizeLayout = new QHBoxLayout();
    sizeLayout->setSpacing(12);                          // 增加水平间距

    m_pointSizeSpin = new QDoubleSpinBox();
    m_pointSizeSpin->setRange(0.1, 10.0);
    m_pointSizeSpin->setValue(2.0);
    m_pointSizeSpin->setSingleStep(0.1);
    m_pointSizeSpin->setMinimumWidth(90);                // 增加最小宽度确保数字完全可见
    m_pointSizeSpin->setStyleSheet(
        "QDoubleSpinBox {"
        "   padding: 6px 8px;"
        "   font-size: 12px;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "}"
        "QDoubleSpinBox:focus {"
        "   border-color: #3498db;"
        "}"
    );
    sizeLayout->addWidget(m_pointSizeSpin);

    QLabel* unitLabel = new QLabel("像素");
    unitLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 11px;"
        "   color: #7f8c8d;"
        "   margin-left: 4px;"
        "}"
    );
    sizeLayout->addWidget(unitLabel);

    sizeLayout->addStretch();                            // 右侧弹性空间
    layout->addLayout(sizeLayout);

    // 添加分隔空间
    layout->addSpacing(12);

    // 操作按钮 - 增大主要按钮
    m_renderButton = new QPushButton("渲染俯视图");
    m_renderButton->setStyleSheet(
        "QPushButton {"
        "   padding: 12px 20px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   background-color: #9b59b6;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   min-height: 24px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #8e44ad;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #7d3c98;"
        "}"
    );
    layout->addWidget(m_renderButton);

    // 添加按钮间距
    layout->addSpacing(6);

    m_saveImageButton = new QPushButton("保存图像");
    m_saveImageButton->setStyleSheet(
        "QPushButton {"
        "   padding: 8px 16px;"
        "   font-size: 12px;"
        "   font-weight: 500;"
        "   background-color: #95a5a6;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   min-height: 18px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #7f8c8d;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #6c7b7d;"
        "}"
    );
    layout->addWidget(m_saveImageButton);
    
    connect(m_renderModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Stage1DemoWidget::onRenderModeChanged);
    connect(m_pointSizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &Stage1DemoWidget::onPointSizeChanged);
    connect(m_renderButton, &QPushButton::clicked, this, &Stage1DemoWidget::renderTopDownView);
    connect(m_saveImageButton, &QPushButton::clicked, this, &Stage1DemoWidget::saveRenderResult);
}

// 性能测试组已删除

void Stage1DemoWidget::createDisplayArea()
{
    // 创建渲染结果标题 - 优化字体和样式
    QLabel* titleLabel = new QLabel("渲染结果");
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-weight: bold;"
        "   font-size: 16px;"
        "   color: #2c3e50;"
        "   padding: 8px 0;"
        "   margin-bottom: 8px;"
        "}"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    m_displayLayout->addWidget(titleLabel);

    // 渲染显示区域 - 完全响应式设计
    m_renderDisplayLabel = new QLabel();
    m_renderDisplayLabel->setMinimumSize(400, 300);      // 设置合理的最小尺寸
    m_renderDisplayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_renderDisplayLabel->setScaledContents(false);      // 禁止自动缩放，避免二次缩放导致坐标错乱

    // 启用鼠标跟踪和事件处理（用于线段绘制）
    m_renderDisplayLabel->setMouseTracking(true);
    setMouseTracking(true);
    m_renderDisplayLabel->setStyleSheet(
        "QLabel {"
        "   border: 3px solid #bdc3c7;"
        "   background-color: #ffffff;"
        "   border-radius: 8px;"
        "   padding: 20px;"
        "   font-size: 14px;"
        "   color: #34495e;"
        "   font-weight: 500;"
        "}"
        "QLabel:hover {"
        "   border-color: #85929e;"
        "}"
    );
    m_renderDisplayLabel->setAlignment(Qt::AlignCenter);
    m_renderDisplayLabel->setText("点击渲染俯视图按钮开始渲染\n\n支持的操作：\n• 加载点云文件\n• 生成测试数据\n• 调整渲染参数\n• 保存渲染结果");
    m_renderDisplayLabel->setWordWrap(true);

    m_displayLayout->addWidget(m_renderDisplayLabel, 1); // 给予更多空间
}

// 文件操作槽函数
void Stage1DemoWidget::loadPointCloudFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Load Point Cloud File",
        "",
        "Point Cloud Files (*.pcd *.las *.laz *.ply *.xyz *.txt);;All Files (*.*)");

    if (fileName.isEmpty()) {
        return;
    }

    // 显示加载状态
    QApplication::processEvents();

    try {
        QElapsedTimer timer;
        timer.start();

        // 根据文件扩展名选择合适的读取器
        QString ext = QFileInfo(fileName).suffix().toLower();
        qDebug() << "Loading file with extension:" << ext;

        if (ext == "las" || ext == "laz") {
            // LAS/LAZ文件处理
            WallExtraction::LASReader reader;
            if (reader.canReadFile(fileName)) {
                m_currentPointCloud = reader.readPointCloudWithAttributes(fileName);
                m_currentSimpleCloud.clear();
                for (const auto& point : m_currentPointCloud) {
                    m_currentSimpleCloud.push_back(point.position);
                }
                qDebug() << "Successfully loaded LAS/LAZ file with" << m_currentPointCloud.size() << "points";
            } else {
                throw std::runtime_error("Failed to read LAS/LAZ file");
            }
        } else if (ext == "pcd") {
            // PCD文件处理
            qDebug() << "Loading PCD file:" << fileName;
            std::vector<QVector3D> simplePoints = PCDReader::ReadVec3PointCloudPCD(fileName);
            if (simplePoints.empty()) {
                throw std::runtime_error("Failed to read PCD file or file is empty");
            }

            // 转换为带属性的点云格式，并进行数据验证
            m_currentPointCloud.clear();
            m_currentSimpleCloud.clear();
            m_currentPointCloud.reserve(simplePoints.size());
            m_currentSimpleCloud.reserve(simplePoints.size());

            qDebug() << "=== Data Validation and Cleaning ===";
            int validPoints = 0;
            int invalidPoints = 0;

            for (size_t i = 0; i < simplePoints.size(); ++i) {
                const QVector3D& pos = simplePoints[i];

                // 数据有效性检查
                bool isValid = true;

                // 检查是否为无穷大或NaN
                if (!std::isfinite(pos.x()) || !std::isfinite(pos.y()) || !std::isfinite(pos.z())) {
                    isValid = false;
                }

                // 检查是否在合理范围内（假设点云在±1000米范围内）
                const float MAX_COORD = 1000.0f;
                if (qAbs(pos.x()) > MAX_COORD || qAbs(pos.y()) > MAX_COORD || qAbs(pos.z()) > MAX_COORD) {
                    isValid = false;
                }

                if (isValid) {
                    WallExtraction::PointWithAttributes point;
                    point.position = pos;

                    // 为PCD点云生成基本属性
                    point.attributes["intensity"] = static_cast<int>(point.position.z() * 1000);
                    point.attributes["classification"] = (point.position.z() > 0) ? 6 : 2;
                    point.attributes["red"] = static_cast<int>((point.position.z() / 10.0f) * 65535);
                    point.attributes["green"] = 65535 - point.attributes["red"].toInt();
                    point.attributes["blue"] = (point.attributes["red"].toInt() + 32767) % 65535;

                    m_currentPointCloud.push_back(point);
                    m_currentSimpleCloud.push_back(pos);
                    validPoints++;
                } else {
                    invalidPoints++;
                    if (invalidPoints <= 5) {
                        qDebug() << "Invalid point" << i << ":" << pos;
                    }
                }
            }

            qDebug() << "Data validation completed:";
            qDebug() << "  Valid points:" << validPoints;
            qDebug() << "  Invalid points:" << invalidPoints;
            qDebug() << "  Validation rate:" << (validPoints * 100.0 / simplePoints.size()) << "%";

            if (validPoints == 0) {
                qDebug() << "WARNING: No valid points found in PCD file - generating test data instead";
                QMessageBox::warning(this, "数据损坏",
                    QString("PCD文件数据损坏（包含无效坐标）\n"
                           "将生成测试数据进行演示\n\n"
                           "原始文件：%1\n"
                           "无效点数：%2")
                    .arg(fileName)
                    .arg(invalidPoints));

                generateValidTestData(50000); // 生成5万个测试点
            } else {
                qDebug() << "Successfully loaded PCD file with" << m_currentPointCloud.size() << "valid points";
            }
        } else if (ext == "ply") {
            // PLY文件处理
            qDebug() << "Loading PLY file:" << fileName;

            // 使用PointCloudProcessor读取PLY文件
            WallExtraction::PointCloudProcessor processor;
            if (!processor.canReadFile(fileName)) {
                throw std::runtime_error("Cannot read PLY file");
            }

            std::vector<QVector3D> simplePoints = processor.readPointCloud(fileName);
            if (simplePoints.empty()) {
                throw std::runtime_error("Failed to read PLY file or file is empty");
            }

            // 转换为带属性的点云格式，并进行数据验证
            m_currentPointCloud.clear();
            m_currentSimpleCloud.clear();
            m_currentPointCloud.reserve(simplePoints.size());
            m_currentSimpleCloud.reserve(simplePoints.size());

            qDebug() << "=== PLY Data Validation and Cleaning ===";
            int validPoints = 0;
            int invalidPoints = 0;

            for (size_t i = 0; i < simplePoints.size(); ++i) {
                const QVector3D& pos = simplePoints[i];

                // 数据有效性检查
                bool isValid = true;

                // 检查是否为无穷大或NaN
                if (!std::isfinite(pos.x()) || !std::isfinite(pos.y()) || !std::isfinite(pos.z())) {
                    isValid = false;
                }

                // 检查是否在合理范围内
                const float MAX_COORD = 1000.0f;
                if (qAbs(pos.x()) > MAX_COORD || qAbs(pos.y()) > MAX_COORD || qAbs(pos.z()) > MAX_COORD) {
                    isValid = false;
                }

                if (isValid) {
                    WallExtraction::PointWithAttributes point;
                    point.position = pos;

                    // 为PLY点云生成基本属性
                    point.attributes["intensity"] = static_cast<int>(point.position.z() * 1000);
                    point.attributes["classification"] = (point.position.z() > 0) ? 6 : 2;
                    point.attributes["red"] = static_cast<int>((point.position.z() / 10.0f) * 65535);
                    point.attributes["green"] = 65535 - point.attributes["red"].toInt();
                    point.attributes["blue"] = (point.attributes["red"].toInt() + 32767) % 65535;

                    m_currentPointCloud.push_back(point);
                    m_currentSimpleCloud.push_back(pos);
                    validPoints++;
                } else {
                    invalidPoints++;
                }
            }

            qDebug() << "PLY validation completed: Valid=" << validPoints << "Invalid=" << invalidPoints;

            if (validPoints == 0) {
                throw std::runtime_error("No valid points found in PLY file - data may be corrupted");
            }
            qDebug() << "Successfully loaded PLY file with" << m_currentPointCloud.size() << "points";
        } else if (ext == "xyz" || ext == "txt") {
            // XYZ/TXT文件处理
            qDebug() << "Loading XYZ/TXT file:" << fileName;

            WallExtraction::PointCloudProcessor processor;
            if (!processor.canReadFile(fileName)) {
                throw std::runtime_error("Cannot read XYZ/TXT file");
            }

            std::vector<QVector3D> simplePoints = processor.readPointCloud(fileName);
            if (simplePoints.empty()) {
                throw std::runtime_error("Failed to read XYZ/TXT file or file is empty");
            }

            // 转换为带属性的点云格式
            m_currentPointCloud.clear();
            m_currentSimpleCloud = simplePoints;
            m_currentPointCloud.reserve(simplePoints.size());

            for (size_t i = 0; i < simplePoints.size(); ++i) {
                WallExtraction::PointWithAttributes point;
                point.position = simplePoints[i];

                // 为XYZ/TXT点云生成基本属性
                point.attributes["intensity"] = static_cast<int>(point.position.z() * 1000);
                point.attributes["classification"] = (point.position.z() > 0) ? 6 : 2;
                point.attributes["red"] = static_cast<int>((point.position.z() / 10.0f) * 65535);
                point.attributes["green"] = 65535 - point.attributes["red"].toInt();
                point.attributes["blue"] = (point.attributes["red"].toInt() + 32767) % 65535;

                m_currentPointCloud.push_back(point);
            }
            qDebug() << "Successfully loaded XYZ/TXT file with" << m_currentPointCloud.size() << "points";
        } else {
            // 不支持的格式
            throw std::runtime_error(QString("Unsupported file format: %1").arg(ext).toStdString());
        }

        timer.elapsed(); // 记录加载时间
        m_currentFileName = fileName;

        // 清除之前的线段标注数据（新点云应该有独立的标注）
        clearLineSegmentData();

        processLoadedPointCloud();

        // 文件加载成功
        m_fileInfoLabel->setText(QString("File: %1 (%2 points)")
                                .arg(QFileInfo(fileName).baseName())
                                .arg(m_currentPointCloud.size()));

        qDebug() << "Point cloud file loaded successfully, previous line segment data cleared";

    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString("Failed to load file: %1").arg(e.what()));
        // 错误已通过消息框显示
    }
}

void Stage1DemoWidget::generateTestData()
{
    qDebug() << "=== Generating Test Data ===";

    // 清除之前的线段标注数据
    clearLineSegmentData();

    int pointCount = 25000; // 生成25K点的测试数据
    generateSampleData(pointCount);

    m_currentFileName = "Generated Test Data";
    processLoadedPointCloud();

    // 测试数据生成完成
    m_fileInfoLabel->setText(QString("Generated: %1 points").arg(pointCount));

    qDebug() << "Test data generated, previous line segment data cleared";
}

void Stage1DemoWidget::clearPointCloud()
{
    qDebug() << "=== Clearing Point Cloud and Associated Data ===";

    // 设置清除标志，防止异步渲染
    m_isClearing = true;

    // 清除点云数据
    m_currentPointCloud.clear();
    m_currentSimpleCloud.clear();
    m_currentFileName.clear();
    qDebug() << "Point cloud data cleared";

    // 完全清除UI显示
    m_fileInfoLabel->setText("No file loaded");

    // 彻底清除渲染显示
    m_renderDisplayLabel->clear(); // 清除所有内容
    m_renderDisplayLabel->setPixmap(QPixmap()); // 设置空pixmap
    m_renderDisplayLabel->setText("No render result"); // 设置提示文本

    // 强制立即更新显示
    m_renderDisplayLabel->update();
    m_renderDisplayLabel->repaint();
    qDebug() << "UI display cleared";

    // 重置统计信息
    m_stats = Statistics();

    // 清除线段标注数据
    clearLineSegmentData();

    // 完全重置渲染器状态
    if (m_renderer) {
        m_renderer->clearRenderBuffer();

        // 重置渲染器的视图边界和视口大小
        m_renderer->setViewBounds(-100.0f, 100.0f, -100.0f, 100.0f);
        m_renderer->setViewportSize(QSize(800, 600));

        qDebug() << "Renderer buffer cleared and state reset";
    }

    // 清除LOD数据
    if (m_lodManager) {
        m_lodManager->clearLODData();
        qDebug() << "LOD data cleared";
    }

    // 清除颜色映射缓存
    if (m_colorMapper) {
        // ColorMappingManager没有公开的clear方法，但会在下次使用时自动重置
        qDebug() << "Color mapping will be reset on next use";
    }

    // 清除内存管理器数据
    if (m_memoryManager) {
        m_memoryManager->clearAllData();
        qDebug() << "Memory manager data cleared";
    }

    // 重置坐标映射缓存
    m_lastScaledPixmapSize = QSize();
    m_lastPixmapTopLeft = QPoint();
    m_currentViewportSize = QSize();
    m_currentViewBounds = QRectF();

    // 重置UI控件状态
    if (m_lodLevelSlider) {
        m_lodLevelSlider->setValue(0);
    }
    if (m_lodInfoLabel) {
        m_lodInfoLabel->setText("LOD not generated");
    }

    // 强制刷新整个界面，确保清除操作立即生效
    this->update();
    this->repaint();
    QApplication::processEvents(); // 处理所有待处理的事件

    // 清除标志重置，允许后续正常渲染
    m_isClearing = false;

    qDebug() << "Point cloud and all associated data cleared completely";
    qDebug() << "Interface forcefully refreshed to ensure immediate effect";
    qDebug() << "Clearing flag reset, normal rendering can resume";
}

// LOD控制槽函数
void Stage1DemoWidget::onLODLevelChanged(int level)
{
    QString levelText = QString("Level %1 (%2%)")
                       .arg(level)
                       .arg(100 >> level); // 每级减少50%
    m_lodLevelLabel->setText(levelText);

    if (m_lodManager->getLODLevelCount() > 0) {
        updateLODDisplay();
    }
}

void Stage1DemoWidget::onLODStrategyChanged(int strategy)
{
    Q_UNUSED(strategy)
    // LOD策略已更改
}

void Stage1DemoWidget::generateLODLevels()
{
    if (m_currentSimpleCloud.empty()) {
        QMessageBox::information(this, "Info", "Please load point cloud data first");
        return;
    }

    // 生成LOD级别
    QApplication::processEvents();

    QElapsedTimer timer;
    timer.start();

    // 设置LOD策略（简化实现）
    // 注意：实际的LOD策略设置需要根据具体的API实现

    bool success = m_lodManager->generateLODLevels(m_currentSimpleCloud);
    qint64 lodTime = timer.elapsed();

    if (success) {
        m_stats.lodLevels = m_lodManager->getLODLevelCount();
        m_lodInfoLabel->setText(QString("Generated %1 LOD levels in %2 ms")
                               .arg(m_stats.lodLevels)
                               .arg(lodTime));
        // LOD生成完成

        updateLODDisplay();
    } else {
        QMessageBox::warning(this, "Error", "Failed to generate LOD levels");
        // LOD生成失败
    }
}

// 颜色映射控制槽函数
void Stage1DemoWidget::onColorSchemeChanged(int scheme)
{
    WallExtraction::ColorScheme colorScheme =
        static_cast<WallExtraction::ColorScheme>(scheme);
    m_colorMapper->setColorScheme(colorScheme);

    qDebug() << "Color scheme changed to:" << scheme;

    // 自动更新值范围
    if (!m_currentPointCloud.empty()) {
        m_colorMapper->autoCalculateValueRange(m_currentPointCloud);
        auto range = m_colorMapper->getValueRange();
        m_minValueSpin->setValue(range.first);
        m_maxValueSpin->setValue(range.second);
        qDebug() << "Updated value range:" << range.first << "to" << range.second;
    }

    generateColorBar();

    // 重新渲染点云以应用新的颜色映射
    if (!m_currentPointCloud.empty()) {
        updateTopDownView();
        qDebug() << "Triggered re-rendering with new color scheme";
    }
}

void Stage1DemoWidget::onColorRangeChanged()
{
    float minVal = m_minValueSpin->value();
    float maxVal = m_maxValueSpin->value();

    if (minVal >= maxVal) {
        qDebug() << "Invalid range: min" << minVal << ">= max" << maxVal;
        return;
    }

    qDebug() << "Color range changed to:" << minVal << "-" << maxVal;
    m_colorMapper->setValueRange(minVal, maxVal);
    generateColorBar();

    // 重新渲染点云以应用新的数值范围
    if (!m_currentPointCloud.empty()) {
        updateTopDownView();
        qDebug() << "Triggered re-rendering with new value range";
    }
}

void Stage1DemoWidget::generateColorBar()
{
    QImage colorBar = m_colorMapper->generateColorBar(200, 20);
    if (!colorBar.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(colorBar);
        m_colorBarLabel->setPixmap(pixmap.scaled(m_colorBarLabel->size(), Qt::KeepAspectRatio));
    }
}

// 渲染控制槽函数
void Stage1DemoWidget::onRenderModeChanged(int mode)
{
    WallExtraction::TopDownRenderMode renderMode =
        static_cast<WallExtraction::TopDownRenderMode>(mode);
    m_renderer->setRenderMode(renderMode);

    // 渲染模式已更改
}

void Stage1DemoWidget::onPointSizeChanged(double size)
{
    m_renderer->setPointSize(static_cast<float>(size));
}

void Stage1DemoWidget::renderTopDownView()
{
    // 如果正在清除过程中，阻止渲染
    if (m_isClearing) {
        qDebug() << "Clearing in progress, skipping renderTopDownView";
        return;
    }

    if (m_currentPointCloud.empty()) {
        QMessageBox::information(this, "Info", "Please load point cloud data first");
        return;
    }

    qDebug() << "=== Starting Top-Down View Rendering ===";
    qDebug() << "Point cloud size:" << m_currentPointCloud.size();

    // 大数据量检测和处理
    const size_t MAX_RENDER_POINTS = 500000; // 50万个点的渲染限制
    std::vector<WallExtraction::PointWithAttributes> renderPoints;

    if (m_currentPointCloud.size() > MAX_RENDER_POINTS) {
        qDebug() << "Large point cloud detected (" << m_currentPointCloud.size() << " points)";
        qDebug() << "Applying intelligent sampling to" << MAX_RENDER_POINTS << "points";

        // 智能采样
        renderPoints = performIntelligentSampling(m_currentPointCloud, MAX_RENDER_POINTS);

        // 显示采样信息给用户
        QMessageBox::information(this, "大数据量处理",
            QString("检测到大点云文件（%1个点）\n"
                   "为了确保渲染性能，已自动采样到%2个点\n"
                   "如需处理完整数据，请先生成LOD级别")
            .arg(m_currentPointCloud.size())
            .arg(renderPoints.size()));
    } else {
        renderPoints = m_currentPointCloud;
    }

    qDebug() << "Rendering point count:" << renderPoints.size();

    // 验证点云数据
    validatePointCloudAttributes();

    // 调试渲染管道
    debugRenderingPipeline();

    // 渲染俯视图
    QApplication::processEvents();

    // 动态计算渲染区域大小
    QSize renderSize = calculateOptimalRenderSize();
    m_renderer->setViewportSize(renderSize);
    m_currentViewportSize = renderSize; // 保存用于坐标一致性
    qDebug() << "Render viewport size:" << renderSize;

    // 动态计算视图边界
    QRectF viewBounds = calculatePointCloudBounds();
    m_renderer->setViewBounds(viewBounds.left(), viewBounds.right(),
                             viewBounds.top(), viewBounds.bottom());
    m_currentViewBounds = viewBounds; // 保存用于坐标一致性
    qDebug() << "View bounds:" << viewBounds;
    qDebug() << "View bounds details - left:" << viewBounds.left() << "right:" << viewBounds.right()
             << "top:" << viewBounds.top() << "bottom:" << viewBounds.bottom()
             << "width:" << viewBounds.width() << "height:" << viewBounds.height();

    // 优化颜色映射
    optimizeColorMappingForTopDown();

    QElapsedTimer timer;
    timer.start();

    bool success = false;
    if (m_lodManager->getLODLevelCount() > 0) {
        // 使用LOD数据渲染 - 注意：LOD返回的是QVector3D类型，需要转换
        int lodLevel = m_lodLevelSlider->value();
        auto lodPoints = m_lodManager->getLODPoints(lodLevel);

        // 为了支持完整的颜色映射，我们需要从原始数据中提取对应的属性
        // 这里暂时使用QVector3D版本，但只支持高度映射
        success = m_renderer->renderTopDownView(lodPoints);
        qDebug() << "Rendering with LOD level" << lodLevel << "(" << lodPoints.size() << "points) - Limited to height mapping";
    } else {
        // 使用采样后的数据渲染 - 这里使用PointWithAttributes版本，支持所有颜色方案
        success = m_renderer->renderTopDownView(renderPoints);
        qDebug() << "Rendering with sampled PointWithAttributes data (" << renderPoints.size() << "points) - Full color mapping support";
    }

    qint64 renderTime = timer.elapsed();

    if (success) {
        QImage renderResult = m_renderer->getRenderBuffer();
        qDebug() << "Render result size:" << renderResult.size();
        qDebug() << "Render result format:" << renderResult.format();
        qDebug() << "Render result null:" << renderResult.isNull();

        if (!renderResult.isNull() && !renderResult.size().isEmpty()) {
            // 确保图像不为空且有有效尺寸
            QPixmap pixmap = QPixmap::fromImage(renderResult);

            // 在点云渲染结果上绘制线段
            pixmap = drawLineSegmentsOnPixmap(pixmap);

            // 获取显示标签的当前尺寸
            QSize labelSize = m_renderDisplayLabel->size();
            qDebug() << "Display label size:" << labelSize;

            if (labelSize.width() > 0 && labelSize.height() > 0) {
                // 计算在label中居中显示的尺寸（保持等比）
                QSize pixmapSize = pixmap.size();
                QSize targetSize = pixmapSize;
                if (pixmapSize.width() > labelSize.width() || pixmapSize.height() > labelSize.height()) {
                    targetSize = pixmapSize.scaled(labelSize, Qt::KeepAspectRatio);
                }

                QPixmap scaledPixmap = pixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                m_renderDisplayLabel->setPixmap(scaledPixmap);
                m_renderDisplayLabel->setText("");

                // 记录缩放后尺寸与偏移（label可能比图像大，图像会居中显示）
                m_lastScaledPixmapSize = scaledPixmap.size();
                int offsetX = (labelSize.width() - m_lastScaledPixmapSize.width()) / 2;
                int offsetY = (labelSize.height() - m_lastScaledPixmapSize.height()) / 2;
                m_lastPixmapTopLeft = QPoint(offsetX, offsetY);
                qDebug() << "Scaled pixmap size:" << m_lastScaledPixmapSize << "top-left offset:" << m_lastPixmapTopLeft;

                // 强制立即更新显示
                m_renderDisplayLabel->update();
                m_renderDisplayLabel->repaint();

                qDebug() << "Successfully displayed render result with line segments";
                qDebug() << "Original size:" << pixmap.size() << "-> Scaled size:" << scaledPixmap.size();
            } else {
                qDebug() << "Display label has invalid size, using original image";
                m_renderDisplayLabel->setPixmap(pixmap);
                m_renderDisplayLabel->setText("");
            }
        } else {
            qDebug() << "Render result is null or empty";
            m_renderDisplayLabel->setText("渲染结果为空\n请检查点云数据和渲染参数");
        }

        m_stats.lastRenderTime = renderTime;
        m_stats.fps = 1000.0 / qMax(renderTime, qint64(1));

        qDebug() << "Rendering completed successfully in" << renderTime << "ms";
    } else {
        qDebug() << "Rendering failed";
        m_renderDisplayLabel->setText("渲染失败\n请检查点云数据和渲染设置");

        // 详细的错误诊断
        QString errorDetails = QString("俯视图渲染失败\n\n详细信息：\n"
                                     "• 原始点云大小：%1 个点\n"
                                     "• 渲染点云大小：%2 个点\n"
                                     "• 渲染模式：%3\n"
                                     "• 点大小：%4\n"
                                     "• 视口大小：%5\n\n"
                                     "可能的原因：\n"
                                     "• 投影变换失败\n"
                                     "• 视锥体剔除过度\n"
                                     "• 渲染缓冲区问题\n"
                                     "• 内存不足\n\n"
                                     "建议解决方案：\n"
                                     "• 尝试生成LOD级别\n"
                                     "• 调整渲染参数\n"
                                     "• 检查点云数据有效性")
                              .arg(m_currentPointCloud.size())
                              .arg(renderPoints.size())
                              .arg(static_cast<int>(m_renderer->getRenderMode()))
                              .arg(m_renderer->getPointSize())
                              .arg(QString("%1x%2").arg(m_renderer->getViewportSize().width())
                                                   .arg(m_renderer->getViewportSize().height()));

        QMessageBox::warning(this, "渲染错误", errorDetails);
    }
}

void Stage1DemoWidget::saveRenderResult()
{
    QImage renderResult = m_renderer->getRenderBuffer();
    if (renderResult.isNull()) {
        QMessageBox::information(this, "Info", "No render result to save");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Render Result",
        "render_result.png",
        "PNG Files (*.png);;JPEG Files (*.jpg);;All Files (*.*)");

    if (!fileName.isEmpty()) {
        if (renderResult.save(fileName)) {
            QMessageBox::information(this, "保存成功", QString("渲染结果已保存到: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, "Error", "Failed to save render result");
        }
    }
}

// 性能测试方法已删除

// 内存测试方法已删除

// 空间索引测试方法已删除

// 统计更新方法已删除

void Stage1DemoWidget::updateRenderView()
{
    qDebug() << "=== updateRenderView called ===";
    qDebug() << "Point cloud size:" << m_currentPointCloud.size();
    qDebug() << "Is clearing:" << m_isClearing;

    // 如果正在清除过程中，阻止任何渲染操作
    if (m_isClearing) {
        qDebug() << "Clearing in progress, skipping render update";
        return;
    }

    if (!m_currentPointCloud.empty()) {
        qDebug() << "Calling renderTopDownView to update display";
        renderTopDownView();
    } else {
        qDebug() << "Point cloud is empty - clearing display label pixmap";

        // 完全清除显示内容
        m_renderDisplayLabel->clear(); // 清除所有内容
        m_renderDisplayLabel->setPixmap(QPixmap()); // 设置空pixmap
        m_renderDisplayLabel->setText("No render result"); // 设置提示文本

        // 重置坐标映射缓存
        m_lastScaledPixmapSize = QSize();
        m_lastPixmapTopLeft = QPoint();
        m_currentViewportSize = QSize();
        m_currentViewBounds = QRectF();

        // 强制立即更新显示
        m_renderDisplayLabel->update();
        m_renderDisplayLabel->repaint();

        qDebug() << "Display completely cleared and updated";
    }
}

QPixmap Stage1DemoWidget::drawLineSegmentsOnPixmap(const QPixmap& basePixmap)
{
    qDebug() << "=== drawLineSegmentsOnPixmap ===";
    qDebug() << "Base pixmap size:" << basePixmap.size();
    qDebug() << "Current view bounds:" << m_currentViewBounds;
    qDebug() << "Current viewport size:" << m_currentViewportSize;
    qDebug() << "ViewBounds details - left:" << m_currentViewBounds.left() << "right:" << m_currentViewBounds.right()
             << "top:" << m_currentViewBounds.top() << "bottom:" << m_currentViewBounds.bottom();

    // 创建一个可编辑的副本
    QPixmap result = basePixmap;

    // 检查LineDrawingTool是否可用
    if (!m_wallManager || !m_wallManager->getLineDrawingTool()) {
        qDebug() << "LineDrawingTool not available for line segment rendering";
        return result;
    }

    auto lineDrawingTool = m_wallManager->getLineDrawingTool();
    auto lineSegments = lineDrawingTool->getLineSegments();

    if (lineSegments.empty()) {
        qDebug() << "No line segments to draw";
        return result;
    }

    // 不再强制依赖 m_currentViewBounds 的有效性；
    // 统一用 ViewProjectionManager 的矩阵进行转换即可。

    qDebug() << "Drawing" << lineSegments.size() << "line segments on pixmap";

    // 创建QPainter在pixmap上绘制
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 设置线段绘制样式 - 改为白色
    QPen linePen(QColor(255, 255, 255), 4); // 白色，4像素宽，更明显
    painter.setPen(linePen);

    // 获取pixmap尺寸用于坐标转换
    QSize pixmapSize = basePixmap.size();
    qDebug() << "Pixmap size for coordinate conversion:" << pixmapSize;

    // 检查渲染器是否可用，使用渲染器的坐标系统
    if (!m_renderer) {
        qDebug() << "ERROR: Renderer not available for coordinate conversion";
        return result;
    }

    // 获取渲染器的ViewProjectionManager - 这是关键！
    auto projectionManager = m_renderer->getProjectionManager();
    if (!projectionManager) {
        qDebug() << "ERROR: ViewProjectionManager not available";
        return result;
    }

    // 确保ViewProjectionManager使用与pixmap相同的视口大小
    projectionManager->setViewportSize(pixmapSize);

    // 确保ViewProjectionManager使用与点云渲染相同的视图边界
    if (!m_currentViewBounds.isEmpty()) {
        projectionManager->setViewBounds(m_currentViewBounds);
        qDebug() << "Set projection manager bounds to:" << m_currentViewBounds;
    } else {
        qWarning() << "Current view bounds is empty, using default";
        // 使用实际点云边界作为备用
        QRectF actualBounds = getActualPointCloudBounds();
        if (!actualBounds.isEmpty()) {
            projectionManager->setViewBounds(actualBounds);
            qDebug() << "Using actual point cloud bounds:" << actualBounds;
        }
    }

    // 调试：验证 ProjectionManager 的设置
    qDebug() << "Final ProjectionManager settings:";
    qDebug() << "  ViewportSize:" << projectionManager->getViewportSize();
    qDebug() << "  ViewBounds:" << projectionManager->getViewBounds();

    // 绘制每个线段
    int segmentIndex = 0;
    for (const auto& segment : lineSegments) {
        qDebug() << "--- Segment" << segmentIndex << "---";
        qDebug() << "World start point:" << segment.startPoint;
        qDebug() << "World end point:" << segment.endPoint;

        // 转换为3D世界坐标（Z=0用于俯视图）
        QVector3D worldStart(segment.startPoint.x(), segment.startPoint.y(), 0);
        QVector3D worldEnd(segment.endPoint.x(), segment.endPoint.y(), 0);

        qDebug() << "Converting world coordinates using ViewProjectionManager";
        qDebug() << "World start:" << worldStart;
        qDebug() << "World end:" << worldEnd;

        // ViewProjectionManager 有问题，直接使用手动转换
        QRectF bounds = projectionManager->getViewBounds();
        qDebug() << "Using manual coordinate conversion";
        qDebug() << "World coordinates - start:" << worldStart << "end:" << worldEnd;
        qDebug() << "View bounds:" << bounds;
        qDebug() << "Pixmap size:" << pixmapSize;

        // 手动坐标转换：世界坐标 -> 标准化坐标 -> 像素坐标
        float normalizedStartX = (worldStart.x() - bounds.left()) / bounds.width();
        float normalizedStartY = (worldStart.y() - bounds.top()) / bounds.height();
        QVector2D startScreen(normalizedStartX * pixmapSize.width(),
                             normalizedStartY * pixmapSize.height());

        float normalizedEndX = (worldEnd.x() - bounds.left()) / bounds.width();
        float normalizedEndY = (worldEnd.y() - bounds.top()) / bounds.height();
        QVector2D endScreen(normalizedEndX * pixmapSize.width(),
                           normalizedEndY * pixmapSize.height());

        qDebug() << "Manual conversion:";
        qDebug() << "  Start normalized:" << normalizedStartX << normalizedStartY << "-> pixels:" << startScreen;
        qDebug() << "  End normalized:" << normalizedEndX << normalizedEndY << "-> pixels:" << endScreen;

        // 检查坐标是否在合理范围内
        bool coordsOutOfBounds = (startScreen.x() < 0 || startScreen.x() > pixmapSize.width() ||
                                 startScreen.y() < 0 || startScreen.y() > pixmapSize.height() ||
                                 endScreen.x() < 0 || endScreen.x() > pixmapSize.width() ||
                                 endScreen.y() < 0 || endScreen.y() > pixmapSize.height());

        if (coordsOutOfBounds) {
            qDebug() << "WARNING: Line segment coordinates are outside pixmap bounds!";
            qDebug() << "Pixmap size:" << pixmapSize << "Start:" << startScreen << "End:" << endScreen;
            // 调试阶段：改为夹取到可见范围，而不是跳过
            auto clampPoint = [&](const QPointF& p) -> QPointF {
                double cx = qBound(0.0, static_cast<double>(p.x()), static_cast<double>(pixmapSize.width() - 1));
                double cy = qBound(0.0, static_cast<double>(p.y()), static_cast<double>(pixmapSize.height() - 1));
                return QPointF(cx, cy);
            };
            QPointF startClamped = clampPoint(QPointF(startScreen.x(), startScreen.y()));
            QPointF endClamped = clampPoint(QPointF(endScreen.x(), endScreen.y()));
            qDebug() << "Clamped to:" << startClamped << "->" << endClamped;
            painter.drawLine(startClamped, endClamped);
            painter.setBrush(QBrush(QColor(255, 128, 0))); // 橙色端点，表示被夹取
            painter.drawEllipse(startClamped, 4, 4);
            painter.drawEllipse(endClamped, 4, 4);
            painter.setBrush(QBrush());
            segmentIndex++;
            continue;
        }

        // 绘制线段 - 转换QVector2D到QPointF
        QPointF startPoint(startScreen.x(), startScreen.y());
        QPointF endPoint(endScreen.x(), endScreen.y());

        painter.drawLine(startPoint, endPoint);

        // 绘制端点（小圆圈）
        painter.setBrush(QBrush(QColor(255, 255, 0))); // 黄色端点
        painter.drawEllipse(startPoint, 4, 4);
        painter.drawEllipse(endPoint, 4, 4);
        painter.setBrush(QBrush()); // 清除画刷

        qDebug() << "Drew line segment from" << startPoint << "to" << endPoint;
        segmentIndex++;
    }

    painter.end();

    qDebug() << "Finished drawing line segments";
    return result;
}

// 辅助方法：将世界坐标转换为屏幕坐标（与LineDrawingTool保持一致）
QPointF Stage1DemoWidget::worldToScreen(const QVector3D& worldPos, const QSize& screenSize,
                                       float worldMin, float worldMax) const
{
    // 使用与LineDrawingTool相同的坐标转换逻辑
    // LineDrawingTool使用±50的世界坐标范围

    qDebug() << "Converting world pos:" << worldPos << "to screen size:" << screenSize;

    // 简化的屏幕坐标转换（与LineDrawingTool::worldToScreen保持一致）
    float width = screenSize.width();
    float height = screenSize.height();

    if (width <= 0 || height <= 0) {
        qDebug() << "Invalid screen size, returning original coordinates";
        return QPointF(worldPos.x(), worldPos.y());
    }

    // 使用与LineDrawingTool相同的转换公式
    float x = (worldPos.x() / 50.0f + 1.0f) * width * 0.5f;
    float y = (1.0f - worldPos.y() / 50.0f) * height * 0.5f;

    QPointF result(x, y);
    qDebug() << "Converted to screen pos:" << result;

    return result;
}

// 简化的世界坐标到屏幕坐标转换（备用方案）
QPointF Stage1DemoWidget::simplifiedWorldToScreen(const QVector3D& worldPos, const QSize& screenSize) const
{
    qDebug() << "Using simplified coordinate conversion for:" << worldPos;

    // 使用与LineDrawingTool相同的简化转换逻辑
    float width = screenSize.width();
    float height = screenSize.height();

    if (width <= 0 || height <= 0) {
        qDebug() << "Invalid screen size, returning original coordinates";
        return QPointF(worldPos.x(), worldPos.y());
    }

    // 使用与LineDrawingTool相同的转换公式（±50的世界坐标范围）
    float x = (worldPos.x() / 50.0f + 1.0f) * width * 0.5f;
    float y = (1.0f - worldPos.y() / 50.0f) * height * 0.5f;

    QPointF result(x, y);
    qDebug() << "Simplified conversion result:" << result;

    return result;
}

// 获取实际点云的坐标范围
QRectF Stage1DemoWidget::getActualPointCloudBounds() const
{
    qDebug() << "=== getActualPointCloudBounds ===";

    if (m_currentPointCloud.empty()) {
        qDebug() << "Point cloud is empty, returning empty bounds";
        return QRectF();
    }

    // 计算点云的实际边界
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    // 遍历所有点以获得精确边界（对于线段标注精度很重要）
    for (const auto& point : m_currentPointCloud) {
        minX = std::min(minX, point.position.x());
        maxX = std::max(maxX, point.position.x());
        minY = std::min(minY, point.position.y());
        maxY = std::max(maxY, point.position.y());
    }

    // 不添加边距，使用精确边界以确保线段标注准确
    QRectF bounds(minX, minY, maxX - minX, maxY - minY);

    qDebug() << "Calculated actual bounds:" << bounds;
    qDebug() << "Point cloud size:" << m_currentPointCloud.size();

    return bounds;
}

// 精确的世界坐标到屏幕坐标转换（使用实际边界）
QPointF Stage1DemoWidget::accurateWorldToScreen(const QVector3D& worldPos, const QSize& screenSize, const QRectF& bounds) const
{
    qDebug() << "Accurate conversion - World:" << worldPos << "Bounds:" << bounds;

    float width = screenSize.width();
    float height = screenSize.height();

    if (width <= 0 || height <= 0 || bounds.width() <= 0 || bounds.height() <= 0) {
        qDebug() << "Invalid parameters for coordinate conversion";
        return QPointF(worldPos.x(), worldPos.y());
    }

    // 将世界坐标映射到屏幕坐标
    float normalizedX = (worldPos.x() - bounds.left()) / bounds.width();
    float normalizedY = (worldPos.y() - bounds.top()) / bounds.height();

    // 转换到屏幕坐标，Y轴翻转
    float screenX = normalizedX * width;
    float screenY = (1.0f - normalizedY) * height;

    QPointF result(screenX, screenY);
    qDebug() << "Accurate conversion result:" << result;

    return result;
}

// 精确的屏幕坐标到世界坐标转换（使用实际边界）
QVector3D Stage1DemoWidget::accurateScreenToWorld(const QVector2D& screenPoint, const QRectF& bounds) const
{
    qDebug() << "Accurate screen to world - Screen:" << screenPoint << "Bounds:" << bounds;

    // 使用渲染显示标签的尺寸
    QSize renderSize = m_renderDisplayLabel->size();
    float width = renderSize.width();
    float height = renderSize.height();

    if (width <= 0 || height <= 0 || bounds.width() <= 0 || bounds.height() <= 0) {
        qDebug() << "Invalid parameters for screen to world conversion";
        return QVector3D(screenPoint.x(), screenPoint.y(), 0.0f);
    }

    // 将屏幕坐标标准化到[0,1]范围
    float normalizedX = screenPoint.x() / width;
    float normalizedY = screenPoint.y() / height;

    // Y轴翻转（屏幕坐标Y向下，世界坐标Y向上）
    normalizedY = 1.0f - normalizedY;

    // 映射到世界坐标
    float worldX = bounds.left() + normalizedX * bounds.width();
    float worldY = bounds.top() + normalizedY * bounds.height();

    QVector3D result(worldX, worldY, 0.0f);
    qDebug() << "Accurate screen to world result:" << result;

    return result;
}

// 清除线段标注数据
void Stage1DemoWidget::clearLineSegmentData()
{
    qDebug() << "=== Clearing Line Segment Data ===";

    if (!m_wallManager) {
        qDebug() << "WallManager not available, skipping line segment data clearing";
        return;
    }

    auto lineDrawingTool = m_wallManager->getLineDrawingTool();
    if (!lineDrawingTool) {
        qDebug() << "LineDrawingTool not available, skipping line segment data clearing";
        return;
    }

    // 清除所有线段数据
    lineDrawingTool->clearAll();
    qDebug() << "All line segments cleared from LineDrawingTool";

    // 重置绘制状态
    lineDrawingTool->setDrawingMode(WallExtraction::DrawingMode::None);
    qDebug() << "Drawing mode reset to None";

    // 如果有线段绘制工具栏，也重置其状态
    if (m_lineDrawingToolbar) {
        // 工具栏会自动响应LineDrawingTool的状态变化
        qDebug() << "LineDrawingToolbar will update automatically";
    }

    qDebug() << "Line segment data clearing completed";
}

// 私有辅助方法
void Stage1DemoWidget::processLoadedPointCloud()
{
    if (m_currentPointCloud.empty()) {
        qDebug() << "ERROR: Point cloud is empty in processLoadedPointCloud()";
        return;
    }

    qDebug() << "=== Processing Loaded Point Cloud ===";
    qDebug() << "Point count:" << m_currentPointCloud.size();

    // 详细分析点云数据
    analyzePointCloudData();

    m_stats.pointCount = m_currentPointCloud.size();

    // 自动计算颜色映射范围
    m_colorMapper->autoCalculateValueRange(m_currentPointCloud);
    auto range = m_colorMapper->getValueRange();

    qDebug() << "Color mapping range:" << range.first << "to" << range.second;

    m_minValueSpin->setValue(range.first);
    m_maxValueSpin->setValue(range.second);

    // 生成颜色条
    generateColorBar();

    qDebug() << "Point cloud processing completed";

    // 重置LOD信息
    m_lodInfoLabel->setText("LOD not generated");
    m_lodLevelSlider->setValue(0);

    // 统计更新已删除
}

void Stage1DemoWidget::updatePointCloudInfo()
{
    if (m_currentPointCloud.empty()) {
        m_fileInfoLabel->setText("No file loaded");
        return;
    }

    QString info = QString("Points: %1").arg(m_currentPointCloud.size());
    if (!m_currentFileName.isEmpty()) {
        info = QString("File: %1 (%2 points)")
               .arg(QFileInfo(m_currentFileName).baseName())
               .arg(m_currentPointCloud.size());
    }

    m_fileInfoLabel->setText(info);
}

void Stage1DemoWidget::generateSampleData(int pointCount)
{
    qDebug() << "=== Generating Sample Data ===";
    qDebug() << "Requested point count:" << pointCount;

    m_currentPointCloud.clear();
    m_currentSimpleCloud.clear();

    m_currentPointCloud.reserve(pointCount);
    m_currentSimpleCloud.reserve(pointCount);

    // 生成更有意义的建筑物样式点云数据
    for (int i = 0; i < pointCount; ++i) {
        WallExtraction::PointWithAttributes point;

        // 创建更分散的坐标分布
        float x = (i % 200) * 0.5f - 50.0f;  // -50 到 50
        float y = ((i / 200) % 200) * 0.5f - 50.0f;  // -50 到 50

        // 创建更有变化的高度分布
        float baseHeight = 5.0f;
        float variation = qSin(x * 0.05f) * qCos(y * 0.05f) * 8.0f;
        float z = baseHeight + variation + (i % 10) * 0.5f;  // 0 到 18

        point.position = QVector3D(x, y, z);

        // 生成更合理的属性值
        point.attributes["intensity"] = static_cast<int>(z * 1000 + (i % 1000));  // 基于高度的强度
        point.attributes["classification"] = (z > 10.0f) ? 6 : 2;  // 建筑物或地面

        // 基于高度的RGB颜色
        int heightColor = static_cast<int>((z / 20.0f) * 65535);
        point.attributes["red"] = heightColor;
        point.attributes["green"] = 65535 - heightColor;
        point.attributes["blue"] = (heightColor + 32767) % 65535;

        m_currentPointCloud.push_back(point);
        m_currentSimpleCloud.push_back(point.position);
    }

    qDebug() << "Generated" << m_currentPointCloud.size() << "points";
    qDebug() << "Sample data generation completed";
}

void Stage1DemoWidget::updateTopDownView()
{
    if (!m_currentPointCloud.empty()) {
        renderTopDownView();
    }
}

QSize Stage1DemoWidget::calculateOptimalRenderSize() const
{
    // 获取显示区域的实际大小
    QSize displaySize = m_renderDisplayLabel->size();

    // 确保最小尺寸
    int minSize = 400;
    int width = qMax(minSize, displaySize.width());
    int height = qMax(minSize, displaySize.height());

    // 限制最大尺寸以保证性能
    int maxSize = 1200;
    width = qMin(maxSize, width);
    height = qMin(maxSize, height);

    qDebug() << "Display label size:" << displaySize << "-> Optimal render size:" << QSize(width, height);
    return QSize(width, height);
}

QRectF Stage1DemoWidget::calculatePointCloudBounds() const
{
    if (m_currentPointCloud.empty()) {
        return QRectF(-100, -100, 200, 200); // 默认边界
    }

    // 计算点云的实际边界
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (const auto& point : m_currentPointCloud) {
        minX = qMin(minX, point.position.x());
        maxX = qMax(maxX, point.position.x());
        minY = qMin(minY, point.position.y());
        maxY = qMax(maxY, point.position.y());
    }

    // 添加边距（10%）
    float rangeX = maxX - minX;
    float rangeY = maxY - minY;
    float marginX = rangeX * 0.1f;
    float marginY = rangeY * 0.1f;

    QRectF bounds(minX - marginX, minY - marginY,
                  rangeX + 2 * marginX, rangeY + 2 * marginY);

    qDebug() << "Point cloud bounds:" << bounds;
    qDebug() << "X range:" << minX << "to" << maxX << "(" << rangeX << ")";
    qDebug() << "Y range:" << minY << "to" << maxY << "(" << rangeY << ")";

    return bounds;
}

void Stage1DemoWidget::optimizeColorMappingForTopDown()
{
    if (!m_colorMapper || m_currentPointCloud.empty()) {
        return;
    }

    qDebug() << "=== Optimizing Color Mapping for Top-Down View ===";

    // 自动计算颜色映射范围
    m_colorMapper->autoCalculateValueRange(m_currentPointCloud);
    auto range = m_colorMapper->getValueRange();

    qDebug() << "Auto-calculated color range:" << range.first << "to" << range.second;

    // 更新UI中的数值范围显示
    if (m_minValueSpin && m_maxValueSpin) {
        m_minValueSpin->setValue(range.first);
        m_maxValueSpin->setValue(range.second);
        qDebug() << "Updated UI range controls";
    }

    // 生成新的颜色条
    generateColorBar();

    qDebug() << "Color mapping optimization completed";
}

void Stage1DemoWidget::analyzePointCloudData()
{
    if (m_currentPointCloud.empty()) {
        qDebug() << "WARNING: Cannot analyze empty point cloud";
        return;
    }

    qDebug() << "=== Point Cloud Data Analysis ===";

    // 统计坐标范围
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    // 统计属性
    QSet<QString> availableAttributes;
    QMap<QString, QPair<float, float>> attributeRanges;

    for (const auto& point : m_currentPointCloud) {
        // 坐标范围
        minX = qMin(minX, point.position.x());
        maxX = qMax(maxX, point.position.x());
        minY = qMin(minY, point.position.y());
        maxY = qMax(maxY, point.position.y());
        minZ = qMin(minZ, point.position.z());
        maxZ = qMax(maxZ, point.position.z());

        // 属性统计
        for (auto it = point.attributes.begin(); it != point.attributes.end(); ++it) {
            QString attrName = it.key();
            float attrValue = it.value().toFloat();

            availableAttributes.insert(attrName);

            if (!attributeRanges.contains(attrName)) {
                attributeRanges[attrName] = qMakePair(attrValue, attrValue);
            } else {
                auto& range = attributeRanges[attrName];
                range.first = qMin(range.first, attrValue);
                range.second = qMax(range.second, attrValue);
            }
        }
    }

    // 输出分析结果
    qDebug() << "Point count:" << m_currentPointCloud.size();
    qDebug() << "X range:" << minX << "to" << maxX << "(" << (maxX - minX) << ")";
    qDebug() << "Y range:" << minY << "to" << maxY << "(" << (maxY - minY) << ")";
    qDebug() << "Z range:" << minZ << "to" << maxZ << "(" << (maxZ - minZ) << ")";

    qDebug() << "Available attributes:" << availableAttributes.values();
    for (auto it = attributeRanges.begin(); it != attributeRanges.end(); ++it) {
        qDebug() << "Attribute" << it.key() << "range:" << it.value().first << "to" << it.value().second;
    }

    // 检查数据有效性
    if (maxX - minX < 0.001f || maxY - minY < 0.001f) {
        qDebug() << "WARNING: Point cloud has very small spatial extent";
    }

    if (availableAttributes.isEmpty()) {
        qDebug() << "WARNING: Point cloud has no attributes";
    }

    qDebug() << "Point cloud analysis completed";
}

void Stage1DemoWidget::validatePointCloudAttributes()
{
    qDebug() << "=== Validating Point Cloud Attributes ===";

    if (m_currentPointCloud.empty()) {
        qDebug() << "ERROR: Point cloud is empty";
        return;
    }

    // 检查前几个点的属性
    for (int i = 0; i < qMin(3, static_cast<int>(m_currentPointCloud.size())); ++i) {
        const auto& point = m_currentPointCloud[i];
        qDebug() << "Point" << i << ":";
        qDebug() << "  Position:" << point.position;
        qDebug() << "  Attributes count:" << point.attributes.size();

        for (auto it = point.attributes.begin(); it != point.attributes.end(); ++it) {
            qDebug() << "    " << it.key() << ":" << it.value();
        }
    }
}

void Stage1DemoWidget::debugRenderingPipeline()
{
    qDebug() << "=== Debugging Rendering Pipeline ===";

    // 检查渲染器状态
    if (!m_renderer) {
        qDebug() << "ERROR: Renderer is null";
        return;
    }

    qDebug() << "Renderer initialized:" << m_renderer->isInitialized();
    qDebug() << "Render mode:" << static_cast<int>(m_renderer->getRenderMode());
    qDebug() << "Point size:" << m_renderer->getPointSize();
    qDebug() << "Viewport size:" << m_renderer->getViewportSize();
    qDebug() << "View bounds:" << m_renderer->getViewBounds();

    // 检查颜色映射器状态
    if (!m_colorMapper) {
        qDebug() << "ERROR: Color mapper is null";
        return;
    }

    qDebug() << "Color mapper initialized:" << m_colorMapper->isInitialized();
    qDebug() << "Current color scheme:" << static_cast<int>(m_colorMapper->getCurrentColorScheme());
    auto range = m_colorMapper->getValueRange();
    qDebug() << "Color mapping range:" << range.first << "to" << range.second;

    qDebug() << "Rendering pipeline debug completed";
}

std::vector<WallExtraction::PointWithAttributes> Stage1DemoWidget::performIntelligentSampling(
    const std::vector<WallExtraction::PointWithAttributes>& points,
    size_t targetCount) const
{
    qDebug() << "=== Performing Intelligent Sampling ===";
    qDebug() << "Input points:" << points.size() << "Target count:" << targetCount;

    if (points.size() <= targetCount) {
        qDebug() << "No sampling needed";
        return points;
    }

    std::vector<WallExtraction::PointWithAttributes> sampledPoints;
    sampledPoints.reserve(targetCount);

    // 计算采样步长
    double step = static_cast<double>(points.size()) / targetCount;
    qDebug() << "Sampling step:" << step;

    // 均匀采样策略
    for (size_t i = 0; i < targetCount; ++i) {
        size_t index = static_cast<size_t>(i * step);
        if (index < points.size()) {
            sampledPoints.push_back(points[index]);
        }
    }

    qDebug() << "Sampling completed. Output points:" << sampledPoints.size();

    // 验证采样结果的空间分布
    if (!sampledPoints.empty()) {
        float minX = sampledPoints[0].position.x();
        float maxX = minX, minY = sampledPoints[0].position.y();
        float maxY = minY, minZ = sampledPoints[0].position.z(), maxZ = minZ;

        for (const auto& point : sampledPoints) {
            minX = qMin(minX, point.position.x());
            maxX = qMax(maxX, point.position.x());
            minY = qMin(minY, point.position.y());
            maxY = qMax(maxY, point.position.y());
            minZ = qMin(minZ, point.position.z());
            maxZ = qMax(maxZ, point.position.z());
        }

        qDebug() << "Sampled data bounds:";
        qDebug() << "  X:" << minX << "to" << maxX;
        qDebug() << "  Y:" << minY << "to" << maxY;
        qDebug() << "  Z:" << minZ << "to" << maxZ;
    }

    return sampledPoints;
}

void Stage1DemoWidget::generateValidTestData(int pointCount)
{
    qDebug() << "=== Generating Valid Test Data ===";
    qDebug() << "Generating" << pointCount << "test points";

    m_currentPointCloud.clear();
    m_currentSimpleCloud.clear();
    m_currentPointCloud.reserve(pointCount);
    m_currentSimpleCloud.reserve(pointCount);

    // 生成一个简单的房间结构
    const float roomWidth = 10.0f;
    const float roomHeight = 8.0f;
    const float roomDepth = 3.0f;

    for (int i = 0; i < pointCount; ++i) {
        QVector3D position;

        // 生成不同类型的点
        float type = static_cast<float>(i) / pointCount;

        if (type < 0.4f) {
            // 地面点 (40%)
            position.setX((static_cast<float>(rand()) / RAND_MAX - 0.5f) * roomWidth);
            position.setY((static_cast<float>(rand()) / RAND_MAX - 0.5f) * roomHeight);
            position.setZ(0.0f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f);
        } else if (type < 0.7f) {
            // 墙面点 (30%)
            if (rand() % 2 == 0) {
                // 左右墙
                position.setX((rand() % 2 == 0) ? -roomWidth/2 : roomWidth/2);
                position.setY((static_cast<float>(rand()) / RAND_MAX - 0.5f) * roomHeight);
                position.setZ((static_cast<float>(rand()) / RAND_MAX) * roomDepth);
            } else {
                // 前后墙
                position.setX((static_cast<float>(rand()) / RAND_MAX - 0.5f) * roomWidth);
                position.setY((rand() % 2 == 0) ? -roomHeight/2 : roomHeight/2);
                position.setZ((static_cast<float>(rand()) / RAND_MAX) * roomDepth);
            }
        } else {
            // 天花板和其他点 (30%)
            position.setX((static_cast<float>(rand()) / RAND_MAX - 0.5f) * roomWidth);
            position.setY((static_cast<float>(rand()) / RAND_MAX - 0.5f) * roomHeight);
            position.setZ(roomDepth + (static_cast<float>(rand()) / RAND_MAX) * 0.5f);
        }

        // 创建点云数据
        WallExtraction::PointWithAttributes point;
        point.position = position;

        // 生成属性
        point.attributes["intensity"] = static_cast<int>(position.z() * 1000);
        point.attributes["classification"] = (position.z() < 0.5f) ? 2 : 6; // 地面或其他
        point.attributes["red"] = static_cast<int>((position.z() / roomDepth) * 65535);
        point.attributes["green"] = 65535 - point.attributes["red"].toInt();
        point.attributes["blue"] = (point.attributes["red"].toInt() + 32767) % 65535;

        m_currentPointCloud.push_back(point);
        m_currentSimpleCloud.push_back(position);
    }

    qDebug() << "Test data generation completed:" << m_currentPointCloud.size() << "points";

    // 显示数据范围
    if (!m_currentSimpleCloud.empty()) {
        float minX = m_currentSimpleCloud[0].x(), maxX = minX;
        float minY = m_currentSimpleCloud[0].y(), maxY = minY;
        float minZ = m_currentSimpleCloud[0].z(), maxZ = minZ;

        for (const auto& point : m_currentSimpleCloud) {
            minX = qMin(minX, point.x());
            maxX = qMax(maxX, point.x());
            minY = qMin(minY, point.y());
            maxY = qMax(maxY, point.y());
            minZ = qMin(minZ, point.z());
            maxZ = qMax(maxZ, point.z());
        }

        qDebug() << "Test data bounds:";
        qDebug() << "  X:" << minX << "to" << maxX;
        qDebug() << "  Y:" << minY << "to" << maxY;
        qDebug() << "  Z:" << minZ << "to" << maxZ;
    }
}

void Stage1DemoWidget::updateColorMapping()
{
    // 如果正在清除过程中，阻止颜色映射更新
    if (m_isClearing) {
        qDebug() << "Clearing in progress, skipping color mapping update";
        return;
    }

    generateColorBar();
    if (!m_currentPointCloud.empty()) {
        updateTopDownView();
    }
}

void Stage1DemoWidget::updateLODDisplay()
{
    // 如果正在清除过程中，阻止LOD更新
    if (m_isClearing) {
        qDebug() << "Clearing in progress, skipping LOD display update";
        return;
    }

    if (m_lodManager->getLODLevelCount() == 0) {
        return;
    }

    int currentLevel = m_lodLevelSlider->value();
    auto lodPoints = m_lodManager->getLODPoints(currentLevel);

    QString info = QString("LOD Level %1: %2 points")
                  .arg(currentLevel)
                  .arg(lodPoints.size());
    m_lodLevelLabel->setText(info);

    // 自动更新渲染
    if (!lodPoints.empty()) {
        updateTopDownView();
    }
}

void Stage1DemoWidget::connectSignals()
{
    qDebug() << "Connecting signals for responsive UI...";

    // 文件操作按钮连接
    if (m_loadFileButton) {
        connect(m_loadFileButton, &QPushButton::clicked, this, &Stage1DemoWidget::loadPointCloudFile);
    }
    if (m_saveImageButton) {
        connect(m_saveImageButton, &QPushButton::clicked, this, &Stage1DemoWidget::saveCurrentImage);
    }

    // 渲染操作按钮连接
    if (m_renderButton) {
        connect(m_renderButton, &QPushButton::clicked, this, &Stage1DemoWidget::renderTopDownView);
    }
    if (m_generateDataButton) {
        connect(m_generateDataButton, &QPushButton::clicked, this, &Stage1DemoWidget::generateTestData);
    }
    if (m_clearDataButton) {
        connect(m_clearDataButton, &QPushButton::clicked, this, &Stage1DemoWidget::clearPointCloud);
    }

    // LOD控制连接
    if (m_lodLevelSlider) {
        connect(m_lodLevelSlider, &QSlider::valueChanged, this, &Stage1DemoWidget::onLODLevelChanged);
    }
    if (m_generateLODButton) {
        connect(m_generateLODButton, &QPushButton::clicked, this, &Stage1DemoWidget::generateLODLevels);
    }

    // 颜色映射控制连接
    if (m_colorSchemeCombo) {
        connect(m_colorSchemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &Stage1DemoWidget::onColorSchemeChanged);
    }
    if (m_generateColorBarButton) {
        connect(m_generateColorBarButton, &QPushButton::clicked, this, &Stage1DemoWidget::generateColorBar);
    }

    // 渲染控制连接
    if (m_renderModeCombo) {
        connect(m_renderModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &Stage1DemoWidget::onRenderModeChanged);
    }
    if (m_pointSizeSpin) {
        connect(m_pointSizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &Stage1DemoWidget::onPointSizeChanged);
    }

    // 线段绘制控制连接
    if (m_lineDrawingToolbar) {
        connect(m_lineDrawingToolbar, &WallExtraction::LineDrawingToolbar::drawingModeChangeRequested,
                this, &Stage1DemoWidget::onLineDrawingModeChanged);
        connect(m_lineDrawingToolbar, &WallExtraction::LineDrawingToolbar::editModeChangeRequested,
                this, &Stage1DemoWidget::onEditModeChanged);
        qDebug() << "Line drawing toolbar signals connected";
    }

    // LinePropertyPanel已移除，不再连接相关信号

    // 连接LineDrawingTool的信号
    if (m_wallManager && m_wallManager->getLineDrawingTool()) {
        auto* lineDrawingTool = m_wallManager->getLineDrawingTool();

        connect(lineDrawingTool, &WallExtraction::LineDrawingTool::lineSegmentAdded,
                this, &Stage1DemoWidget::onLineSegmentAdded);
        connect(lineDrawingTool, &WallExtraction::LineDrawingTool::lineSegmentSelected,
                this, &Stage1DemoWidget::onLineSegmentSelected);
        connect(lineDrawingTool, &WallExtraction::LineDrawingTool::lineSegmentRemoved,
                this, &Stage1DemoWidget::onLineSegmentRemoved);
        connect(lineDrawingTool, &WallExtraction::LineDrawingTool::visualFeedbackUpdateRequested,
                this, &Stage1DemoWidget::updateRenderView);

        qDebug() << "LineDrawingTool signals connected";
    }

    // 渲染参数折叠按钮连接
    if (m_toggleRenderParamsButton) {
        // 连接已在createRenderParamsToggleButton中完成
        qDebug() << "Render parameters toggle button signals connected";
    }

    // 线框绘制工具折叠按钮连接
    if (m_toggleLineDrawingButton) {
        // 连接已在createLineDrawingToggleButton中完成
        qDebug() << "Line drawing tools toggle button signals connected";
    }

    qDebug() << "All signals connected successfully";
}



// 缺失的槽函数实现（saveCurrentImage）
void Stage1DemoWidget::saveCurrentImage()
{
    qDebug() << "Saving current image...";
    // 调用现有的保存方法
    saveRenderResult();
}

// 线段绘制相关槽函数实现
void Stage1DemoWidget::onLineDrawingModeChanged(WallExtraction::DrawingMode mode)
{
    qDebug() << "Line drawing mode changed to:" << static_cast<int>(mode);

    // 将模式传递给LineDrawingTool
    if (m_wallManager && m_wallManager->getLineDrawingTool()) {
        m_wallManager->getLineDrawingTool()->setDrawingMode(mode);
        qDebug() << "Drawing mode set on LineDrawingTool";
    } else {
        qWarning() << "LineDrawingTool not available for mode change";
    }
}

void Stage1DemoWidget::onEditModeChanged(WallExtraction::EditMode mode)
{
    qDebug() << "Edit mode changed to:" << static_cast<int>(mode);

    // 将编辑模式传递给LineDrawingTool
    if (m_wallManager && m_wallManager->getLineDrawingTool()) {
        m_wallManager->getLineDrawingTool()->setEditMode(mode);
        qDebug() << "Edit mode set on LineDrawingTool";
    } else {
        qWarning() << "LineDrawingTool not available for edit mode change";
    }
}

void Stage1DemoWidget::onLineSegmentAdded()
{
    qDebug() << "Line segment added";
    // 可以在这里添加线段添加后的处理逻辑
    // 例如更新统计信息、刷新显示等
}

void Stage1DemoWidget::onLineSegmentSelected(int segmentId)
{
    qDebug() << "Line segment selected:" << segmentId;
    // 线段选中处理逻辑（LinePropertyPanel已移除）
}

void Stage1DemoWidget::onLineSegmentRemoved(int segmentId)
{
    qDebug() << "Line segment removed:" << segmentId;
    // 可以在这里添加线段删除后的处理逻辑
}

// toggleLinePropertyPanel方法已移除，因为LinePropertyPanel已被完全移除

// 内存使用更新方法已删除

// 渲染统计更新方法已删除

// 性能指标更新方法已删除

// ==================== 坐标映射辅助方法 ====================

// 将QLabel内坐标映射到渲染视口坐标
bool Stage1DemoWidget::mapLabelPosToViewport(const QPoint& labelPos, QPointF& viewportPos) const
{
    qDebug() << "=== mapLabelPosToViewport ===";
    qDebug() << "Input labelPos:" << labelPos;
    qDebug() << "Last scaled pixmap size:" << m_lastScaledPixmapSize;
    qDebug() << "Last pixmap top left:" << m_lastPixmapTopLeft;

    // 检查是否有有效的缩放pixmap信息
    if (m_lastScaledPixmapSize.isEmpty()) {
        qDebug() << "No scaled pixmap size recorded, cannot map coordinates";
        return false;
    }

    // 检查点击是否在实际渲染的pixmap区域内（排除letterbox区域）
    QRect pixmapRect(m_lastPixmapTopLeft, m_lastScaledPixmapSize);
    qDebug() << "Pixmap rect:" << pixmapRect;

    if (!pixmapRect.contains(labelPos)) {
        qDebug() << "Click position" << labelPos << "is outside pixmap area" << pixmapRect;
        return false;
    }

    // 将label内坐标转换为pixmap内坐标（去除letterbox偏移）
    QPoint pixmapLocal = labelPos - m_lastPixmapTopLeft;
    qDebug() << "Pixmap local pos:" << pixmapLocal;

    // 确保pixmap坐标在有效范围内
    if (pixmapLocal.x() < 0 || pixmapLocal.y() < 0 ||
        pixmapLocal.x() >= m_lastScaledPixmapSize.width() ||
        pixmapLocal.y() >= m_lastScaledPixmapSize.height()) {
        qDebug() << "Pixmap local position out of bounds:" << pixmapLocal;
        return false;
    }

    // 计算在pixmap内的相对位置（0.0-1.0）
    float relativeX = static_cast<float>(pixmapLocal.x()) / m_lastScaledPixmapSize.width();
    float relativeY = static_cast<float>(pixmapLocal.y()) / m_lastScaledPixmapSize.height();

    qDebug() << "Relative coords:" << relativeX << "," << relativeY;

    // 映射到渲染视口坐标
    viewportPos.setX(relativeX * m_currentViewportSize.width());
    viewportPos.setY(relativeY * m_currentViewportSize.height());

    qDebug() << "Final viewport pos:" << viewportPos;
    qDebug() << "Current viewport size:" << m_currentViewportSize;

    return true;
}

// 检查点击是否在渲染区域内
bool Stage1DemoWidget::isPointInRenderedArea(const QPoint& labelPos) const
{
    if (m_lastScaledPixmapSize.isEmpty()) {
        return false;
    }

    QRect pixmapRect(m_lastPixmapTopLeft, m_lastScaledPixmapSize);
    return pixmapRect.contains(labelPos);
}

// ==================== 新的视口坐标转换方法 ====================

// 视口坐标到世界坐标转换（基于当前视图边界）
QVector3D Stage1DemoWidget::viewportToWorld(const QVector2D& viewportPoint, const QRectF& bounds) const
{
    qDebug() << "=== viewportToWorld ===";
    qDebug() << "Viewport point:" << viewportPoint;
    qDebug() << "Current viewport size:" << m_currentViewportSize;
    qDebug() << "Current view bounds:" << m_currentViewBounds;

    if (m_currentViewportSize.isEmpty() || m_currentViewBounds.isEmpty()) {
        qDebug() << "ERROR: Invalid viewport size or view bounds";
        return QVector3D(viewportPoint.x(), viewportPoint.y(), 0.0f);
    }

    // 将视口坐标标准化到[0,1]范围
    float normalizedX = viewportPoint.x() / m_currentViewportSize.width();
    float normalizedY = viewportPoint.y() / m_currentViewportSize.height();

    // 映射到世界坐标（使用当前视图边界）
    float worldX = m_currentViewBounds.left() + normalizedX * m_currentViewBounds.width();
    float worldY = m_currentViewBounds.top() + (1.0f - normalizedY) * m_currentViewBounds.height(); // Y轴翻转

    QVector3D result(worldX, worldY, 0.0f);
    qDebug() << "Converted to world:" << result;

    return result;
}

// 世界坐标到视口坐标转换（基于当前视图边界）
QPointF Stage1DemoWidget::worldToViewport(const QVector3D& worldPoint, const QRectF& bounds) const
{
    qDebug() << "=== worldToViewport ===";
    qDebug() << "World point:" << worldPoint;
    qDebug() << "Current viewport size:" << m_currentViewportSize;
    qDebug() << "Current view bounds:" << m_currentViewBounds;

    if (m_currentViewportSize.isEmpty() || m_currentViewBounds.isEmpty()) {
        qDebug() << "ERROR: Invalid viewport size or view bounds";
        return QPointF(worldPoint.x(), worldPoint.y());
    }

    // 将世界坐标标准化到[0,1]范围
    float normalizedX = (worldPoint.x() - m_currentViewBounds.left()) / m_currentViewBounds.width();
    float normalizedY = (worldPoint.y() - m_currentViewBounds.top()) / m_currentViewBounds.height();

    // Y轴翻转并映射到视口坐标
    float viewportX = normalizedX * m_currentViewportSize.width();
    float viewportY = (1.0f - normalizedY) * m_currentViewportSize.height();

    QPointF result(viewportX, viewportY);
    qDebug() << "Converted to viewport:" << result;

    return result;
}

// 视口坐标到pixmap坐标转换（用于线段绘制）
QPointF Stage1DemoWidget::viewportToPixmap(const QPointF& viewportPoint, const QSize& pixmapSize) const
{
    if (m_currentViewportSize.isEmpty()) {
        qDebug() << "ERROR: Current viewport size is empty";
        return viewportPoint;
    }

    // 计算缩放比例
    float scaleX = static_cast<float>(pixmapSize.width()) / m_currentViewportSize.width();
    float scaleY = static_cast<float>(pixmapSize.height()) / m_currentViewportSize.height();

    QPointF result(viewportPoint.x() * scaleX, viewportPoint.y() * scaleY);
    qDebug() << "Viewport to pixmap:" << viewportPoint << "->" << result;

    return result;
}

// 直接的世界坐标到pixmap坐标转换（最简化，避免累积误差）
QPointF Stage1DemoWidget::directWorldToPixmap(const QVector3D& worldPoint, const QSize& pixmapSize, const QRectF& worldBounds) const
{
    qDebug() << "=== directWorldToPixmap ===";
    qDebug() << "World point:" << worldPoint;
    qDebug() << "Pixmap size:" << pixmapSize;
    qDebug() << "World bounds:" << worldBounds;

    // 增强的输入验证
    if (worldBounds.isEmpty() || pixmapSize.isEmpty()) {
        qDebug() << "ERROR: Invalid world bounds or pixmap size";
        return QPointF(worldPoint.x(), worldPoint.y());
    }

    if (worldBounds.width() <= 0 || worldBounds.height() <= 0) {
        qWarning() << "Invalid world bounds dimensions:" << worldBounds;
        return QPointF(worldPoint.x(), worldPoint.y());
    }

    if (pixmapSize.width() <= 0 || pixmapSize.height() <= 0) {
        qWarning() << "Invalid pixmap dimensions:" << pixmapSize;
        return QPointF(worldPoint.x(), worldPoint.y());
    }

    // 将世界坐标标准化到[0,1]范围
    float normalizedX = (worldPoint.x() - worldBounds.left()) / worldBounds.width();
    float normalizedY = (worldPoint.y() - worldBounds.top()) / worldBounds.height();

    qDebug() << "Normalized coords:" << normalizedX << "," << normalizedY;

    // 确保标准化坐标在合理范围内（允许稍微超出边界）
    if (normalizedX < -0.1f || normalizedX > 1.1f || normalizedY < -0.1f || normalizedY > 1.1f) {
        qWarning() << "World point significantly outside bounds:" << worldPoint << "in bounds:" << worldBounds;
    }

    // 直接映射到pixmap坐标，Y轴翻转
    float pixmapX = normalizedX * pixmapSize.width();
    float pixmapY = (1.0f - normalizedY) * pixmapSize.height();

    QPointF result(pixmapX, pixmapY);
    qDebug() << "Direct conversion result:" << result;

    return result;
}

// 直接的视口坐标到世界坐标转换（与渲染器完全一致）
QVector3D Stage1DemoWidget::directViewportToWorld(const QVector2D& viewportPoint, const QRectF& worldBounds) const
{
    qDebug() << "=== directViewportToWorld ===";
    qDebug() << "Viewport point:" << viewportPoint;
    qDebug() << "Current viewport size:" << m_currentViewportSize;
    qDebug() << "World bounds:" << worldBounds;

    // 增强的输入验证
    if (m_currentViewportSize.isEmpty() || worldBounds.isEmpty()) {
        qDebug() << "ERROR: Invalid viewport size or world bounds";
        return QVector3D(viewportPoint.x(), viewportPoint.y(), 0.0f);
    }

    if (m_currentViewportSize.width() <= 0 || m_currentViewportSize.height() <= 0) {
        qWarning() << "Invalid viewport dimensions:" << m_currentViewportSize;
        return QVector3D(0, 0, 0);
    }

    if (worldBounds.width() <= 0 || worldBounds.height() <= 0) {
        qWarning() << "Invalid world bounds dimensions:" << worldBounds;
        return QVector3D(0, 0, 0);
    }

    // 将视口坐标标准化到[0,1]范围
    float normalizedX = viewportPoint.x() / m_currentViewportSize.width();
    float normalizedY = viewportPoint.y() / m_currentViewportSize.height();

    // 确保标准化坐标在有效范围内
    normalizedX = qBound(0.0f, normalizedX, 1.0f);
    normalizedY = qBound(0.0f, normalizedY, 1.0f);

    qDebug() << "Normalized coords:" << normalizedX << "," << normalizedY;

    // 映射到世界坐标，Y轴翻转
    float worldX = worldBounds.left() + normalizedX * worldBounds.width();
    float worldY = worldBounds.top() + (1.0f - normalizedY) * worldBounds.height();

    QVector3D result(worldX, worldY, 0.0f);
    qDebug() << "Direct viewport to world result:" << result;

    return result;
}


// 直接的世界坐标到视口坐标转换（与渲染器完全一致）
QPointF Stage1DemoWidget::directWorldToViewport(const QVector3D& worldPoint, const QRectF& worldBounds) const
{
    qDebug() << "=== directWorldToViewport ===";
    qDebug() << "World point:" << worldPoint;
    qDebug() << "Current viewport size:" << m_currentViewportSize;
    qDebug() << "World bounds:" << worldBounds;

    if (m_currentViewportSize.isEmpty() || worldBounds.isEmpty()) {
        qDebug() << "ERROR: Invalid viewport size or world bounds";
        return QPointF(worldPoint.x(), worldPoint.y());
    }

    // 将世界坐标标准化到[0,1]范围
    float normalizedX = (worldPoint.x() - worldBounds.left()) / worldBounds.width();
    float normalizedY = (worldPoint.y() - worldBounds.top()) / worldBounds.height();

    // 映射到视口坐标，Y轴翻转
    float viewportX = normalizedX * m_currentViewportSize.width();
    float viewportY = (1.0f - normalizedY) * m_currentViewportSize.height();

    QPointF result(viewportX, viewportY);
    qDebug() << "Direct world to viewport result:" << result;

    return result;
}
