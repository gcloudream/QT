#include "wall_fitting_result_dialog.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QHeaderView>
#include <QSplitter>
#include <QDebug>

namespace WallExtraction {

WallFittingResultDialog::WallFittingResultDialog(const WallFittingResult& result, QWidget* parent)
    : QDialog(parent)
    , m_result(result)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_tabWidget(nullptr)
    , m_wallsTab(nullptr)
    , m_planesTab(nullptr)
    , m_statisticsTab(nullptr)
    , m_logTab(nullptr)
    , m_wallsTable(nullptr)
    , m_planesTable(nullptr)
    , m_logTextEdit(nullptr)
    , m_exportButton(nullptr)
    , m_closeButton(nullptr)
{
    setupUI();
    setResult(result);
    applyStyles();
    
    // 设置对话框属性
    setWindowTitle("墙面拟合结果");
    setModal(false);
    setMinimumSize(DIALOG_MIN_WIDTH, DIALOG_MIN_HEIGHT);
    resize(DIALOG_MIN_WIDTH + 200, DIALOG_MIN_HEIGHT + 100);
    
    // 居中显示
    if (parent) {
        move(parent->geometry().center() - rect().center());
    }
    
    qDebug() << "WallFittingResultDialog created with" << m_result.walls.size() << "walls";
}

WallFittingResultDialog::~WallFittingResultDialog()
{
    qDebug() << "WallFittingResultDialog destroyed";
}

void WallFittingResultDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 创建标签页控件
    m_tabWidget = new QTabWidget(this);
    
    setupWallsTab();
    setupPlanesTab();
    setupStatisticsTab();
    setupLogTab();
    setupButtonLayout();
    
    m_mainLayout->addWidget(m_tabWidget);
    m_mainLayout->addLayout(m_buttonLayout);
    
    setLayout(m_mainLayout);
    
