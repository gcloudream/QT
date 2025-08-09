#ifndef LINE_DRAWING_TOOL_H
#define LINE_DRAWING_TOOL_H

#include <QObject>
#include <QWidget>
#include <QVector3D>
#include <QVector2D>
#include <QColor>
#include <QDateTime>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QJsonObject>
#include <QJsonDocument>
#include <vector>
#include <memory>
#include <unordered_set>
#include <functional>

namespace WallExtraction {

// 排序条件枚举
enum class SortCriteria {
    ById = 0,           // 按ID排序
    ByLength = 1,       // 按长度排序
    ByCreatedTime = 2,  // 按创建时间排序
    BySelection = 3     // 按选择状态排序
};

// 绘制模式枚举
enum class DrawingMode {
    None = 0,           // 无操作模式
    SingleLine = 1,     // 单线段绘制
    Polyline = 2,       // 多段线绘制
    Selection = 3,      // 选择模式
    Edit = 4           // 编辑模式
};

// 编辑模式枚举
enum class EditMode {
    None = 0,           // 无编辑操作
    MoveEndpoint = 1,   // 移动端点
    SplitSegment = 2,   // 分割线段
    MergeSegments = 3   // 合并线段
};

// 线段数据结构
struct LineSegment {
    int id;                     // 唯一标识符
    QVector3D startPoint;       // 起点坐标
    QVector3D endPoint;         // 终点坐标
    int polylineId;             // 所属多段线ID（-1表示独立线段）
    QString description;        // 用户描述
    QDateTime createdTime;      // 创建时间
    bool isSelected;            // 是否被选中
    QColor color;              // 显示颜色

    // 构造函数
    LineSegment() : id(-1), polylineId(-1), isSelected(false), color(Qt::blue) {
        createdTime = QDateTime::currentDateTime();
    }

    // 计算长度
    float length() const {
        return startPoint.distanceToPoint(endPoint);
    }

    // 转换为JSON
    QJsonObject toJson() const;

    // 从JSON创建
    static LineSegment fromJson(const QJsonObject& json);
};

// 多段线数据结构
struct Polyline {
    int id;                         // 唯一标识符
    std::vector<int> segmentIds;    // 包含的线段ID列表
    QString description;            // 用户描述
    QDateTime createdTime;          // 创建时间
    bool isClosed;                  // 是否闭合
    QColor color;                   // 显示颜色

    // 构造函数
    Polyline() : id(-1), isClosed(false), color(Qt::darkBlue) {
        createdTime = QDateTime::currentDateTime();
    }

    // 计算总长度
    float totalLength(const std::vector<LineSegment>& segments) const;

    // 转换为JSON
    QJsonObject toJson() const;

    // 从JSON创建
    static Polyline fromJson(const QJsonObject& json);
};

// 线段信息结构（用于UI显示）
struct LineSegmentInfo {
    int id;
    QVector3D startPoint;
    QVector3D endPoint;
    float length;
    int polylineId;
    QString description;
    QDateTime createdTime;
    bool isSelected;
};

/**
 * @brief 交互式线段绘制工具类
 *
 * 提供完整的线段绘制功能，包括：
 * - 多种绘制模式（单线段、多段线、选择、编辑）
 * - 鼠标和键盘交互
 * - 线段数据管理和持久化
 * - 实时视觉反馈
 */
class LineDrawingTool : public QObject
{
    Q_OBJECT

public:
    explicit LineDrawingTool(QWidget* parent = nullptr);
    ~LineDrawingTool();

    // 初始化和状态
    bool initialize();
    bool isInitialized() const;
    void reset();

    // 绘制模式管理
    DrawingMode getDrawingMode() const;
    void setDrawingMode(DrawingMode mode);

    EditMode getEditMode() const;
    void setEditMode(EditMode mode);

    // 数据访问
    const std::vector<LineSegment>& getLineSegments() const;
    const std::vector<Polyline>& getPolylines() const;
    std::vector<LineSegmentInfo> getSegmentInfoList() const;
    std::vector<LineSegmentInfo> getSortedSegmentInfoList(SortCriteria criteria, bool ascending = true) const;
    LineSegmentInfo getSegmentInfo(int segmentId) const;

    // 选择管理
    const std::unordered_set<int>& getSelectedSegmentIds() const;
    std::vector<int> getSelectedSegmentIdsVector() const;  // 返回vector版本
    void selectSegment(int segmentId, bool multiSelect = false);
    void deselectSegment(int segmentId);
    void clearSelection();
    void selectAll();
    void deleteSelectedSegments();

    // 线段操作
    int addLineSegment(const QVector3D& startPoint, const QVector3D& endPoint,
                       int polylineId = -1, const QString& description = "");
    bool removeLineSegment(int segmentId);
    bool updateLineSegment(int segmentId, const LineSegment& newSegment);
    bool updateSegmentDescription(int segmentId, const QString& description);
    LineSegment* getLineSegment(int segmentId);

    // 多段线操作
    int createPolyline(const std::vector<int>& segmentIds, const QString& description = "");
    bool removePolyline(int polylineId);
    bool updatePolyline(int polylineId, const Polyline& newPolyline);
    Polyline* getPolyline(int polylineId);

    // 编辑操作
    bool splitSegment(int segmentId, const QVector3D& splitPoint);
    bool mergeSegments(int segmentId1, int segmentId2);
    bool moveEndpoint(int segmentId, bool isStartPoint, const QVector3D& newPosition);

