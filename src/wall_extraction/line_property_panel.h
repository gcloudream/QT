#ifndef LINE_PROPERTY_PANEL_H
#define LINE_PROPERTY_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include "line_drawing_tool.h"
#include "line_info_panel.h"
#include "line_list_widget.h"

namespace WallExtraction {

/**
 * @brief 线段属性面板类
 * 
 * 综合的属性面板，包含线段信息显示和列表管理功能
 * 支持可折叠、可拖拽的界面布局
 */
class LinePropertyPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LinePropertyPanel(QWidget* parent = nullptr);
    ~LinePropertyPanel();

    // 设置关联的线段绘制工具
    void setLineDrawingTool(LineDrawingTool* tool);
    
    // 面板显示控制
    void showPanel();
    void hidePanel();
    void togglePanel();
    bool isPanelVisible() const;
    
    // 面板大小控制
    void setMinimumPanelSize(const QSize& size);
    void setMaximumPanelSize(const QSize& size);
    
    // 获取子组件
    LineInfoPanel* getInfoPanel() const { return m_infoPanel; }
    LineListWidget* getListWidget() const { return m_listWidget; }

public slots:
    // 响应线段工具的信号
    void onLineSegmentAdded(const LineSegment& segment);
    void onLineSegmentRemoved(int segmentId);
    void onLineSegmentSelected(int segmentId);
    
    // 面板控制槽函数
    void onToggleButtonClicked();
    void onTabChanged(int index);

signals:
    // 面板状态变化信号
    void panelVisibilityChanged(bool visible);
    void panelSizeChanged(const QSize& size);
    
    // 转发子组件信号
    void segmentSelectionRequested(int segmentId);
    void segmentDeletionRequested(int segmentId);
    void segmentDescriptionEditRequested(int segmentId);
    void segmentDescriptionChanged(int segmentId, const QString& description);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();
    void setupHeaderWidget();
    void setupContentWidget();
    void setupAnimations();
    void connectSignals();
    void updateToggleButtonText();
    void animateToggle(bool show);

    // 现代化样式方法
    QString getModernPanelStyle() const;
    QString getModernTabWidgetStyle() const;
    QPushButton* createStyledButton(const QString& text, const QString& styleClass);
    QLabel* createStyledLabel(const QString& text, const QString& styleClass);

private:
    // 关联的线段绘制工具
    LineDrawingTool* m_lineDrawingTool;
    
    // 主布局
    QVBoxLayout* m_mainLayout;
    
    // 头部控件
    QWidget* m_headerWidget;
    QHBoxLayout* m_headerLayout;
    QLabel* m_titleLabel;
    QPushButton* m_toggleButton;
    QFrame* m_headerSeparator;
    
    // 内容控件
    QWidget* m_contentWidget;
    QTabWidget* m_tabWidget;
    QSplitter* m_splitter;
    
    // 子面板
    LineInfoPanel* m_infoPanel;
    LineListWidget* m_listWidget;
    
    // 动画效果
    QPropertyAnimation* m_toggleAnimation;
    
    // 状态变量
    bool m_isVisible;
    bool m_isAnimating;
    QSize m_collapsedSize;
    QSize m_expandedSize;
    int m_currentTabIndex;
};

} // namespace WallExtraction

#endif // LINE_PROPERTY_PANEL_H
