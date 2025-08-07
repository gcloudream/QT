#include "line_list_widget.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>

namespace WallExtraction {

LineListWidget::LineListWidget(QWidget* parent)
    : QWidget(parent)
    , m_lineDrawingTool(nullptr)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_filterGroup(nullptr)
    , m_tableWidget(nullptr)
    , m_contextMenu(nullptr)
    , m_currentSortCriteria(SortCriteria::ById)
    , m_sortAscending(true)
    , m_contextMenuSegmentId(-1)
{
    setupUI();
    connectSignals();
    setupContextMenu();
    updateList();
}

LineListWidget::~LineListWidget()
{
    // Qt会自动清理子对象
}

void LineListWidget::setLineDrawingTool(LineDrawingTool* tool)
{
    // 断开之前工具的信号连接
    if (m_lineDrawingTool) {
        disconnect(m_lineDrawingTool, nullptr, this, nullptr);
    }
    
    m_lineDrawingTool = tool;
    
    // 连接新工具的信号
    if (m_lineDrawingTool) {
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentAdded,
                this, &LineListWidget::onLineSegmentAdded);
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentRemoved,
                this, &LineListWidget::onLineSegmentRemoved);
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentSelected,
                this, &LineListWidget::onLineSegmentSelected);
    }
    
    updateList();
}

void LineListWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);
    m_mainLayout->setSpacing(8);
    
    m_splitter = new QSplitter(Qt::Vertical, this);
    
    setupFilterGroup();
    setupTableWidget();
    
    m_splitter->addWidget(m_filterGroup);
    m_splitter->addWidget(m_tableWidget);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    
    m_mainLayout->addWidget(m_splitter);
}

void LineListWidget::setupFilterGroup()
{
    m_filterGroup = new QGroupBox("过滤和排序", this);
    QVBoxLayout* mainLayout = new QVBoxLayout(m_filterGroup);
    
    // 排序控件
    QHBoxLayout* sortLayout = new QHBoxLayout();
    sortLayout->addWidget(new QLabel("排序:", this));
    
    m_sortComboBox = new QComboBox(this);
    m_sortComboBox->addItem("按ID", static_cast<int>(SortCriteria::ById));
    m_sortComboBox->addItem("按长度", static_cast<int>(SortCriteria::ByLength));
    m_sortComboBox->addItem("按创建时间", static_cast<int>(SortCriteria::ByCreatedTime));
    m_sortComboBox->addItem("按选择状态", static_cast<int>(SortCriteria::BySelection));
    
    m_ascendingCheckBox = new QCheckBox("升序", this);
    m_ascendingCheckBox->setChecked(true);
    
    sortLayout->addWidget(m_sortComboBox);
    sortLayout->addWidget(m_ascendingCheckBox);
    sortLayout->addStretch();
    
    // 过滤控件
    QHBoxLayout* filterLayout1 = new QHBoxLayout();
    filterLayout1->addWidget(new QLabel("描述包含:", this));
    m_descriptionFilterEdit = new QLineEdit(this);
    m_descriptionFilterEdit->setPlaceholderText("输入描述关键词...");
    filterLayout1->addWidget(m_descriptionFilterEdit);
    
    QHBoxLayout* filterLayout2 = new QHBoxLayout();
    filterLayout2->addWidget(new QLabel("长度范围:", this));
    m_minLengthSpinBox = new QDoubleSpinBox(this);
    m_minLengthSpinBox->setRange(0.0, 999999.0);
    m_minLengthSpinBox->setDecimals(3);
    m_minLengthSpinBox->setSuffix(" 单位");
    filterLayout2->addWidget(m_minLengthSpinBox);
    
    filterLayout2->addWidget(new QLabel("到", this));
    m_maxLengthSpinBox = new QDoubleSpinBox(this);
    m_maxLengthSpinBox->setRange(0.0, 999999.0);
    m_maxLengthSpinBox->setDecimals(3);
    m_maxLengthSpinBox->setValue(999999.0);
    m_maxLengthSpinBox->setSuffix(" 单位");
    filterLayout2->addWidget(m_maxLengthSpinBox);
    
    QHBoxLayout* filterLayout3 = new QHBoxLayout();
    m_showSelectedCheckBox = new QCheckBox("显示选中", this);
    m_showSelectedCheckBox->setChecked(true);
    m_showUnselectedCheckBox = new QCheckBox("显示未选中", this);
    m_showUnselectedCheckBox->setChecked(true);
    m_resetFilterButton = new QPushButton("重置过滤器", this);
    
    filterLayout3->addWidget(m_showSelectedCheckBox);
    filterLayout3->addWidget(m_showUnselectedCheckBox);
    filterLayout3->addStretch();
    filterLayout3->addWidget(m_resetFilterButton);
    
    mainLayout->addLayout(sortLayout);
    mainLayout->addLayout(filterLayout1);
    mainLayout->addLayout(filterLayout2);
    mainLayout->addLayout(filterLayout3);
}

