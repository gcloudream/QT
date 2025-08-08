#ifndef LINE_DRAWING_TOOLBAR_H
#define LINE_DRAWING_TOOLBAR_H

#include <QWidget>
#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QButtonGroup>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include "line_drawing_tool.h"

namespace WallExtraction {

/**
 * @brief 线段绘制工具栏类
 * 
 * 提供线段绘制的UI控件，包括模式切换、工具选择等
 */
class LineDrawingToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit LineDrawingToolbar(QWidget* parent = nullptr);
    ~LineDrawingToolbar();

    // 设置关联的线段绘制工具
    void setLineDrawingTool(LineDrawingTool* tool);
    
    // 启用/禁用工具栏
    void setEnabled(bool enabled);
    
    // 更新状态显示
    void updateStatus();

public slots:
    // 模式切换槽函数
    void onDrawingModeChanged();
    void onEditModeChanged();
    
    // 工具操作槽函数
    void onClearAllLines();
    void onDeleteSelected();
    void onSaveLines();
    void onLoadLines();
    
    // 响应线段工具的信号
    void onLineSegmentAdded(const LineSegment& segment);
    void onLineSegmentRemoved(int segmentId);
    void onLineSegmentSelected(int segmentId);

signals:
    // 模式变更信号
    void drawingModeChangeRequested(DrawingMode mode);
    void editModeChangeRequested(EditMode mode);
    
    // 操作请求信号
    void clearAllRequested();
    void deleteSelectedRequested();
    void saveRequested();
    void loadRequested();

private slots:
    void onDrawingModeButtonClicked(int modeId);
    void onEditModeButtonClicked(int modeId);

private:
    void setupUI();
    void setupDrawingModeGroup();
    void setupEditModeGroup();
    void setupToolsGroup();
    void setupStatusGroup();
    void connectSignals();
    void updateModeButtons();
    void updateStatusLabels();

    // 现代化样式方法
    QString getModernToolbarStyle() const;
    QString getModernGroupBoxStyle() const;
    QPushButton* createStyledButton(const QString& text, const QString& styleClass);
    QLabel* createStyledLabel(const QString& text, const QString& styleClass);

private:
    // 关联的线段绘制工具
    LineDrawingTool* m_lineDrawingTool;
    
    // 主布局
    QVBoxLayout* m_mainLayout;
    
    // 绘制模式组
    QGroupBox* m_drawingModeGroup;
    QButtonGroup* m_drawingModeButtonGroup;
    QPushButton* m_noneButton;
    QPushButton* m_singleLineButton;
    QPushButton* m_polylineButton;
    QPushButton* m_selectionButton;
    QPushButton* m_editButton;
    
    // 编辑模式组
    QGroupBox* m_editModeGroup;
    QButtonGroup* m_editModeButtonGroup;
    QPushButton* m_moveEndpointButton;
    QPushButton* m_splitSegmentButton;
    QPushButton* m_mergeSegmentsButton;
    
    // 工具组
    QGroupBox* m_toolsGroup;
    QPushButton* m_clearAllButton;
    QPushButton* m_deleteSelectedButton;
    QPushButton* m_saveButton;
    QPushButton* m_loadButton;
    
    // 状态显示组
    QGroupBox* m_statusGroup;
    QLabel* m_segmentCountLabel;
    QLabel* m_selectedCountLabel;
    QLabel* m_currentModeLabel;
    
    // 当前状态
    DrawingMode m_currentDrawingMode;
    EditMode m_currentEditMode;
    int m_segmentCount;
    int m_selectedCount;
};

} // namespace WallExtraction

#endif // LINE_DRAWING_TOOLBAR_H