    // 连接信号
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &WallFittingResultDialog::onTabChanged);
    connect(m_exportButton, &QPushButton::clicked, this, &WallFittingResultDialog::onExportButtonClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void WallFittingResultDialog::setupWallsTab()
{
    m_wallsTab = new QWidget();
    m_wallsLayout = new QVBoxLayout(m_wallsTab);
    
    // 信息标签
    m_wallsInfoLabel = new QLabel("墙面检测结果", m_wallsTab);
    m_wallsInfoLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2c3e50;");
    
    // 墙面表格
    m_wallsTable = new QTableWidget(m_wallsTab);
    m_wallsTable->setColumnCount(8);
    QStringList wallHeaders;
    wallHeaders << "ID" << "起点" << "终点" << "长度(m)" << "高度(m)" << "厚度(m)" << "置信度" << "描述";
    m_wallsTable->setHorizontalHeaderLabels(wallHeaders);
    
    // 设置表格属性
    m_wallsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_wallsTable->setAlternatingRowColors(true);
    m_wallsTable->horizontalHeader()->setStretchLastSection(true);
    m_wallsTable->verticalHeader()->setVisible(false);
    m_wallsTable->verticalHeader()->setDefaultSectionSize(TABLE_ROW_HEIGHT);
    
    m_wallsLayout->addWidget(m_wallsInfoLabel);
    m_wallsLayout->addWidget(m_wallsTable);
    
    m_tabWidget->addTab(m_wallsTab, "墙面 (0)");
    
    connect(m_wallsTable, &QTableWidget::itemSelectionChanged,
            this, &WallFittingResultDialog::onWallTableSelectionChanged);
}

void WallFittingResultDialog::setupPlanesTab()
{
    m_planesTab = new QWidget();
    m_planesLayout = new QVBoxLayout(m_planesTab);
    
    // 信息标签
    m_planesInfoLabel = new QLabel("平面检测结果", m_planesTab);
    m_planesInfoLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2c3e50;");
    
    // 平面表格
    m_planesTable = new QTableWidget(m_planesTab);
    m_planesTable->setColumnCount(6);
    QStringList planeHeaders;
    planeHeaders << "ID" << "法向量" << "距离" << "内点数" << "置信度" << "类型";
    m_planesTable->setHorizontalHeaderLabels(planeHeaders);
    
    // 设置表格属性
    m_planesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_planesTable->setAlternatingRowColors(true);
    m_planesTable->horizontalHeader()->setStretchLastSection(true);
    m_planesTable->verticalHeader()->setVisible(false);
    m_planesTable->verticalHeader()->setDefaultSectionSize(TABLE_ROW_HEIGHT);
    
    m_planesLayout->addWidget(m_planesInfoLabel);
    m_planesLayout->addWidget(m_planesTable);
    
    m_tabWidget->addTab(m_planesTab, "平面 (0)");
    
    connect(m_planesTable, &QTableWidget::itemSelectionChanged,
            this, &WallFittingResultDialog::onPlaneTableSelectionChanged);
}

void WallFittingResultDialog::setupStatisticsTab()
{
    m_statisticsTab = new QWidget();
    m_statisticsLayout = new QVBoxLayout(m_statisticsTab);
    
    // 通用统计组
    m_generalStatsGroup = new QGroupBox("通用统计", m_statisticsTab);
    QVBoxLayout* generalLayout = new QVBoxLayout(m_generalStatsGroup);
    
    m_totalPointsLabel = new QLabel("总点数: --", m_generalStatsGroup);
    m_processedPointsLabel = new QLabel("处理点数: --", m_generalStatsGroup);
    m_unassignedPointsLabel = new QLabel("未分配点数: --", m_generalStatsGroup);
    m_wallCountLabel = new QLabel("墙面数量: --", m_generalStatsGroup);
    m_planeCountLabel = new QLabel("平面数量: --", m_generalStatsGroup);
    
    generalLayout->addWidget(m_totalPointsLabel);
    generalLayout->addWidget(m_processedPointsLabel);
    generalLayout->addWidget(m_unassignedPointsLabel);
    generalLayout->addWidget(m_wallCountLabel);
    generalLayout->addWidget(m_planeCountLabel);
    
    // 性能统计组
    m_performanceStatsGroup = new QGroupBox("性能统计", m_statisticsTab);
    QVBoxLayout* performanceLayout = new QVBoxLayout(m_performanceStatsGroup);
    
    m_processingTimeLabel = new QLabel("处理时间: --", m_performanceStatsGroup);
    
    performanceLayout->addWidget(m_processingTimeLabel);
    
    // 质量统计组
    m_qualityStatsGroup = new QGroupBox("质量统计", m_statisticsTab);
    QVBoxLayout* qualityLayout = new QVBoxLayout(m_qualityStatsGroup);
    
    m_averageWallLengthLabel = new QLabel("平均墙面长度: --", m_qualityStatsGroup);
    m_totalWallAreaLabel = new QLabel("总墙面面积: --", m_qualityStatsGroup);
    m_averageConfidenceLabel = new QLabel("平均置信度: --", m_qualityStatsGroup);
    
    qualityLayout->addWidget(m_averageWallLengthLabel);
    qualityLayout->addWidget(m_totalWallAreaLabel);
    qualityLayout->addWidget(m_averageConfidenceLabel);
    
    m_statisticsLayout->addWidget(m_generalStatsGroup);
    m_statisticsLayout->addWidget(m_performanceStatsGroup);
    m_statisticsLayout->addWidget(m_qualityStatsGroup);
    m_statisticsLayout->addStretch();
    
    m_tabWidget->addTab(m_statisticsTab, "统计信息");
}

void WallFittingResultDialog::setupLogTab()
{
    m_logTab = new QWidget();
    m_logLayout = new QVBoxLayout(m_logTab);
    
    QLabel* logLabel = new QLabel("处理日志", m_logTab);
    logLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2c3e50;");
    
    m_logTextEdit = new QTextEdit(m_logTab);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Consolas", 9));
    
    m_logLayout->addWidget(logLabel);
    m_logLayout->addWidget(m_logTextEdit);
    
    m_tabWidget->addTab(m_logTab, "处理日志");
}

void WallFittingResultDialog::setupButtonLayout()
{
    m_buttonLayout = new QHBoxLayout();
    
    m_exportButton = new QPushButton("导出结果", this);
    m_closeButton = new QPushButton("关闭", this);
    
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_exportButton);
    m_buttonLayout->addWidget(m_closeButton);
}

void WallFittingResultDialog::setResult(const WallFittingResult& result)
{
    m_result = result;
    
    populateWallsTable();
    populatePlanesTable();
    updateStatistics();
    generateProcessingLog();
    
    // 更新标签页标题
    m_tabWidget->setTabText(0, QString("墙面 (%1)").arg(m_result.walls.size()));
    m_tabWidget->setTabText(1, QString("平面 (%1)").arg(m_result.planes.size()));
}

const WallFittingResult& WallFittingResultDialog::getResult() const
{
    return m_result;
}