void LineListWidget::setupTableWidget()
{
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(COL_COUNT);
    
    // 设置表头
    QStringList headers;
    headers << "ID" << "起点" << "终点" << "长度" << "多段线ID" << "创建时间" << "描述";
    m_tableWidget->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setSortingEnabled(false); // 我们自己处理排序
    m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // 设置列宽
    QHeaderView* header = m_tableWidget->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(COL_ID, 60);
    header->resizeSection(COL_START_POINT, 120);
    header->resizeSection(COL_END_POINT, 120);
    header->resizeSection(COL_LENGTH, 80);
    header->resizeSection(COL_POLYLINE_ID, 80);
    header->resizeSection(COL_CREATED_TIME, 140);
}

void LineListWidget::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_selectAction = m_contextMenu->addAction("选择线段");
    m_deleteAction = m_contextMenu->addAction("删除线段");
    m_contextMenu->addSeparator();
    m_editDescriptionAction = m_contextMenu->addAction("编辑描述");
    
    connect(m_selectAction, &QAction::triggered, this, &LineListWidget::onSelectSegment);
    connect(m_deleteAction, &QAction::triggered, this, &LineListWidget::onDeleteSegment);
    connect(m_editDescriptionAction, &QAction::triggered, this, &LineListWidget::onEditDescription);
}

void LineListWidget::connectSignals()
{
    // 过滤和排序信号
    connect(m_sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LineListWidget::onSortCriteriaChanged);
    connect(m_ascendingCheckBox, &QCheckBox::toggled,
            this, &LineListWidget::onSortCriteriaChanged);
    
    connect(m_descriptionFilterEdit, &QLineEdit::textChanged,
            this, &LineListWidget::onFilterChanged);
    connect(m_minLengthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &LineListWidget::onFilterChanged);
    connect(m_maxLengthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &LineListWidget::onFilterChanged);
    connect(m_showSelectedCheckBox, &QCheckBox::toggled,
            this, &LineListWidget::onFilterChanged);
    connect(m_showUnselectedCheckBox, &QCheckBox::toggled,
            this, &LineListWidget::onFilterChanged);
    connect(m_resetFilterButton, &QPushButton::clicked,
            this, &LineListWidget::onResetFilter);
    
    // 表格信号
    connect(m_tableWidget, &QTableWidget::itemSelectionChanged,
            this, &LineListWidget::onTableItemSelectionChanged);
    connect(m_tableWidget, &QTableWidget::itemDoubleClicked,
            this, &LineListWidget::onTableItemDoubleClicked);
    connect(m_tableWidget, &QTableWidget::customContextMenuRequested,
            this, &LineListWidget::onTableContextMenuRequested);
}

// 响应线段工具信号的槽函数
void LineListWidget::onLineSegmentAdded(const LineSegment& segment)
{
    Q_UNUSED(segment)
    updateList();
}

void LineListWidget::onLineSegmentRemoved(int segmentId)
{
    Q_UNUSED(segmentId)
    updateList();
}

