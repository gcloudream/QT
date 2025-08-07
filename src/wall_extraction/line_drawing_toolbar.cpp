#include "line_drawing_toolbar.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

namespace WallExtraction {

LineDrawingToolbar::LineDrawingToolbar(QWidget* parent)
    : QWidget(parent)
    , m_lineDrawingTool(nullptr)
    , m_mainLayout(nullptr)
    , m_drawingModeGroup(nullptr)
    , m_drawingModeButtonGroup(nullptr)
    , m_editModeGroup(nullptr)
    , m_editModeButtonGroup(nullptr)
    , m_toolsGroup(nullptr)
    , m_statusGroup(nullptr)
    , m_currentDrawingMode(DrawingMode::None)
    , m_currentEditMode(EditMode::None)
    , m_segmentCount(0)
    , m_selectedCount(0)
{
    setupUI();
    connectSignals();
    updateStatusLabels();
}

LineDrawingToolbar::~LineDrawingToolbar()
{
    // Qt会自动清理子对象
}

void LineDrawingToolbar::setLineDrawingTool(LineDrawingTool* tool)
{
    // 断开之前工具的信号连接
    if (m_lineDrawingTool) {
        disconnect(m_lineDrawingTool, nullptr, this, nullptr);
    }
    
    m_lineDrawingTool = tool;
    
    // 连接新工具的信号
    if (m_lineDrawingTool) {
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentAdded,
                this, &LineDrawingToolbar::onLineSegmentAdded);
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentRemoved,
                this, &LineDrawingToolbar::onLineSegmentRemoved);
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentSelected,
                this, &LineDrawingToolbar::onLineSegmentSelected);
        connect(m_lineDrawingTool, &LineDrawingTool::drawingModeChanged,
                this, &LineDrawingToolbar::onDrawingModeChanged);
    }
    
    updateStatus();
}

void LineDrawingToolbar::setupUI()
{
    // 设置主工具栏样式
    setObjectName("LineDrawingToolbar");
    setStyleSheet(getModernToolbarStyle());
    setMinimumHeight(120);  // 大幅增加最小高度以容纳更大的按钮
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 使用垂直布局来更好地组织内容
    QVBoxLayout* mainVerticalLayout = new QVBoxLayout(this);
    mainVerticalLayout->setContentsMargins(20, 16, 20, 16);  // 增加边距
    mainVerticalLayout->setSpacing(12);  // 增加间距

    // 创建工具栏标题
    QLabel* toolbarTitle = createStyledLabel("线段绘制工具", "toolbar-title");
    mainVerticalLayout->addWidget(toolbarTitle);

    // 创建主要工具区域
    QWidget* toolsArea = new QWidget();
    m_mainLayout = new QHBoxLayout(toolsArea);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(24);  // 大幅增加组件间距

    setupDrawingModeGroup();
    setupEditModeGroup();
    setupToolsGroup();
    setupStatusGroup();

    // 添加弹性空间
    m_mainLayout->addStretch();

    mainVerticalLayout->addWidget(toolsArea);
}

void LineDrawingToolbar::setupDrawingModeGroup()
{
    m_drawingModeGroup = new QGroupBox("绘制模式", this);
    m_drawingModeGroup->setObjectName("DrawingModeGroup");
    m_drawingModeGroup->setStyleSheet(getModernGroupBoxStyle());
    m_drawingModeGroup->setMinimumWidth(500);  // 增加宽度以容纳更大的按钮

    // 改为水平布局，单行排列，避免拥挤
    QHBoxLayout* layout = new QHBoxLayout(m_drawingModeGroup);
    layout->setContentsMargins(16, 20, 16, 16);  // 增加边距
    layout->setSpacing(12);  // 增加按钮间距

    m_drawingModeButtonGroup = new QButtonGroup(this);

    // 创建按钮并设置现代化样式，大幅增加按钮尺寸确保文字完全可见
    m_noneButton = createStyledButton("无", "mode-button xlarge");
    m_noneButton->setCheckable(true);
    m_noneButton->setChecked(true);
    m_noneButton->setMinimumSize(80, 40);  // 大幅增加尺寸
    m_noneButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_drawingModeButtonGroup->addButton(m_noneButton, static_cast<int>(DrawingMode::None));

    m_singleLineButton = createStyledButton("单线段", "mode-button xlarge");
    m_singleLineButton->setCheckable(true);
    m_singleLineButton->setMinimumSize(80, 40);
    m_singleLineButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_drawingModeButtonGroup->addButton(m_singleLineButton, static_cast<int>(DrawingMode::SingleLine));

    m_polylineButton = createStyledButton("多段线", "mode-button xlarge");
    m_polylineButton->setCheckable(true);
    m_polylineButton->setMinimumSize(80, 40);
    m_polylineButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_drawingModeButtonGroup->addButton(m_polylineButton, static_cast<int>(DrawingMode::Polyline));

    m_selectionButton = createStyledButton("选择", "mode-button xlarge");
    m_selectionButton->setCheckable(true);
    m_selectionButton->setMinimumSize(80, 40);
    m_selectionButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_drawingModeButtonGroup->addButton(m_selectionButton, static_cast<int>(DrawingMode::Selection));

    m_editButton = createStyledButton("编辑", "mode-button xlarge");
    m_editButton->setCheckable(true);
    m_editButton->setMinimumSize(80, 40);
    m_editButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_drawingModeButtonGroup->addButton(m_editButton, static_cast<int>(DrawingMode::Edit));

    // 水平排列所有按钮，确保有足够空间
    layout->addWidget(m_noneButton);
    layout->addWidget(m_singleLineButton);
    layout->addWidget(m_polylineButton);
    layout->addWidget(m_selectionButton);
    layout->addWidget(m_editButton);

    m_mainLayout->addWidget(m_drawingModeGroup);
}

