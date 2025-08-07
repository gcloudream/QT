#ifndef LINE_INFO_PANEL_H
#define LINE_INFO_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QSplitter>
#include "line_drawing_tool.h"

namespace WallExtraction {

/**
 * @brief 线段信息显示面板类
 * 
 * 显示选中线段的详细信息，支持编辑线段描述
 */
class LineInfoPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LineInfoPanel(QWidget* parent = nullptr);
    ~LineInfoPanel();

    // 设置关联的线段绘制工具
    void setLineDrawingTool(LineDrawingTool* tool);
    
    // 更新显示内容
    void updateDisplay();
    void updateSelectedSegmentInfo();

public slots:
    // 响应线段工具的信号
    void onLineSegmentSelected(int segmentId);
    void onLineSegmentAdded(const LineSegment& segment);
    void onLineSegmentRemoved(int segmentId);
    
    // 编辑操作槽函数
    void onDescriptionChanged();
    void onSaveDescription();

signals:
    // 编辑请求信号
    void segmentDescriptionChanged(int segmentId, const QString& description);

private:
    void setupUI();
    void setupSelectedSegmentGroup();
    void setupStatisticsGroup();
    void connectSignals();
    void clearSelectedSegmentInfo();
    void updateStatistics();

private:
    // 关联的线段绘制工具
    LineDrawingTool* m_lineDrawingTool;
    
    // 主布局
    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // 选中线段信息组
    QGroupBox* m_selectedSegmentGroup;
    QLabel* m_segmentIdLabel;
    QLabel* m_startPointLabel;
    QLabel* m_endPointLabel;
    QLabel* m_lengthLabel;
    QLabel* m_polylineIdLabel;
    QLabel* m_createdTimeLabel;
    QLineEdit* m_descriptionEdit;
    QPushButton* m_saveDescriptionButton;
    
    // 统计信息组
    QGroupBox* m_statisticsGroup;
    QLabel* m_totalSegmentsLabel;
    QLabel* m_selectedSegmentsLabel;
    QLabel* m_totalPolylinesLabel;
    QLabel* m_totalLengthLabel;
    QLabel* m_averageLengthLabel;
    QLabel* m_minLengthLabel;
    QLabel* m_maxLengthLabel;
    
    // 当前状态
    int m_currentSelectedSegmentId;
    bool m_descriptionModified;
};

} // namespace WallExtraction

#endif // LINE_INFO_PANEL_H