void WallFittingResultDialog::populateWallsTable()
{
    m_wallsTable->setRowCount(m_result.walls.size());
    
    for (int i = 0; i < m_result.walls.size(); ++i) {
        const WallSegment& wall = m_result.walls[i];
        
        m_wallsTable->setItem(i, 0, new QTableWidgetItem(QString::number(wall.id)));
        m_wallsTable->setItem(i, 1, new QTableWidgetItem(formatVector3D(wall.startPoint)));
        m_wallsTable->setItem(i, 2, new QTableWidgetItem(formatVector3D(wall.endPoint)));
        m_wallsTable->setItem(i, 3, new QTableWidgetItem(formatFloat(wall.length())));
        m_wallsTable->setItem(i, 4, new QTableWidgetItem(formatFloat(wall.height)));
        m_wallsTable->setItem(i, 5, new QTableWidgetItem(formatFloat(wall.thickness)));
        m_wallsTable->setItem(i, 6, new QTableWidgetItem(formatFloat(wall.confidence, 3)));
        m_wallsTable->setItem(i, 7, new QTableWidgetItem(QString("墙面 %1").arg(wall.id)));
    }
    
    // 调整列宽
    m_wallsTable->resizeColumnsToContents();
}

void WallFittingResultDialog::populatePlanesTable()
{
    m_planesTable->setRowCount(m_result.planes.size());

    for (int i = 0; i < m_result.planes.size(); ++i) {
        const Plane3D& plane = m_result.planes[i];
        
        m_planesTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        m_planesTable->setItem(i, 1, new QTableWidgetItem(formatVector3D(plane.normal)));
        m_planesTable->setItem(i, 2, new QTableWidgetItem(formatFloat(plane.distance)));
        m_planesTable->setItem(i, 3, new QTableWidgetItem(QString::number(plane.inlierIndices.size())));
        m_planesTable->setItem(i, 4, new QTableWidgetItem(formatFloat(plane.confidence, 3)));
        m_planesTable->setItem(i, 5, new QTableWidgetItem("垂直平面"));
    }
    
    // 调整列宽
    m_planesTable->resizeColumnsToContents();
}

QString WallFittingResultDialog::formatVector3D(const QVector3D& vector) const
{
    return QString("(%1, %2, %3)")
           .arg(formatFloat(vector.x()))
           .arg(formatFloat(vector.y()))
           .arg(formatFloat(vector.z()));
}

QString WallFittingResultDialog::formatFloat(float value, int precision) const
{
    return QString::number(value, 'f', precision);
}

void WallFittingResultDialog::updateStatistics()
{
    // 通用统计
    m_totalPointsLabel->setText(QString("总点数: %1").arg(m_result.totalPoints));
    m_processedPointsLabel->setText(QString("处理点数: %1").arg(m_result.processedPoints));
    m_unassignedPointsLabel->setText(QString("未分配点数: %1").arg(m_result.unassignedPoints));
    m_wallCountLabel->setText(QString("墙面数量: %1").arg(m_result.walls.size()));
    m_planeCountLabel->setText(QString("平面数量: %1").arg(m_result.planes.size()));
    
    // 性能统计
    m_processingTimeLabel->setText(QString("处理时间: %1").arg(formatDuration(m_result.processingTime)));
    
    // 质量统计
    if (!m_result.walls.empty()) {
        float totalLength = 0.0f;
        float totalArea = 0.0f;
        float totalConfidence = 0.0f;
        
        for (const WallSegment& wall : m_result.walls) {
            totalLength += wall.length();
            totalArea += wall.length() * wall.height;
            totalConfidence += wall.confidence;
        }
        
        float avgLength = totalLength / m_result.walls.size();
        float avgConfidence = totalConfidence / m_result.walls.size();
        
        m_averageWallLengthLabel->setText(QString("平均墙面长度: %1 m").arg(formatFloat(avgLength)));
        m_totalWallAreaLabel->setText(QString("总墙面面积: %1 m²").arg(formatFloat(totalArea)));
        m_averageConfidenceLabel->setText(QString("平均置信度: %1").arg(formatFloat(avgConfidence, 3)));
    } else {
        m_averageWallLengthLabel->setText("平均墙面长度: --");
        m_totalWallAreaLabel->setText("总墙面面积: --");
        m_averageConfidenceLabel->setText("平均置信度: --");
    }
}

QString WallFittingResultDialog::formatDuration(float seconds) const
{
    if (seconds < 1.0f) {
        return QString("%1 ms").arg(formatFloat(seconds * 1000, 0));
    } else if (seconds < 60.0f) {
        return QString("%1 s").arg(formatFloat(seconds, 2));
    } else {
        int minutes = static_cast<int>(seconds / 60);
        float remainingSeconds = seconds - minutes * 60;
        return QString("%1m %2s").arg(minutes).arg(formatFloat(remainingSeconds, 1));
    }
}

