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
    setMinimumHeight(150);  // 减少高度，因为移除了标题
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 使用垂直布局来组织所有内容
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 8, 12, 8);  // 进一步减少边距
    m_mainLayout->setSpacing(6);  // 减少间距，使布局更紧凑

    // 移除标题，直接排列所有组件
    setupDrawingModeGroup();
    setupEditModeGroup();
    setupToolsGroup();
    setupStatusGroup();

    // 添加少量弹性空间
    m_mainLayout->addStretch(1);
}

void LineDrawingToolbar::setupDrawingModeGroup()
{
    m_drawingModeGroup = new QGroupBox("绘制模式", this);
    m_drawingModeGroup->setObjectName("DrawingModeGroup");
    m_drawingModeGroup->setStyleSheet(getModernGroupBoxStyle());
    m_drawingModeGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 改为水平布局，单行排列
    QHBoxLayout* layout = new QHBoxLayout(m_drawingModeGroup);
    layout->setContentsMargins(8, 12, 8, 8);  // 减少边距
    layout->setSpacing(4);  // 减少按钮间距

    m_drawingModeButtonGroup = new QButtonGroup(this);

    // 创建按钮并设置适合水平排列的尺寸
    m_noneButton = createStyledButton("无", "mode-button compact");
    m_noneButton->setCheckable(true);
    m_noneButton->setChecked(true);
    m_noneButton->setMinimumSize(40, 28);  // 适合水平排列的尺寸
    m_noneButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_drawingModeButtonGroup->addButton(m_noneButton, static_cast<int>(DrawingMode::None));

    m_singleLineButton = createStyledButton("单线段", "mode-button compact");
    m_singleLineButton->setCheckable(true);
    m_singleLineButton->setMinimumSize(60, 28);  // 3个字需要稍宽
    m_singleLineButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_drawingModeButtonGroup->addButton(m_singleLineButton, static_cast<int>(DrawingMode::SingleLine));

    m_polylineButton = createStyledButton("多段线", "mode-button compact");
    m_polylineButton->setCheckable(true);
    m_polylineButton->setMinimumSize(60, 28);
    m_polylineButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_drawingModeButtonGroup->addButton(m_polylineButton, static_cast<int>(DrawingMode::Polyline));

    m_selectionButton = createStyledButton("选择", "mode-button compact");
    m_selectionButton->setCheckable(true);
    m_selectionButton->setMinimumSize(40, 28);
    m_selectionButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_drawingModeButtonGroup->addButton(m_selectionButton, static_cast<int>(DrawingMode::Selection));

    m_editButton = createStyledButton("编辑", "mode-button compact");
    m_editButton->setCheckable(true);
    m_editButton->setMinimumSize(40, 28);
    m_editButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_drawingModeButtonGroup->addButton(m_editButton, static_cast<int>(DrawingMode::Edit));

    // 水平排列所有按钮
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
    m_editModeGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 改为水平布局，单行排列
    QHBoxLayout* layout = new QHBoxLayout(m_editModeGroup);
    layout->setContentsMargins(8, 12, 8, 8);  // 减少边距
    layout->setSpacing(4);  // 减少间距

    m_editModeButtonGroup = new QButtonGroup(this);

    m_moveEndpointButton = createStyledButton("移动端点", "edit-button compact");
    m_moveEndpointButton->setCheckable(true);
    m_moveEndpointButton->setChecked(true);
    m_moveEndpointButton->setMinimumSize(70, 28);  // 4个字适合水平排列
    m_moveEndpointButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_editModeButtonGroup->addButton(m_moveEndpointButton, static_cast<int>(EditMode::MoveEndpoint));

    m_splitSegmentButton = createStyledButton("分割线段", "edit-button compact");
    m_splitSegmentButton->setCheckable(true);
    m_splitSegmentButton->setMinimumSize(70, 28);
    m_splitSegmentButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_editModeButtonGroup->addButton(m_splitSegmentButton, static_cast<int>(EditMode::SplitSegment));

    m_mergeSegmentsButton = createStyledButton("合并线段", "edit-button compact");
    m_mergeSegmentsButton->setCheckable(true);
    m_mergeSegmentsButton->setMinimumSize(70, 28);
    m_mergeSegmentsButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_editModeButtonGroup->addButton(m_mergeSegmentsButton, static_cast<int>(EditMode::MergeSegments));

    layout->addWidget(m_moveEndpointButton);
    layout->addWidget(m_splitSegmentButton);
    layout->addWidget(m_mergeSegmentsButton);
    layout->addStretch();

    // 默认禁用编辑模式组
    m_editModeGroup->setEnabled(false);

    m_mainLayout->addWidget(m_editModeGroup);
}