    // 数据持久化
    bool saveToFile(const QString& filename) const;
    bool loadFromFile(const QString& filename);
    QJsonDocument exportToJson() const;
    bool importFromJson(const QJsonDocument& document);

    // 设置外部坐标转换函数（用于与渲染系统集成）
    void setCoordinateConverter(std::function<QVector3D(const QVector2D&)> screenToWorldFunc,
                               std::function<QVector2D(const QVector3D&)> worldToScreenFunc);

    // 事件处理
    bool handleMousePressEvent(QMouseEvent* event);
    bool handleMouseMoveEvent(QMouseEvent* event);
    bool handleMouseReleaseEvent(QMouseEvent* event);
    bool handleKeyPressEvent(QKeyEvent* event);

    // 工具方法
    void clearAll();
    void clearAllSegments();
    void clearAllPolylines();
    int getNextSegmentId() const;
    int getNextPolylineId() const;

    // 统计信息
    int getTotalSegmentCount() const;
    int getSelectedSegmentCount() const;
    int getTotalPolylineCount() const;
    float getTotalLength() const;

signals:
    // 数据变更信号
    void lineSegmentAdded(const LineSegment& segment);
    void lineSegmentRemoved(int segmentId);
    void lineSegmentUpdated(int segmentId, const LineSegment& segment);
    void lineSegmentSelected(int segmentId);
    void lineSegmentDeselected(int segmentId);

    void polylineAdded(const Polyline& polyline);
    void polylineRemoved(int polylineId);
    void polylineUpdated(int polylineId, const Polyline& polyline);

    // 模式变更信号
    void drawingModeChanged(DrawingMode mode);
    void editModeChanged(EditMode mode);

    // 操作状态信号
    void operationStarted(const QString& operation);
    void operationCompleted(const QString& operation);
    void operationCancelled(const QString& operation);

    // 错误和警告信号
    void errorOccurred(const QString& error);
    void warningOccurred(const QString& warning);

    // 渲染更新信号
    void visualFeedbackUpdateRequested();

private slots:
    void onInternalUpdate();

private:
    // 初始化方法
    void initializeDataStructures();
    void setupEventHandling();

    // 事件处理辅助方法
    bool handleSingleLineDrawing(QMouseEvent* event);
    bool handlePolylineDrawing(QMouseEvent* event);
    bool handleSelection(QMouseEvent* event);
    bool handleEditing(QMouseEvent* event);

    // 编辑模式处理
    bool handleMoveEndpoint(QMouseEvent* event);
    bool handleSplitSegment(QMouseEvent* event);
    bool handleMergeSegments(QMouseEvent* event);

    // 几何计算辅助方法
    int findSegmentAtPoint(const QVector2D& screenPoint, float tolerance = 5.0f) const;
    bool isPointNearEndpoint(const QVector2D& screenPoint, int segmentId,
                            bool& isStartPoint, float tolerance = 8.0f) const;
    QVector3D screenToWorld(const QVector2D& screenPoint) const;
    QVector2D worldToScreen(const QVector3D& worldPoint) const;



    float distancePointToLineSegment(const QVector2D& point,
                                    const QVector2D& lineStart,
                                    const QVector2D& lineEnd) const;

    // 数据管理辅助方法
    void validateDataIntegrity();
    void updateSegmentIndices();
    void updatePolylineIndices();

    // 渲染辅助方法
    void updateVisualFeedback();
    void highlightSegment(int segmentId, bool highlight);

private:
    // 基本状态
    bool m_initialized;
    QWidget* m_parentWidget;

    // 绘制状态
    DrawingMode m_currentDrawingMode;
    EditMode m_currentEditMode;

    // 数据存储
    std::vector<LineSegment> m_lineSegments;
    std::vector<Polyline> m_polylines;
    std::unordered_set<int> m_selectedSegmentIds;

    // ID管理
    int m_nextSegmentId;
    int m_nextPolylineId;

    // 交互状态
    bool m_isDrawing;
    bool m_isEditing;
    QVector3D m_currentStartPoint;
    QVector3D m_currentEndPoint;
    int m_currentPolylineId;        // 当前正在绘制的多段线ID
    std::vector<int> m_currentPolylineSegments;  // 当前多段线的线段列表

    // 编辑状态
    int m_editingSegmentId;
    bool m_editingStartPoint;
    QVector3D m_originalPosition;

    // 视觉反馈
    std::unordered_set<int> m_highlightedSegments;

    // 配置参数
    float m_selectionTolerance;     // 选择容差（像素）
    float m_endpointTolerance;      // 端点容差（像素）
    QColor m_defaultLineColor;      // 默认线段颜色
    QColor m_selectedLineColor;     // 选中线段颜色
    QColor m_previewLineColor;      // 预览线段颜色

    // 外部坐标转换函数
    std::function<QVector3D(const QVector2D&)> m_externalScreenToWorld;
    std::function<QVector2D(const QVector3D&)> m_externalWorldToScreen;
};

} // namespace WallExtraction

// 注册元类型以支持信号槽
Q_DECLARE_METATYPE(WallExtraction::DrawingMode)
Q_DECLARE_METATYPE(WallExtraction::EditMode)
Q_DECLARE_METATYPE(WallExtraction::LineSegment)
Q_DECLARE_METATYPE(WallExtraction::Polyline)

#endif // LINE_DRAWING_TOOL_H