void WallFittingResultDialog::generateProcessingLog()
{
    QStringList logEntries;
    
    logEntries << "=== 墙面拟合处理日志 ===";
    logEntries << QString("开始时间: %1").arg(QDateTime::currentDateTime().toString());
    logEntries << "";
    
    logEntries << "输入数据:";
    logEntries << QString("  总点数: %1").arg(m_result.totalPoints);
    logEntries << "";
    
    if (m_result.success) {
        logEntries << "处理结果: 成功";
        logEntries << QString("  检测到 %1 个平面").arg(m_result.planes.size());
        logEntries << QString("  提取到 %1 个墙面").arg(m_result.walls.size());
        logEntries << QString("  处理时间: %1").arg(formatDuration(m_result.processingTime));
    } else {
        logEntries << "处理结果: 失败";
        logEntries << QString("  错误信息: %1").arg(m_result.errorMessage);
    }
    
    logEntries << "";
    logEntries << "详细信息:";
    logEntries << QString("  处理点数: %1").arg(m_result.processedPoints);
    logEntries << QString("  未分配点数: %1").arg(m_result.unassignedPoints);
    
    m_logTextEdit->setPlainText(logEntries.join("\n"));
}

void WallFittingResultDialog::onWallTableSelectionChanged()
{
    QList<QTableWidgetItem*> selectedItems = m_wallsTable->selectedItems();
    if (!selectedItems.isEmpty()) {
        int row = selectedItems.first()->row();
        if (row >= 0 && row < m_result.walls.size()) {
            emit wallSelected(m_result.walls[row].id);
        }
    }
}

void WallFittingResultDialog::onPlaneTableSelectionChanged()
{
    QList<QTableWidgetItem*> selectedItems = m_planesTable->selectedItems();
    if (!selectedItems.isEmpty()) {
        int row = selectedItems.first()->row();
        if (row >= 0 && row < m_result.planes.size()) {
            emit planeSelected(row);
        }
    }
}

void WallFittingResultDialog::onExportButtonClicked()
{
    exportResults();
}

void WallFittingResultDialog::exportResults()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出墙面拟合结果",
        QString("wall_fitting_result_%1.json").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "JSON Files (*.json);;CSV Files (*.csv);;All Files (*)"
    );
    
    if (!fileName.isEmpty()) {
        if (fileName.endsWith(".csv", Qt::CaseInsensitive)) {
            exportToCSV();
        } else {
            exportToJSON();
        }
        emit resultExported(fileName);
    }
}

void WallFittingResultDialog::exportToJSON()
{
    // JSON导出实现将在下一个编辑中添加
    QMessageBox::information(this, "导出", "JSON导出功能正在开发中");
}

void WallFittingResultDialog::exportToCSV()
{
    // CSV导出实现将在下一个编辑中添加
    QMessageBox::information(this, "导出", "CSV导出功能正在开发中");
}

void WallFittingResultDialog::onTabChanged(int index)
{
    Q_UNUSED(index)
    // 标签页切换处理
}

void WallFittingResultDialog::applyStyles()
{
    setStyleSheet(QString(
        "QDialog {"
        "    background-color: #f8f9fa;"
        "}"
        "%1"
        "%2"
        "%3"
    ).arg(getTableStyle())
     .arg(getButtonStyle())
     .arg(getTabStyle()));
}

QString WallFittingResultDialog::getTableStyle() const
{
    return QString(
        "QTableWidget {"
        "    background-color: white;"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 4px;"
        "    gridline-color: #dee2e6;"
        "}"
        "QTableWidget::item {"
        "    padding: 4px 8px;"
        "    border: none;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #007bff;"
        "    color: white;"
        "}"
        "QHeaderView::section {"
        "    background-color: #e9ecef;"
        "    padding: 6px 8px;"
        "    border: 1px solid #dee2e6;"
        "    font-weight: bold;"
        "}"
    );
}

QString WallFittingResultDialog::getButtonStyle() const
{
    return QString(
        "QPushButton {"
        "    background-color: #007bff;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 8px 16px;"
        "    font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #0056b3;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #004085;"
        "}"
    );
}

QString WallFittingResultDialog::getTabStyle() const
{
    return QString(
        "QTabWidget::pane {"
        "    border: 1px solid #dee2e6;"
        "    background-color: white;"
        "}"
        "QTabBar::tab {"
        "    background-color: #e9ecef;"
        "    padding: 8px 16px;"
        "    margin-right: 2px;"
        "    border-top-left-radius: 4px;"
        "    border-top-right-radius: 4px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: white;"
        "    border-bottom: 2px solid #007bff;"
        "}"
    );
}

} // namespace WallExtraction