void LineListWidget::onLineSegmentSelected(int segmentId)
{
    Q_UNUSED(segmentId)
    updateTableSelection();
}

// 排序和过滤槽函数
void LineListWidget::onSortCriteriaChanged()
{
    int criteriaIndex = m_sortComboBox->currentData().toInt();
    m_currentSortCriteria = static_cast<SortCriteria>(criteriaIndex);
    m_sortAscending = m_ascendingCheckBox->isChecked();

    populateTable();
}

void LineListWidget::onFilterChanged()
{
    // 更新过滤条件
    m_currentFilter.descriptionFilter = m_descriptionFilterEdit->text();
    m_currentFilter.minLength = static_cast<float>(m_minLengthSpinBox->value());
    m_currentFilter.maxLength = static_cast<float>(m_maxLengthSpinBox->value());
    m_currentFilter.showSelected = m_showSelectedCheckBox->isChecked();
    m_currentFilter.showUnselected = m_showUnselectedCheckBox->isChecked();

    populateTable();
}

void LineListWidget::onResetFilter()
{
    // 重置所有过滤器
    m_descriptionFilterEdit->clear();
    m_minLengthSpinBox->setValue(0.0);
    m_maxLengthSpinBox->setValue(999999.0);
    m_showSelectedCheckBox->setChecked(true);
    m_showUnselectedCheckBox->setChecked(true);

    // 重置排序
    m_sortComboBox->setCurrentIndex(0);
    m_ascendingCheckBox->setChecked(true);

    onFilterChanged();
}

// 表格操作槽函数
void LineListWidget::onTableItemSelectionChanged()
{
    // 这里可以添加选择变化的处理逻辑
}

void LineListWidget::onTableItemDoubleClicked(QTableWidgetItem* item)
{
    if (!item) return;

    int row = item->row();
    QTableWidgetItem* idItem = m_tableWidget->item(row, COL_ID);
    if (idItem) {
        int segmentId = idItem->text().toInt();
        emit segmentSelectionRequested(segmentId);
    }
}

void LineListWidget::onTableContextMenuRequested(const QPoint& pos)
{
    QTableWidgetItem* item = m_tableWidget->itemAt(pos);
    if (!item) return;

    int row = item->row();
    QTableWidgetItem* idItem = m_tableWidget->item(row, COL_ID);
    if (idItem) {
        m_contextMenuSegmentId = idItem->text().toInt();
        m_contextMenu->exec(m_tableWidget->mapToGlobal(pos));
    }
}

// 上下文菜单操作
void LineListWidget::onSelectSegment()
{
    if (m_contextMenuSegmentId != -1) {
        emit segmentSelectionRequested(m_contextMenuSegmentId);
    }
}

void LineListWidget::onDeleteSegment()
{
    if (m_contextMenuSegmentId != -1) {
        int ret = QMessageBox::question(this, "确认删除",
                                       QString("确定要删除线段 %1 吗？").arg(m_contextMenuSegmentId),
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            emit segmentDeletionRequested(m_contextMenuSegmentId);
        }
    }
}

void LineListWidget::onEditDescription()
{
    if (m_contextMenuSegmentId != -1) {
        emit segmentDescriptionEditRequested(m_contextMenuSegmentId);
    }
}

void LineListWidget::updateList()
{
    populateTable();
}

void LineListWidget::refreshList()
{
    updateList();
}