void LineDrawingToolbar::setupEditModeGroup()
{
    m_editModeGroup = new QGroupBox("编辑模式", this);
    m_editModeGroup->setObjectName("EditModeGroup");
    m_editModeGroup->setStyleSheet(getModernGroupBoxStyle());

    m_editModeGroup->setMinimumWidth(350);  // 增加宽度

    QHBoxLayout* layout = new QHBoxLayout(m_editModeGroup);
    layout->setContentsMargins(16, 20, 16, 16);  // 增加边距
    layout->setSpacing(12);  // 增加间距

    m_editModeButtonGroup = new QButtonGroup(this);

    m_moveEndpointButton = createStyledButton("移动端点", "edit-button xlarge");
    m_moveEndpointButton->setCheckable(true);
    m_moveEndpointButton->setChecked(true);
    m_moveEndpointButton->setMinimumSize(90, 40);  // 4个字需要更宽
    m_moveEndpointButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_editModeButtonGroup->addButton(m_moveEndpointButton, static_cast<int>(EditMode::MoveEndpoint));

    m_splitSegmentButton = createStyledButton("分割线段", "edit-button xlarge");
    m_splitSegmentButton->setCheckable(true);
    m_splitSegmentButton->setMinimumSize(90, 40);
    m_splitSegmentButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_editModeButtonGroup->addButton(m_splitSegmentButton, static_cast<int>(EditMode::SplitSegment));

    m_mergeSegmentsButton = createStyledButton("合并线段", "edit-button xlarge");
    m_mergeSegmentsButton->setCheckable(true);
    m_mergeSegmentsButton->setMinimumSize(90, 40);
    m_mergeSegmentsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_editModeButtonGroup->addButton(m_mergeSegmentsButton, static_cast<int>(EditMode::MergeSegments));

    layout->addWidget(m_moveEndpointButton);
    layout->addWidget(m_splitSegmentButton);
    layout->addWidget(m_mergeSegmentsButton);

    // 默认禁用编辑模式组
    m_editModeGroup->setEnabled(false);

    m_mainLayout->addWidget(m_editModeGroup);
}

void LineDrawingToolbar::setupToolsGroup()
{
    m_toolsGroup = new QGroupBox("操作工具", this);
    m_toolsGroup->setObjectName("ToolsGroup");
    m_toolsGroup->setStyleSheet(getModernGroupBoxStyle());
    m_toolsGroup->setMinimumWidth(350);  // 增加宽度

    // 改为水平布局，单行排列所有工具按钮
    QHBoxLayout* layout = new QHBoxLayout(m_toolsGroup);
    layout->setContentsMargins(16, 20, 16, 16);  // 增加边距
    layout->setSpacing(12);  // 增加按钮间距

    // 创建更大的按钮，确保文字完全可见
    m_saveButton = createStyledButton("保存", "tool-button success xlarge");
    m_saveButton->setMinimumSize(80, 40);
    m_saveButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_loadButton = createStyledButton("加载", "tool-button primary xlarge");
    m_loadButton->setMinimumSize(80, 40);
    m_loadButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_deleteSelectedButton = createStyledButton("删除选中", "tool-button warning xlarge");
    m_deleteSelectedButton->setMinimumSize(90, 40);  // 稍微更宽以容纳4个字
    m_deleteSelectedButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_clearAllButton = createStyledButton("清空所有", "tool-button danger xlarge");
    m_clearAllButton->setMinimumSize(90, 40);
    m_clearAllButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 水平排列所有按钮
    layout->addWidget(m_saveButton);
    layout->addWidget(m_loadButton);
    layout->addWidget(m_deleteSelectedButton);
    layout->addWidget(m_clearAllButton);

    m_mainLayout->addWidget(m_toolsGroup);
}

