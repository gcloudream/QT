#include "line_property_panel.h"
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QResizeEvent>

namespace WallExtraction {

LinePropertyPanel::LinePropertyPanel(QWidget* parent)
    : QWidget(parent)
    , m_lineDrawingTool(nullptr)
    , m_mainLayout(nullptr)
    , m_headerWidget(nullptr)
    , m_contentWidget(nullptr)
    , m_tabWidget(nullptr)
    , m_splitter(nullptr)
    , m_infoPanel(nullptr)
    , m_listWidget(nullptr)
    , m_toggleAnimation(nullptr)
    , m_opacityEffect(nullptr)
    , m_opacityAnimation(nullptr)
    , m_animationGroup(nullptr)
    , m_isVisible(true)
    , m_isAnimating(false)
    , m_collapsedSize(300, 40)
    , m_expandedSize(300, 600)
    , m_currentTabIndex(0)
{
    setupUI();
    setupAnimations();
    connectSignals();
    
    // 设置默认大小
    resize(m_expandedSize);
    setMinimumSize(m_collapsedSize);
}

LinePropertyPanel::~LinePropertyPanel()
{
    // Qt会自动清理子对象
}

void LinePropertyPanel::setLineDrawingTool(LineDrawingTool* tool)
{
    // 断开之前工具的信号连接
    if (m_lineDrawingTool) {
        disconnect(m_lineDrawingTool, nullptr, this, nullptr);
    }
    
    m_lineDrawingTool = tool;
    
    // 设置子组件的工具
    if (m_infoPanel) {
        m_infoPanel->setLineDrawingTool(tool);
    }
    if (m_listWidget) {
        m_listWidget->setLineDrawingTool(tool);
    }
    
    // 连接新工具的信号
    if (m_lineDrawingTool) {
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentAdded,
                this, &LinePropertyPanel::onLineSegmentAdded);
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentRemoved,
                this, &LinePropertyPanel::onLineSegmentRemoved);
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentSelected,
                this, &LinePropertyPanel::onLineSegmentSelected);
    }
}

void LinePropertyPanel::setupUI()
{
    // 设置面板样式
    setObjectName("LinePropertyPanel");
    setStyleSheet(getModernPanelStyle());
    
    // 设置面板的最小尺寸和大小策略，确保有足够空间
    setMinimumHeight(250);  // 增加最小高度
    setMinimumWidth(400);   // 增加最小宽度
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 16, 16, 16);  // 增加边距
    m_mainLayout->setSpacing(12);  // 增加间距
    
    setupHeaderWidget();
    setupContentWidget();
}

void LinePropertyPanel::setupHeaderWidget()
{
    m_headerWidget = new QWidget(this);
    m_headerWidget->setObjectName("HeaderWidget");
    m_headerWidget->setFixedHeight(48);  // 进一步增加高度

    m_headerLayout = new QHBoxLayout(m_headerWidget);
    m_headerLayout->setContentsMargins(16, 8, 16, 8);  // 增加边距
    m_headerLayout->setSpacing(16);  // 增加间距

    m_titleLabel = createStyledLabel("线段属性", "title-label xlarge");
    m_toggleButton = createStyledButton("折叠", "toggle-button xlarge");
    m_toggleButton->setFixedSize(80, 32);  // 大幅增加按钮尺寸
    
    m_headerLayout->addWidget(m_titleLabel);
    m_headerLayout->addStretch();
    m_headerLayout->addWidget(m_toggleButton);
    
    // 添加分隔线
    m_headerSeparator = new QFrame(this);
    m_headerSeparator->setFrameShape(QFrame::HLine);
    m_headerSeparator->setFrameShadow(QFrame::Sunken);
    m_headerSeparator->setFixedHeight(1);
    
    m_mainLayout->addWidget(m_headerWidget);
    m_mainLayout->addWidget(m_headerSeparator);
}

void LinePropertyPanel::setupContentWidget()
{
    m_contentWidget = new QWidget(this);
    
    QVBoxLayout* contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(8, 8, 8, 8);  // 增加边距
    contentLayout->setSpacing(6);  // 增加间距

    // 创建标签页控件并设置样式
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet(getModernTabWidgetStyle());

    // 创建子面板
    m_infoPanel = new LineInfoPanel(this);
    m_listWidget = new LineListWidget(this);

    // 添加标签页
    m_tabWidget->addTab(m_infoPanel, "线段信息");
    m_tabWidget->addTab(m_listWidget, "线段列表");
    
    contentLayout->addWidget(m_tabWidget);
    
    m_mainLayout->addWidget(m_contentWidget);
}

