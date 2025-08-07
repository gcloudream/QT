#include "line_info_panel.h"
#include <QDebug>
#include <QMessageBox>
#include <algorithm>
#include <numeric>

namespace WallExtraction {

LineInfoPanel::LineInfoPanel(QWidget* parent)
    : QWidget(parent)
    , m_lineDrawingTool(nullptr)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_selectedSegmentGroup(nullptr)
    , m_statisticsGroup(nullptr)
    , m_currentSelectedSegmentId(-1)
    , m_descriptionModified(false)
{
    setupUI();
    connectSignals();
    updateDisplay();
}

LineInfoPanel::~LineInfoPanel()
{
    // Qt会自动清理子对象
}

void LineInfoPanel::setLineDrawingTool(LineDrawingTool* tool)
{
    // 断开之前工具的信号连接
    if (m_lineDrawingTool) {
        disconnect(m_lineDrawingTool, nullptr, this, nullptr);
    }
    
    m_lineDrawingTool = tool;
    
    // 连接新工具的信号
    if (m_lineDrawingTool) {
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentAdded,
                this, &LineInfoPanel::onLineSegmentAdded);
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentRemoved,
                this, &LineInfoPanel::onLineSegmentRemoved);
        connect(m_lineDrawingTool, &LineDrawingTool::lineSegmentSelected,
                this, &LineInfoPanel::onLineSegmentSelected);
    }
    
    updateDisplay();
}

void LineInfoPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);
    m_mainLayout->setSpacing(8);
    
    m_splitter = new QSplitter(Qt::Vertical, this);
    
    setupSelectedSegmentGroup();
    setupStatisticsGroup();
    
    m_splitter->addWidget(m_selectedSegmentGroup);
    m_splitter->addWidget(m_statisticsGroup);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);
    
    m_mainLayout->addWidget(m_splitter);
}

void LineInfoPanel::setupSelectedSegmentGroup()
{
    m_selectedSegmentGroup = new QGroupBox("选中线段信息", this);
    QVBoxLayout* layout = new QVBoxLayout(m_selectedSegmentGroup);
    
    // 基本信息
    m_segmentIdLabel = new QLabel("ID: 无", this);
    m_startPointLabel = new QLabel("起点: 无", this);
    m_endPointLabel = new QLabel("终点: 无", this);
    m_lengthLabel = new QLabel("长度: 无", this);
    m_polylineIdLabel = new QLabel("多段线ID: 无", this);
    m_createdTimeLabel = new QLabel("创建时间: 无", this);
    
    layout->addWidget(m_segmentIdLabel);
    layout->addWidget(m_startPointLabel);
    layout->addWidget(m_endPointLabel);
    layout->addWidget(m_lengthLabel);
    layout->addWidget(m_polylineIdLabel);
    layout->addWidget(m_createdTimeLabel);
    
    // 描述编辑
    QLabel* descLabel = new QLabel("描述:", this);
    m_descriptionEdit = new QLineEdit(this);
    m_descriptionEdit->setPlaceholderText("输入线段描述...");
    m_saveDescriptionButton = new QPushButton("保存描述", this);
    m_saveDescriptionButton->setEnabled(false);
    
    QHBoxLayout* descLayout = new QHBoxLayout();
    descLayout->addWidget(m_descriptionEdit);
    descLayout->addWidget(m_saveDescriptionButton);
    
    layout->addWidget(descLabel);
    layout->addLayout(descLayout);
    
    // 添加弹性空间
    layout->addStretch();
}

void LineInfoPanel::setupStatisticsGroup()
{
    m_statisticsGroup = new QGroupBox("统计信息", this);
    QVBoxLayout* layout = new QVBoxLayout(m_statisticsGroup);
    
    m_totalSegmentsLabel = new QLabel("总线段数: 0", this);
    m_selectedSegmentsLabel = new QLabel("选中线段数: 0", this);
    m_totalPolylinesLabel = new QLabel("总多段线数: 0", this);
    m_totalLengthLabel = new QLabel("总长度: 0.00", this);
    m_averageLengthLabel = new QLabel("平均长度: 0.00", this);
    m_minLengthLabel = new QLabel("最短长度: 0.00", this);
    m_maxLengthLabel = new QLabel("最长长度: 0.00", this);
    
    layout->addWidget(m_totalSegmentsLabel);
    layout->addWidget(m_selectedSegmentsLabel);
    layout->addWidget(m_totalPolylinesLabel);
    layout->addWidget(m_totalLengthLabel);
    layout->addWidget(m_averageLengthLabel);
    layout->addWidget(m_minLengthLabel);
    layout->addWidget(m_maxLengthLabel);
    
    // 添加弹性空间
    layout->addStretch();
}

void LineInfoPanel::connectSignals()
{
    connect(m_descriptionEdit, &QLineEdit::textChanged,
            this, &LineInfoPanel::onDescriptionChanged);
    
    connect(m_saveDescriptionButton, &QPushButton::clicked,
            this, &LineInfoPanel::onSaveDescription);
}

void LineInfoPanel::onLineSegmentSelected(int segmentId)
{
    m_currentSelectedSegmentId = segmentId;
    updateSelectedSegmentInfo();
    updateStatistics();
}

void LineInfoPanel::onLineSegmentAdded(const LineSegment& segment)
{
    Q_UNUSED(segment)
    updateStatistics();
}

void LineInfoPanel::onLineSegmentRemoved(int segmentId)
{
    if (m_currentSelectedSegmentId == segmentId) {
        m_currentSelectedSegmentId = -1;
        clearSelectedSegmentInfo();
    }
    updateStatistics();
}