void LineDrawingToolbar::setupStatusGroup()
{
    m_statusGroup = new QGroupBox("状态信息", this);
    m_statusGroup->setObjectName("StatusGroup");
    m_statusGroup->setStyleSheet(getModernGroupBoxStyle());
    m_statusGroup->setMinimumWidth(180);  // 增加宽度

    QVBoxLayout* layout = new QVBoxLayout(m_statusGroup);
    layout->setContentsMargins(16, 20, 16, 16);  // 增加边距
    layout->setSpacing(10);  // 增加间距

    // 创建状态信息容器，使用更大的尺寸
    QWidget* statusContainer = new QWidget();
    statusContainer->setStyleSheet(R"(
        QWidget {
            background-color: #f8f9fa;
            border: 2px solid #e9ecef;
            border-radius: 8px;
            padding: 12px;
        }
    )");

    QVBoxLayout* statusLayout = new QVBoxLayout(statusContainer);
    statusLayout->setContentsMargins(12, 12, 12, 12);  // 增加内边距
    statusLayout->setSpacing(8);  // 增加间距

    m_segmentCountLabel = createStyledLabel("线段数: 0", "status-label xlarge");
    m_selectedCountLabel = createStyledLabel("选中数: 0", "status-label xlarge");
    m_currentModeLabel = createStyledLabel("模式: 无", "status-label xlarge");

    statusLayout->addWidget(m_segmentCountLabel);
    statusLayout->addWidget(m_selectedCountLabel);
    statusLayout->addWidget(m_currentModeLabel);

    layout->addWidget(statusContainer);

    m_mainLayout->addWidget(m_statusGroup);
}

void LineDrawingToolbar::connectSignals()
{
    // 绘制模式按钮组信号
    connect(m_drawingModeButtonGroup, &QButtonGroup::idClicked,
            this, &LineDrawingToolbar::onDrawingModeButtonClicked);

    // 编辑模式按钮组信号
    connect(m_editModeButtonGroup, &QButtonGroup::idClicked,
            this, &LineDrawingToolbar::onEditModeButtonClicked);
    
    // 工具按钮信号
    connect(m_clearAllButton, &QPushButton::clicked, this, &LineDrawingToolbar::onClearAllLines);
    connect(m_deleteSelectedButton, &QPushButton::clicked, this, &LineDrawingToolbar::onDeleteSelected);
    connect(m_saveButton, &QPushButton::clicked, this, &LineDrawingToolbar::onSaveLines);
    connect(m_loadButton, &QPushButton::clicked, this, &LineDrawingToolbar::onLoadLines);
}

void LineDrawingToolbar::onDrawingModeButtonClicked(int modeId)
{
    DrawingMode mode = static_cast<DrawingMode>(modeId);

    m_currentDrawingMode = mode;

    // 启用/禁用编辑模式组
    m_editModeGroup->setEnabled(mode == DrawingMode::Edit);

    emit drawingModeChangeRequested(mode);
    updateStatusLabels();
}

void LineDrawingToolbar::onEditModeButtonClicked(int modeId)
{
    EditMode mode = static_cast<EditMode>(modeId);

    m_currentEditMode = mode;
    emit editModeChangeRequested(mode);
}

void LineDrawingToolbar::onClearAllLines()
{
    if (!m_lineDrawingTool) {
        return;
    }

    int segmentCount = m_lineDrawingTool->getLineSegments().size();
    if (segmentCount == 0) {
        QMessageBox::information(this, "提示", "没有线段需要清空");
        return;
    }

    int ret = QMessageBox::question(this, "确认清空",
                                   QString("确定要清空所有 %1 个线段吗？").arg(segmentCount),
                                   QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        m_lineDrawingTool->clearAllSegments();
        m_lineDrawingTool->clearAllPolylines();
        updateStatus();
    }
}