void LinePropertyPanel::setupAnimations()
{
    // 设置透明度效果
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_contentWidget->setGraphicsEffect(m_opacityEffect);
    
    // 创建大小动画
    m_toggleAnimation = new QPropertyAnimation(this, "size", this);
    m_toggleAnimation->setDuration(300);
    m_toggleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    
    // 创建透明度动画
    m_opacityAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_opacityAnimation->setDuration(300);
    m_opacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    
    // 创建并行动画组
    m_animationGroup = new QParallelAnimationGroup(this);
    m_animationGroup->addAnimation(m_toggleAnimation);
    m_animationGroup->addAnimation(m_opacityAnimation);
    
    // 连接动画完成信号
    connect(m_animationGroup, &QParallelAnimationGroup::finished,
            this, [this]() {
                m_isAnimating = false;
                if (!m_isVisible) {
                    m_contentWidget->hide();
                }
            });
}

void LinePropertyPanel::connectSignals()
{
    // 连接头部控件信号
    connect(m_toggleButton, &QPushButton::clicked,
            this, &LinePropertyPanel::onToggleButtonClicked);
    
    // 连接标签页信号
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &LinePropertyPanel::onTabChanged);
    
    // 连接子组件信号
    if (m_listWidget) {
        connect(m_listWidget, &LineListWidget::segmentSelectionRequested,
                this, &LinePropertyPanel::segmentSelectionRequested);
        connect(m_listWidget, &LineListWidget::segmentDeletionRequested,
                this, &LinePropertyPanel::segmentDeletionRequested);
        connect(m_listWidget, &LineListWidget::segmentDescriptionEditRequested,
                this, &LinePropertyPanel::segmentDescriptionEditRequested);
    }
    
    if (m_infoPanel) {
        connect(m_infoPanel, &LineInfoPanel::segmentDescriptionChanged,
                this, &LinePropertyPanel::segmentDescriptionChanged);
    }
}

// 面板显示控制方法
void LinePropertyPanel::showPanel()
{
    if (m_isVisible || m_isAnimating) {
        return;
    }

    m_isVisible = true;
    m_contentWidget->show();
    animateToggle(true);
    updateToggleButtonText();

    emit panelVisibilityChanged(true);
}

void LinePropertyPanel::hidePanel()
{
    if (!m_isVisible || m_isAnimating) {
        return;
    }

    m_isVisible = false;
    animateToggle(false);
    updateToggleButtonText();

    emit panelVisibilityChanged(false);
}

void LinePropertyPanel::togglePanel()
{
    if (m_isVisible) {
        hidePanel();
    } else {
        showPanel();
    }
}

bool LinePropertyPanel::isPanelVisible() const
{
    return m_isVisible;
}

void LinePropertyPanel::setMinimumPanelSize(const QSize& size)
{
    m_collapsedSize = size;
    setMinimumSize(size);
}

void LinePropertyPanel::setMaximumPanelSize(const QSize& size)
{
    m_expandedSize = size;
    if (m_isVisible) {
        setMaximumSize(size);
    }
}

// 响应线段工具信号的槽函数
void LinePropertyPanel::onLineSegmentAdded(const LineSegment& segment)
{
    // 子组件会自动响应，这里可以添加额外的处理逻辑
    Q_UNUSED(segment)
}

void LinePropertyPanel::onLineSegmentRemoved(int segmentId)
{
    // 子组件会自动响应，这里可以添加额外的处理逻辑
    Q_UNUSED(segmentId)
}

void LinePropertyPanel::onLineSegmentSelected(int segmentId)
{
    // 子组件会自动响应，这里可以添加额外的处理逻辑
    Q_UNUSED(segmentId)

    // 如果面板是折叠状态，可以考虑自动展开
    if (!m_isVisible) {
        showPanel();
    }
}

// 面板控制槽函数
void LinePropertyPanel::onToggleButtonClicked()
{
    togglePanel();
}

void LinePropertyPanel::onTabChanged(int index)
{
    m_currentTabIndex = index;

    // 可以根据标签页切换调整面板大小
    if (index == 0) { // 信息面板
        // 信息面板通常需要较小的空间
    } else if (index == 1) { // 列表控件
        // 列表控件通常需要更多空间
    }
}

// 事件处理方法
void LinePropertyPanel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    emit panelSizeChanged(event->size());
}