void LineDrawingToolbar::setupToolsGroup()
{
    m_toolsGroup = new QGroupBox("操作工具", this);
    m_toolsGroup->setObjectName("ToolsGroup");
    m_toolsGroup->setStyleSheet(getModernGroupBoxStyle());
    m_toolsGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 改为水平布局，单行排列
    QHBoxLayout* layout = new QHBoxLayout(m_toolsGroup);
    layout->setContentsMargins(8, 12, 8, 8);  // 减少边距
    layout->setSpacing(4);  // 减少按钮间距

    // 创建适合水平排列的按钮
    m_saveButton = createStyledButton("保存", "tool-button success compact");
    m_saveButton->setMinimumSize(50, 28);
    m_saveButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_loadButton = createStyledButton("加载", "tool-button primary compact");
    m_loadButton->setMinimumSize(50, 28);
    m_loadButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_deleteSelectedButton = createStyledButton("删除选中", "tool-button warning compact");
    m_deleteSelectedButton->setMinimumSize(70, 28);  // 4个字适合水平排列
    m_deleteSelectedButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_clearAllButton = createStyledButton("清空所有", "tool-button danger compact");
    m_clearAllButton->setMinimumSize(70, 28);
    m_clearAllButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

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
    m_statusGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout* layout = new QVBoxLayout(m_statusGroup);
    layout->setContentsMargins(8, 12, 8, 8);  // 减少边距
    layout->setSpacing(4);  // 减少间距

    // 创建状态信息容器，使用紧凑尺寸
    QWidget* statusContainer = new QWidget();
    statusContainer->setStyleSheet(R"(
        QWidget {
            background-color: #f8f9fa;
            border: 1px solid #e9ecef;
            border-radius: 6px;
            padding: 6px;
        }
    )");

    // 改为水平布局，单行排列状态信息
    QHBoxLayout* statusLayout = new QHBoxLayout(statusContainer);
    statusLayout->setContentsMargins(6, 6, 6, 6);  // 减少内边距
    statusLayout->setSpacing(12);  // 状态项之间的间距

    m_segmentCountLabel = createStyledLabel("线段数: 0", "status-label compact");
    m_selectedCountLabel = createStyledLabel("选中数: 0", "status-label compact");
    m_currentModeLabel = createStyledLabel("模式: 无", "status-label compact");

    statusLayout->addWidget(m_segmentCountLabel);
    statusLayout->addWidget(m_selectedCountLabel);
    statusLayout->addWidget(m_currentModeLabel);
    statusLayout->addStretch();  // 添加弹性空间

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
    } else if (styleClass.contains("compact")) {
        fontSize = "12px";      // 紧凑但清晰的字体
        padding = "6px 10px";   // 紧凑的内边距
        minWidth = "50px";      // 最小宽度
        minHeight = "28px";     // 适中的高度
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
    } else if (styleClass.contains("compact")) {
        fontSize = "11px";      // 紧凑但清晰的字体
        fontWeight = "500";     // 适中的字体粗细
        padding = "3px 5px";    // 紧凑的内边距
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