void LineDrawingToolbar::onDeleteSelected()
{
    if (!m_lineDrawingTool) {
        return;
    }

    std::vector<int> selectedIds = m_lineDrawingTool->getSelectedSegmentIdsVector();
    if (selectedIds.empty()) {
        QMessageBox::information(this, "提示", "没有选中的线段需要删除");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除",
                                   QString("确定要删除选中的 %1 个线段吗？").arg(selectedIds.size()),
                                   QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        m_lineDrawingTool->deleteSelectedSegments();
        updateStatus();
    }
}

void LineDrawingToolbar::onSaveLines()
{
    if (!m_lineDrawingTool) {
        return;
    }

    if (m_lineDrawingTool->getLineSegments().empty()) {
        QMessageBox::information(this, "提示", "没有线段数据需要保存");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                   "保存线段数据",
                                                   "",
                                                   "JSON文件 (*.json);;所有文件 (*)");

    if (!fileName.isEmpty()) {
        if (m_lineDrawingTool->saveToFile(fileName)) {
            QMessageBox::information(this, "成功", "线段数据已保存");
        } else {
            QMessageBox::warning(this, "错误", "保存线段数据失败");
        }
    }
}

void LineDrawingToolbar::onLoadLines()
{
    if (!m_lineDrawingTool) {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                                   "加载线段数据",
                                                   "",
                                                   "JSON文件 (*.json);;所有文件 (*)");

    if (!fileName.isEmpty()) {
        if (m_lineDrawingTool->loadFromFile(fileName)) {
            QMessageBox::information(this, "成功", "线段数据已加载");
            updateStatus();
        } else {
            QMessageBox::warning(this, "错误", "加载线段数据失败");
        }
    }
}

void LineDrawingToolbar::onLineSegmentAdded(const LineSegment& segment)
{
    Q_UNUSED(segment)
    updateStatus();
}

void LineDrawingToolbar::onLineSegmentRemoved(int segmentId)
{
    Q_UNUSED(segmentId)
    updateStatus();
}

void LineDrawingToolbar::onLineSegmentSelected(int segmentId)
{
    Q_UNUSED(segmentId)
    updateStatus();
}

void LineDrawingToolbar::onDrawingModeChanged()
{
    if (m_lineDrawingTool) {
        m_currentDrawingMode = m_lineDrawingTool->getDrawingMode();
        updateModeButtons();
        updateStatusLabels();
    }
}

void LineDrawingToolbar::onEditModeChanged()
{
    if (m_lineDrawingTool) {
        m_currentEditMode = m_lineDrawingTool->getEditMode();
        updateModeButtons();
    }
}

void LineDrawingToolbar::updateStatus()
{
    if (m_lineDrawingTool) {
        m_segmentCount = m_lineDrawingTool->getLineSegments().size();
        m_selectedCount = m_lineDrawingTool->getSelectedSegmentIds().size();
    } else {
        m_segmentCount = 0;
        m_selectedCount = 0;
    }

    updateStatusLabels();
}

void LineDrawingToolbar::updateModeButtons()
{
    // 更新绘制模式按钮
    QAbstractButton* drawingButton = m_drawingModeButtonGroup->button(static_cast<int>(m_currentDrawingMode));
    if (drawingButton) {
        drawingButton->setChecked(true);
    }

    // 更新编辑模式按钮
    QAbstractButton* editButton = m_editModeButtonGroup->button(static_cast<int>(m_currentEditMode));
    if (editButton) {
        editButton->setChecked(true);
    }

    // 启用/禁用编辑模式组
    m_editModeGroup->setEnabled(m_currentDrawingMode == DrawingMode::Edit);
}

void LineDrawingToolbar::updateStatusLabels()
{
    m_segmentCountLabel->setText(QString("线段数: %1").arg(m_segmentCount));
    m_selectedCountLabel->setText(QString("选中数: %1").arg(m_selectedCount));

    QString modeText;
    switch (m_currentDrawingMode) {
        case DrawingMode::None: modeText = "无"; break;
        case DrawingMode::SingleLine: modeText = "单线段"; break;
        case DrawingMode::Polyline: modeText = "多段线"; break;
        case DrawingMode::Selection: modeText = "选择"; break;
        case DrawingMode::Edit: modeText = "编辑"; break;
    }

    m_currentModeLabel->setText(QString("模式: %1").arg(modeText));
}

// ==================== 现代化样式方法 ====================