void LineListWidget::populateTable()
{
    if (!m_lineDrawingTool) {
        m_tableWidget->setRowCount(0);
        return;
    }

    // 获取排序后的线段信息
    m_currentSegmentInfos = m_lineDrawingTool->getSortedSegmentInfoList(m_currentSortCriteria, m_sortAscending);

    // 应用过滤器
    std::vector<LineSegmentInfo> filteredInfos;
    for (const auto& info : m_currentSegmentInfos) {
        // 长度过滤
        if (info.length < m_currentFilter.minLength || info.length > m_currentFilter.maxLength) {
            continue;
        }

        // 选择状态过滤
        if (info.isSelected && !m_currentFilter.showSelected) {
            continue;
        }
        if (!info.isSelected && !m_currentFilter.showUnselected) {
            continue;
        }

        // 描述过滤
        if (!m_currentFilter.descriptionFilter.isEmpty() &&
            !info.description.contains(m_currentFilter.descriptionFilter, Qt::CaseInsensitive)) {
            continue;
        }

        filteredInfos.push_back(info);
    }

    // 更新表格
    m_tableWidget->setRowCount(filteredInfos.size());

    for (size_t i = 0; i < filteredInfos.size(); ++i) {
        const LineSegmentInfo& info = filteredInfos[i];
        int row = static_cast<int>(i);

        // ID列
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(info.id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_tableWidget->setItem(row, COL_ID, idItem);

        // 起点列
        QString startPointText = QString("(%.2f, %.2f, %.2f)")
                                .arg(info.startPoint.x())
                                .arg(info.startPoint.y())
                                .arg(info.startPoint.z());
        QTableWidgetItem* startItem = new QTableWidgetItem(startPointText);
        startItem->setFlags(startItem->flags() & ~Qt::ItemIsEditable);
        m_tableWidget->setItem(row, COL_START_POINT, startItem);

        // 终点列
        QString endPointText = QString("(%.2f, %.2f, %.2f)")
                              .arg(info.endPoint.x())
                              .arg(info.endPoint.y())
                              .arg(info.endPoint.z());
        QTableWidgetItem* endItem = new QTableWidgetItem(endPointText);
        endItem->setFlags(endItem->flags() & ~Qt::ItemIsEditable);
        m_tableWidget->setItem(row, COL_END_POINT, endItem);

        // 长度列
        QTableWidgetItem* lengthItem = new QTableWidgetItem(QString::number(info.length, 'f', 3));
        lengthItem->setFlags(lengthItem->flags() & ~Qt::ItemIsEditable);
        m_tableWidget->setItem(row, COL_LENGTH, lengthItem);

        // 多段线ID列
        QString polylineText = (info.polylineId != -1) ? QString::number(info.polylineId) : "独立";
        QTableWidgetItem* polylineItem = new QTableWidgetItem(polylineText);
        polylineItem->setFlags(polylineItem->flags() & ~Qt::ItemIsEditable);
        m_tableWidget->setItem(row, COL_POLYLINE_ID, polylineItem);

        // 创建时间列
        QString timeText = info.createdTime.toString("yyyy-MM-dd hh:mm:ss");
        QTableWidgetItem* timeItem = new QTableWidgetItem(timeText);
        timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
        m_tableWidget->setItem(row, COL_CREATED_TIME, timeItem);

        // 描述列
        QTableWidgetItem* descItem = new QTableWidgetItem(info.description);
        m_tableWidget->setItem(row, COL_DESCRIPTION, descItem);

        // 设置选中行的背景色
        if (info.isSelected) {
            for (int col = 0; col < COL_COUNT; ++col) {
                QTableWidgetItem* item = m_tableWidget->item(row, col);
                if (item) {
                    item->setBackground(QColor(200, 230, 255)); // 浅蓝色背景
                }
            }
        }
    }

    updateTableSelection();
}

void LineListWidget::updateTableSelection()
{
    if (!m_lineDrawingTool) {
        return;
    }

    std::vector<int> selectedIds = m_lineDrawingTool->getSelectedSegmentIdsVector();

    // 清除当前选择
    m_tableWidget->clearSelection();

    // 选择对应的行
    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        QTableWidgetItem* idItem = m_tableWidget->item(row, COL_ID);
        if (idItem) {
            int segmentId = idItem->text().toInt();
            auto it = std::find(selectedIds.begin(), selectedIds.end(), segmentId);
            if (it != selectedIds.end()) {
                m_tableWidget->selectRow(row);
            }
        }
    }
}

} // namespace WallExtraction