void LineInfoPanel::onDescriptionChanged()
{
    m_descriptionModified = true;
    m_saveDescriptionButton->setEnabled(true);
}

void LineInfoPanel::onSaveDescription()
{
    if (!m_lineDrawingTool || m_currentSelectedSegmentId == -1) {
        return;
    }
    
    QString description = m_descriptionEdit->text();
    if (m_lineDrawingTool->updateSegmentDescription(m_currentSelectedSegmentId, description)) {
        m_descriptionModified = false;
        m_saveDescriptionButton->setEnabled(false);
        emit segmentDescriptionChanged(m_currentSelectedSegmentId, description);
        QMessageBox::information(this, "成功", "线段描述已保存");
    } else {
        QMessageBox::warning(this, "错误", "保存线段描述失败");
    }
}

void LineInfoPanel::updateDisplay()
{
    updateSelectedSegmentInfo();
    updateStatistics();
}

void LineInfoPanel::updateSelectedSegmentInfo()
{
    if (!m_lineDrawingTool || m_currentSelectedSegmentId == -1) {
        clearSelectedSegmentInfo();
        return;
    }
    
    LineSegmentInfo info = m_lineDrawingTool->getSegmentInfo(m_currentSelectedSegmentId);
    if (info.id == -1) {
        clearSelectedSegmentInfo();
        return;
    }
    
    // 更新基本信息
    m_segmentIdLabel->setText(QString("ID: %1").arg(info.id));
    m_startPointLabel->setText(QString("起点: (%.2f, %.2f, %.2f)")
                              .arg(info.startPoint.x())
                              .arg(info.startPoint.y())
                              .arg(info.startPoint.z()));
    m_endPointLabel->setText(QString("终点: (%.2f, %.2f, %.2f)")
                            .arg(info.endPoint.x())
                            .arg(info.endPoint.y())
                            .arg(info.endPoint.z()));
    m_lengthLabel->setText(QString("长度: %.3f").arg(info.length));
    
    if (info.polylineId != -1) {
        m_polylineIdLabel->setText(QString("多段线ID: %1").arg(info.polylineId));
    } else {
        m_polylineIdLabel->setText("多段线ID: 独立线段");
    }
    
    m_createdTimeLabel->setText(QString("创建时间: %1")
                               .arg(info.createdTime.toString("yyyy-MM-dd hh:mm:ss")));
    
    // 更新描述
    m_descriptionEdit->setText(info.description);
    m_descriptionModified = false;
    m_saveDescriptionButton->setEnabled(false);
}

void LineInfoPanel::clearSelectedSegmentInfo()
{
    m_segmentIdLabel->setText("ID: 无");
    m_startPointLabel->setText("起点: 无");
    m_endPointLabel->setText("终点: 无");
    m_lengthLabel->setText("长度: 无");
    m_polylineIdLabel->setText("多段线ID: 无");
    m_createdTimeLabel->setText("创建时间: 无");
    m_descriptionEdit->clear();
    m_descriptionModified = false;
    m_saveDescriptionButton->setEnabled(false);
}

void LineInfoPanel::updateStatistics()
{
    if (!m_lineDrawingTool) {
        m_totalSegmentsLabel->setText("总线段数: 0");
        m_selectedSegmentsLabel->setText("选中线段数: 0");
        m_totalPolylinesLabel->setText("总多段线数: 0");
        m_totalLengthLabel->setText("总长度: 0.00");
        m_averageLengthLabel->setText("平均长度: 0.00");
        m_minLengthLabel->setText("最短长度: 0.00");
        m_maxLengthLabel->setText("最长长度: 0.00");
        return;
    }

    // 获取线段信息
    std::vector<LineSegmentInfo> segmentInfos = m_lineDrawingTool->getSegmentInfoList();
    std::vector<int> selectedIds = m_lineDrawingTool->getSelectedSegmentIdsVector();
    const std::vector<Polyline>& polylines = m_lineDrawingTool->getPolylines();

    // 基本统计
    int totalSegments = segmentInfos.size();
    int selectedSegments = selectedIds.size();
    int totalPolylines = polylines.size();

    m_totalSegmentsLabel->setText(QString("总线段数: %1").arg(totalSegments));
    m_selectedSegmentsLabel->setText(QString("选中线段数: %1").arg(selectedSegments));
    m_totalPolylinesLabel->setText(QString("总多段线数: %1").arg(totalPolylines));

    if (totalSegments == 0) {
        m_totalLengthLabel->setText("总长度: 0.00");
        m_averageLengthLabel->setText("平均长度: 0.00");
        m_minLengthLabel->setText("最短长度: 0.00");
        m_maxLengthLabel->setText("最长长度: 0.00");
        return;
    }

    // 长度统计
    std::vector<float> lengths;
    lengths.reserve(totalSegments);

    for (const auto& info : segmentInfos) {
        lengths.push_back(info.length);
    }

    float totalLength = std::accumulate(lengths.begin(), lengths.end(), 0.0f);
    float averageLength = totalLength / totalSegments;
    float minLength = *std::min_element(lengths.begin(), lengths.end());
    float maxLength = *std::max_element(lengths.begin(), lengths.end());

    m_totalLengthLabel->setText(QString("总长度: %.3f").arg(totalLength));
    m_averageLengthLabel->setText(QString("平均长度: %.3f").arg(averageLength));
    m_minLengthLabel->setText(QString("最短长度: %.3f").arg(minLength));
    m_maxLengthLabel->setText(QString("最长长度: %.3f").arg(maxLength));
}

} // namespace WallExtraction
