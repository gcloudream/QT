#ifndef WALL_FITTING_RESULT_DIALOG_H
#define WALL_FITTING_RESULT_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QHeaderView>
#include "wall_fitting_algorithm.h"

namespace WallExtraction {

/**
 * @brief 墙面拟合结果显示对话框
 * 
 * 显示墙面拟合算法的执行结果，包括：
 * - 墙面列表和详细信息
 * - 平面检测结果
 * - 统计信息
 * - 处理日志
 * - 结果导出功能
 */
class WallFittingResultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WallFittingResultDialog(const WallFittingResult& result, QWidget* parent = nullptr);
    ~WallFittingResultDialog();

    // 结果设置
    void setResult(const WallFittingResult& result);
    const WallFittingResult& getResult() const;
    
    // 显示控制
    void showWallsTab();
    void showPlanesTab();
    void showStatisticsTab();
    void showLogTab();

public slots:
    void exportResults();
    void exportToCSV();
    void exportToJSON();

signals:
    void resultExported(const QString& filename);
    void wallSelected(int wallId);
    void planeSelected(int planeId);

private slots:
    void onWallTableSelectionChanged();
    void onPlaneTableSelectionChanged();
    void onExportButtonClicked();
    void onTabChanged(int index);

private:
    void setupUI();
    void setupWallsTab();
    void setupPlanesTab();
    void setupStatisticsTab();
    void setupLogTab();
    void setupButtonLayout();
    
    void populateWallsTable();
    void populatePlanesTable();
    void updateStatistics();
    void generateProcessingLog();
    
    void applyStyles();
    QString getTableStyle() const;
    QString getButtonStyle() const;
    QString getTabStyle() const;
    
    // 数据格式化
    QString formatVector3D(const QVector3D& vector) const;
    QString formatFloat(float value, int precision = 2) const;
    QString formatDateTime(const QDateTime& dateTime) const;
    QString formatDuration(float seconds) const;

private:
    // 数据
    WallFittingResult m_result;
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;
    
    QTabWidget* m_tabWidget;
    
    // 墙面标签页
    QWidget* m_wallsTab;
    QVBoxLayout* m_wallsLayout;
    QTableWidget* m_wallsTable;
    QLabel* m_wallsInfoLabel;
    
    // 平面标签页
    QWidget* m_planesTab;
    QVBoxLayout* m_planesLayout;
    QTableWidget* m_planesTable;
    QLabel* m_planesInfoLabel;
    
    // 统计标签页
    QWidget* m_statisticsTab;
    QVBoxLayout* m_statisticsLayout;
    QGroupBox* m_generalStatsGroup;
    QGroupBox* m_performanceStatsGroup;
    QGroupBox* m_qualityStatsGroup;
    
    // 日志标签页
    QWidget* m_logTab;
    QVBoxLayout* m_logLayout;
    QTextEdit* m_logTextEdit;
    
    // 按钮
    QPushButton* m_exportButton;
    QPushButton* m_closeButton;
    
    // 统计标签
    QLabel* m_totalPointsLabel;
    QLabel* m_processedPointsLabel;
    QLabel* m_unassignedPointsLabel;
    QLabel* m_wallCountLabel;
    QLabel* m_planeCountLabel;
    QLabel* m_processingTimeLabel;
    QLabel* m_averageWallLengthLabel;
    QLabel* m_totalWallAreaLabel;
    QLabel* m_averageConfidenceLabel;
    
    // 常量
    static const int DIALOG_MIN_WIDTH = 800;
    static const int DIALOG_MIN_HEIGHT = 600;
    static const int TABLE_ROW_HEIGHT = 25;
};

} // namespace WallExtraction

#endif // WALL_FITTING_RESULT_DIALOG_H
