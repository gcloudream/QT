#ifndef LINE_LIST_WIDGET_H
#define LINE_LIST_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSplitter>
#include <QMenu>
#include <QContextMenuEvent>
#include "line_drawing_tool.h"

namespace WallExtraction {

// 过滤条件结构
struct FilterCriteria {
    float minLength = 0.0f;         // 最小长度
    float maxLength = 1000.0f;      // 最大长度
    bool showSelected = true;       // 显示选中的线段
    bool showUnselected = true;     // 显示未选中的线段
    QString descriptionFilter;      // 描述过滤文本

    FilterCriteria() = default;
};

/**
 * @brief 线段列表控件类
 *
 * 显示所有线段的列表，支持排序、过滤和选择操作
 */
class LineListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LineListWidget(QWidget* parent = nullptr);
    ~LineListWidget();

    // 设置关联的线段绘制工具
    void setLineDrawingTool(LineDrawingTool* tool);
    
    // 更新列表显示
    void updateList();
    void refreshList();

public slots:
    // 响应线段工具的信号
    void onLineSegmentAdded(const LineSegment& segment);
    void onLineSegmentRemoved(int segmentId);
    void onLineSegmentSelected(int segmentId);
    
    // 排序和过滤槽函数
    void onSortCriteriaChanged();
    void onFilterChanged();
    void onResetFilter();
    
    // 表格操作槽函数
    void onTableItemSelectionChanged();
    void onTableItemDoubleClicked(QTableWidgetItem* item);
    void onTableContextMenuRequested(const QPoint& pos);
    
    // 上下文菜单操作
    void onSelectSegment();
    void onDeleteSegment();
    void onEditDescription();

signals:
    // 选择请求信号
    void segmentSelectionRequested(int segmentId);
    void segmentDeletionRequested(int segmentId);
    void segmentDescriptionEditRequested(int segmentId);

private:
    void setupUI();
    void setupFilterGroup();
    void setupTableWidget();
    void setupContextMenu();
    void connectSignals();
    void populateTable();
    void applyFilter();
    void updateTableSelection();
    
    // 表格列枚举
    enum TableColumn {
        COL_ID = 0,
        COL_START_POINT,
        COL_END_POINT,
        COL_LENGTH,
        COL_POLYLINE_ID,
        COL_CREATED_TIME,
        COL_DESCRIPTION,
        COL_COUNT
    };

private:
    // 关联的线段绘制工具
    LineDrawingTool* m_lineDrawingTool;
    
    // 主布局
    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // 过滤器组
    QGroupBox* m_filterGroup;
    QComboBox* m_sortComboBox;
    QCheckBox* m_ascendingCheckBox;
    QLineEdit* m_descriptionFilterEdit;
    QDoubleSpinBox* m_minLengthSpinBox;
    QDoubleSpinBox* m_maxLengthSpinBox;
    QCheckBox* m_showSelectedCheckBox;
    QCheckBox* m_showUnselectedCheckBox;
    QPushButton* m_resetFilterButton;
    
    // 表格控件
    QTableWidget* m_tableWidget;
    
    // 上下文菜单
    QMenu* m_contextMenu;
    QAction* m_selectAction;
    QAction* m_deleteAction;
    QAction* m_editDescriptionAction;
    
    // 当前状态
    std::vector<LineSegmentInfo> m_currentSegmentInfos;
    FilterCriteria m_currentFilter;
    SortCriteria m_currentSortCriteria;
    bool m_sortAscending;
    int m_contextMenuSegmentId;
};

} // namespace WallExtraction

#endif // LINE_LIST_WIDGET_H