QString LineDrawingToolbar::getModernToolbarStyle() const
{
    return R"(
        #LineDrawingToolbar {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                      stop: 0 #ffffff, stop: 1 #f8f9fa);
            border: 2px solid #dee2e6;
            border-radius: 12px;
            padding: 8px;
        }
        QLabel[objectName="toolbar-title"] {
            font-size: 14px;
            font-weight: 600;
            color: #495057;
            padding: 4px 8px;
            background-color: transparent;
            border: none;
        }
    )";
}

QString LineDrawingToolbar::getModernGroupBoxStyle() const
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
            background-color: #f8f9fa;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 8px;
            padding: 0 4px 0 4px;
            background-color: #f8f9fa;
        }
    )";
}

QPushButton* LineDrawingToolbar::createStyledButton(const QString& text, const QString& styleClass)
{
    QPushButton* button = new QPushButton(text, this);
    button->setObjectName(styleClass);

    // 基础样式，支持多种尺寸
    QString fontSize, padding, minWidth, minHeight;

    if (styleClass.contains("xlarge")) {
        fontSize = "14px";      // 更大的字体
        padding = "12px 20px";  // 更大的内边距
        minWidth = "100px";     // 更宽的最小宽度
        minHeight = "40px";     // 更高的最小高度
    } else if (styleClass.contains("large")) {
        fontSize = "12px";
        padding = "8px 16px";
        minWidth = "80px";
        minHeight = "32px";
    } else {
        fontSize = "11px";
        padding = "6px 12px";
        minWidth = "60px";
        minHeight = "24px";
    }

    QString baseStyle = QString(R"(
        QPushButton {
            font-size: %1;
            font-weight: 500;
            padding: %2;
            border: 2px solid #bdc3c7;
            border-radius: 6px;
            background-color: #ffffff;
            color: #2c3e50;
            min-width: %3;
            min-height: %4;
        }
        QPushButton:hover {
            background-color: #ecf0f1;
            border-color: #95a5a6;
            transform: translateY(-1px);
        }
        QPushButton:pressed {
            background-color: #d5dbdb;
            transform: translateY(0px);
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
    )").arg(fontSize, padding, minWidth, minHeight);

    // 根据样式类添加特定颜色
    if (styleClass.contains("danger")) {
        baseStyle += R"(
            QPushButton {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #e74c3c, stop: 1 #c0392b);
                color: white;
                border-color: #a93226;
            }
            QPushButton:hover {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #c0392b, stop: 1 #a93226);
            }
        )";
    } else if (styleClass.contains("warning")) {
        baseStyle += R"(
            QPushButton {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #f39c12, stop: 1 #e67e22);
                color: white;
                border-color: #d68910;
            }
            QPushButton:hover {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #e67e22, stop: 1 #d68910);
            }
        )";
    } else if (styleClass.contains("success")) {
        baseStyle += R"(
            QPushButton {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #27ae60, stop: 1 #229954);
                color: white;
                border-color: #1e8449;
            }
            QPushButton:hover {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #229954, stop: 1 #1e8449);
            }
        )";
    } else if (styleClass.contains("primary")) {
        baseStyle += R"(
            QPushButton {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #3498db, stop: 1 #2980b9);
                color: white;
                border-color: #2471a3;
            }
            QPushButton:hover {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                          stop: 0 #2980b9, stop: 1 #2471a3);
            }
        )";
    }

    button->setStyleSheet(baseStyle);
    return button;
}

QLabel* LineDrawingToolbar::createStyledLabel(const QString& text, const QString& styleClass)
{
    QLabel* label = new QLabel(text, this);
    label->setObjectName(styleClass);

    QString fontSize, fontWeight, padding;

    if (styleClass.contains("xlarge")) {
        fontSize = "13px";      // 更大的字体
        fontWeight = "600";     // 更粗的字体
        padding = "6px 8px";    // 更大的内边距
    } else if (styleClass.contains("large")) {
        fontSize = "12px";
        fontWeight = "500";
        padding = "4px 6px";
    } else {
        fontSize = "11px";
        fontWeight = "500";
        padding = "2px 4px";
    }

    if (styleClass.contains("toolbar-title")) {
        fontWeight = "600";
    }

    QString style = QString(R"(
        QLabel {
            font-size: %1;
            font-weight: %2;
            color: #34495e;
            padding: %3;
            background-color: transparent;
        }
    )").arg(fontSize, fontWeight, padding);

    if (styleClass.contains("status")) {
        style += R"(
            QLabel {
                color: #495057;
                background-color: rgba(52, 73, 94, 0.05);
                border-radius: 3px;
                margin: 1px 0px;
            }
        )";
    }

    label->setStyleSheet(style);
    return label;
}

} // namespace WallExtraction