void LinePropertyPanel::paintEvent(QPaintEvent* event)
{
    // 确保样式表正确应用
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

// 私有辅助方法
void LinePropertyPanel::updateToggleButtonText()
{
    if (m_isVisible) {
        m_toggleButton->setText("折叠");
    } else {
        m_toggleButton->setText("展开");
    }
}

void LinePropertyPanel::animateToggle(bool show)
{
    if (m_isAnimating) {
        return;
    }

    m_isAnimating = true;

    QSize startSize = size();
    QSize endSize = show ? m_expandedSize : m_collapsedSize;

    // 设置大小动画
    m_toggleAnimation->setStartValue(startSize);
    m_toggleAnimation->setEndValue(endSize);

    // 设置透明度动画
    float startOpacity = show ? 0.0f : 1.0f;
    float endOpacity = show ? 1.0f : 0.0f;

    m_opacityAnimation->setStartValue(startOpacity);
    m_opacityAnimation->setEndValue(endOpacity);

    // 启动动画
    m_animationGroup->start();
}

// ==================== 现代化样式方法 ====================

QString LinePropertyPanel::getModernPanelStyle() const
{
    return R"(
        #LinePropertyPanel {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                      stop: 0 #ffffff, stop: 1 #f8f9fa);
            border: 2px solid #dee2e6;
            border-radius: 12px;
            padding: 0px;
        }
        #HeaderWidget {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                      stop: 0 #e9ecef, stop: 1 #dee2e6);
            border-bottom: 2px solid #ced4da;
            border-top-left-radius: 12px;
            border-top-right-radius: 12px;
        }
    )";
}

QPushButton* LinePropertyPanel::createStyledButton(const QString& text, const QString& styleClass)
{
    QPushButton* button = new QPushButton(text, this);
    button->setObjectName(styleClass);

    // 支持不同尺寸的按钮
    QString fontSize, padding, minHeight;

    if (styleClass.contains("xlarge")) {
        fontSize = "14px";
        padding = "8px 16px";
        minHeight = "32px";
    } else if (styleClass.contains("large")) {
        fontSize = "12px";
        padding = "6px 12px";
        minHeight = "28px";
    } else {
        fontSize = "11px";
        padding = "4px 8px";
        minHeight = "20px";
    }

    QString style = QString(R"(
        QPushButton {
            font-size: %1;
            font-weight: 500;
            padding: %2;
            border: 2px solid #bdc3c7;
            border-radius: 6px;
            background-color: #ffffff;
            color: #2c3e50;
            min-height: %3;
        }
        QPushButton:hover {
            background-color: #ecf0f1;
            border-color: #95a5a6;
        }
        QPushButton:pressed {
            background-color: #d5dbdb;
        }
    )").arg(fontSize, padding, minHeight);

    if (styleClass.contains("toggle")) {
        style += R"(
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

    button->setStyleSheet(style);
    return button;
}

QLabel* LinePropertyPanel::createStyledLabel(const QString& text, const QString& styleClass)
{
    QLabel* label = new QLabel(text, this);
    label->setObjectName(styleClass);

    QString fontSize, fontWeight, color;

    if (styleClass.contains("title")) {
        if (styleClass.contains("xlarge")) {
            fontSize = "16px";
            fontWeight = "600";
            color = "#2c3e50";
        } else {
            fontSize = "13px";
            fontWeight = "600";
            color = "#34495e";
        }
    } else {
        if (styleClass.contains("xlarge")) {
            fontSize = "13px";
            fontWeight = "500";
            color = "#495057";
        } else {
            fontSize = "11px";
            fontWeight = "400";
            color = "#7f8c8d";
        }
    }

    QString style = QString(R"(
        QLabel {
            font-size: %1;
            font-weight: %2;
            color: %3;
            background-color: transparent;
            padding: 4px 6px;
        }
    )").arg(fontSize, fontWeight, color);

    label->setStyleSheet(style);
    return label;
}

QString LinePropertyPanel::getModernTabWidgetStyle() const
{
    return R"(
        QTabWidget::pane {
            border: 2px solid #dee2e6;
            border-radius: 8px;
            background-color: #ffffff;
            margin-top: 4px;
            padding: 8px;
        }
        QTabWidget::tab-bar {
            alignment: left;
        }
        QTabBar::tab {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                      stop: 0 #f8f9fa, stop: 1 #e9ecef);
            border: 2px solid #dee2e6;
            border-bottom: none;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            padding: 8px 16px;
            margin-right: 4px;
            font-size: 12px;
            font-weight: 500;
            color: #6c757d;
            min-width: 80px;
            min-height: 32px;
        }
        QTabBar::tab:selected {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                      stop: 0 #ffffff, stop: 1 #f8f9fa);
            color: #495057;
            border-bottom: 2px solid #ffffff;
            font-weight: 600;
            border-color: #adb5bd;
        }
        QTabBar::tab:hover:!selected {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                      stop: 0 #e9ecef, stop: 1 #dee2e6);
            color: #495057;
            border-color: #ced4da;
        }
    )";
}

} // namespace WallExtraction
