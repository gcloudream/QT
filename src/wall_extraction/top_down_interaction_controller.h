#ifndef TOP_DOWN_INTERACTION_CONTROLLER_H
#define TOP_DOWN_INTERACTION_CONTROLLER_H

#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QVector3D>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <vector>
#include <memory>

namespace WallExtraction {

// 交互模式
enum class InteractionMode {
    Pan,            // 平移
    Zoom,           // 缩放
    Select,         // 选择
    Measure,        // 测量
    None            // 无交互
};

// 选择模式
enum class SelectionMode {
    Point,          // 点选择
    Rectangle,      // 矩形选择
    Polygon,        // 多边形选择
    Circle          // 圆形选择
};

// 测量类型
enum class MeasurementType {
    Distance,       // 距离测量
    Area,           // 面积测量
    Angle           // 角度测量
};

// 交互状态
struct InteractionState {
    InteractionMode mode;
    bool isActive;
    QPointF startPoint;
    QPointF currentPoint;
    QPointF lastPoint;
    
    InteractionState() : mode(InteractionMode::None), isActive(false) {}
};

// 选择结果
struct SelectionResult {
    std::vector<size_t> pointIndices;  // 选中点的索引
    QRectF boundingRect;               // 选择区域边界
    size_t selectionCount;             // 选中点数量
    
    SelectionResult() : selectionCount(0) {}
};

// 测量结果
struct MeasurementResult {
    MeasurementType type;
    float value;                       // 测量值
    QString unit;                      // 单位
    std::vector<QPointF> points;       // 测量点
    bool isValid;
    
    MeasurementResult() : type(MeasurementType::Distance), value(0.0f), isValid(false) {}
};

// 前向声明
class TopDownViewRenderer;
class ViewProjectionManager;

/**
 * @brief 俯视图交互控制器
 * 
 * 负责处理俯视图的用户交互，包括平移、缩放、选择、测量等功能。
 * 提供专门针对2D俯视图优化的交互体验。
 */
class TopDownInteractionController : public QObject
{
    Q_OBJECT

public:
    explicit TopDownInteractionController(QObject* parent = nullptr);
    ~TopDownInteractionController();

    /**
     * @brief 设置渲染器引用
     * @param renderer 渲染器指针
     */
    void setRenderer(TopDownViewRenderer* renderer);

    /**
     * @brief 设置投影管理器引用
     * @param projectionManager 投影管理器指针
     */
    void setProjectionManager(ViewProjectionManager* projectionManager);

    /**
     * @brief 设置交互模式
     * @param mode 交互模式
     */
    void setInteractionMode(InteractionMode mode);

    /**
     * @brief 获取交互模式
     * @return 当前交互模式
     */
    InteractionMode getInteractionMode() const;

    /**
     * @brief 处理鼠标按下事件
     * @param event 鼠标事件
     * @return 是否处理了事件
     */
    bool handleMousePress(QMouseEvent* event);

    /**
     * @brief 处理鼠标移动事件
     * @param event 鼠标事件
     * @return 是否处理了事件
     */
    bool handleMouseMove(QMouseEvent* event);

    /**
     * @brief 处理鼠标释放事件
     * @param event 鼠标事件
     * @return 是否处理了事件
     */
    bool handleMouseRelease(QMouseEvent* event);

    /**
     * @brief 处理滚轮事件
     * @param event 滚轮事件
     * @return 是否处理了事件
     */
    bool handleWheelEvent(QWheelEvent* event);

    /**
     * @brief 处理键盘事件
     * @param event 键盘事件
     * @return 是否处理了事件
     */
    bool handleKeyEvent(QKeyEvent* event);

    // 平移控制
    /**
     * @brief 开始平移
     * @param startPoint 起始点
     */
    void startPan(const QPointF& startPoint);

    /**
     * @brief 更新平移
     * @param currentPoint 当前点
     */
    void updatePan(const QPointF& currentPoint);

    /**
     * @brief 结束平移
     */
    void endPan();

    /**
     * @brief 平移视图
     * @param delta 平移量
     */
    void pan(const QPointF& delta);

    /**
     * @brief 获取视图中心
     * @return 视图中心点
     */
    QPointF getViewCenter() const;

    /**
     * @brief 设置视图中心
     * @param center 新的中心点
     */
    void setViewCenter(const QPointF& center);

    // 缩放控制
    /**
     * @brief 缩放
     * @param factor 缩放因子
     * @param center 缩放中心（屏幕坐标）
     */
    void zoom(float factor, const QPointF& center = QPointF());

    /**
     * @brief 滚轮缩放
     * @param center 缩放中心
     * @param delta 滚轮增量
     */
    void wheelZoom(const QPointF& center, int delta);

    /**
     * @brief 获取缩放级别
     * @return 缩放级别
     */
    float getZoomLevel() const;

    /**
     * @brief 设置缩放级别
     * @param zoom 缩放级别
     */
    void setZoomLevel(float zoom);

    /**
     * @brief 缩放到适合
     * @param margin 边距比例
     */
    void zoomToFit(float margin = 0.1f);

    // 选择控制
    /**
     * @brief 设置选择模式
     * @param mode 选择模式
     */
    void setSelectionMode(SelectionMode mode);

    /**
     * @brief 获取选择模式
     * @return 当前选择模式
     */
    SelectionMode getSelectionMode() const;

    /**
     * @brief 矩形选择
     * @param rect 选择矩形（屏幕坐标）
     * @return 选择结果
     */
    SelectionResult selectPointsInRect(const QRectF& rect);

    /**
     * @brief 圆形选择
     * @param center 圆心（屏幕坐标）
     * @param radius 半径（屏幕坐标）
     * @return 选择结果
     */
    SelectionResult selectPointsInCircle(const QPointF& center, float radius);

    /**
     * @brief 多边形选择
     * @param polygon 多边形顶点（屏幕坐标）
     * @return 选择结果
     */
    SelectionResult selectPointsInPolygon(const std::vector<QPointF>& polygon);

    /**
     * @brief 清除选择
     */
    void clearSelection();

    /**
     * @brief 获取当前选择
     * @return 选择结果
     */
    SelectionResult getCurrentSelection() const;

    // 测量控制
    /**
     * @brief 设置测量类型
     * @param type 测量类型
     */
    void setMeasurementType(MeasurementType type);

    /**
     * @brief 获取测量类型
     * @return 当前测量类型
     */
    MeasurementType getMeasurementType() const;

    /**
     * @brief 测量屏幕距离
     * @param point1 点1（屏幕坐标）
     * @param point2 点2（屏幕坐标）
     * @return 屏幕距离（像素）
     */
    float measureScreenDistance(const QPointF& point1, const QPointF& point2);

    /**
     * @brief 测量世界距离
     * @param point1 点1（屏幕坐标）
     * @param point2 点2（屏幕坐标）
     * @return 世界距离（米）
     */
    float measureWorldDistance(const QPointF& point1, const QPointF& point2);

    /**
     * @brief 测量面积
     * @param points 多边形顶点（屏幕坐标）
     * @return 面积（平方米）
     */
    float measureArea(const std::vector<QPointF>& points);

    /**
     * @brief 测量角度
     * @param center 角的顶点（屏幕坐标）
     * @param point1 角的一边（屏幕坐标）
     * @param point2 角的另一边（屏幕坐标）
     * @return 角度（度）
     */
    float measureAngle(const QPointF& center, const QPointF& point1, const QPointF& point2);

    /**
     * @brief 获取测量结果
     * @return 当前测量结果
     */
    std::vector<MeasurementResult> getMeasurementResults() const;

    /**
     * @brief 清除测量结果
     */
    void clearMeasurements();

    // 设置和配置
    /**
     * @brief 设置平移灵敏度
     * @param sensitivity 灵敏度
     */
    void setPanSensitivity(float sensitivity);

    /**
     * @brief 设置缩放灵敏度
     * @param sensitivity 灵敏度
     */
    void setZoomSensitivity(float sensitivity);

    /**
     * @brief 设置最小缩放级别
     * @param minZoom 最小缩放
     */
    void setMinZoom(float minZoom);

    /**
     * @brief 设置最大缩放级别
     * @param maxZoom 最大缩放
     */
    void setMaxZoom(float maxZoom);

    /**
     * @brief 启用/禁用惯性滚动
     * @param enabled 是否启用
     */
    void setInertialScrollingEnabled(bool enabled);

signals:
    /**
     * @brief 视图改变信号
     */
    void viewChanged();

    /**
     * @brief 选择改变信号
     * @param selection 新的选择结果
     */
    void selectionChanged(const SelectionResult& selection);

    /**
     * @brief 测量完成信号
     * @param result 测量结果
     */
    void measurementCompleted(const MeasurementResult& result);

    /**
     * @brief 交互模式改变信号
     * @param mode 新的交互模式
     */
    void interactionModeChanged(InteractionMode mode);

    /**
     * @brief 状态消息信号
     * @param message 状态消息
     */
    void statusMessage(const QString& message);

private:
    /**
     * @brief 更新交互状态
     */
    void updateInteractionState();

    /**
     * @brief 检查点是否在矩形内
     * @param point 点坐标
     * @param rect 矩形
     * @return 是否在内部
     */
    bool isPointInRect(const QPointF& point, const QRectF& rect) const;

    /**
     * @brief 检查点是否在圆形内
     * @param point 点坐标
     * @param center 圆心
     * @param radius 半径
     * @return 是否在内部
     */
    bool isPointInCircle(const QPointF& point, const QPointF& center, float radius) const;

    /**
     * @brief 检查点是否在多边形内
     * @param point 点坐标
     * @param polygon 多边形顶点
     * @return 是否在内部
     */
    bool isPointInPolygon(const QPointF& point, const std::vector<QPointF>& polygon) const;

    /**
     * @brief 限制缩放级别
     * @param zoom 缩放级别
     * @return 限制后的缩放级别
     */
    float clampZoom(float zoom) const;

private:
    TopDownViewRenderer* m_renderer;
    ViewProjectionManager* m_projectionManager;
    
    InteractionState m_state;
    InteractionMode m_interactionMode;
    SelectionMode m_selectionMode;
    MeasurementType m_measurementType;
    
    // 选择和测量结果
    SelectionResult m_currentSelection;
    std::vector<MeasurementResult> m_measurementResults;
    
    // 配置参数
    float m_panSensitivity;
    float m_zoomSensitivity;
    float m_minZoom;
    float m_maxZoom;
    bool m_inertialScrollingEnabled;
    
    // 临时状态
    std::vector<QPointF> m_tempPolygon;
    std::vector<QPointF> m_tempMeasurementPoints;
};

} // namespace WallExtraction

// 注册元类型
Q_DECLARE_METATYPE(WallExtraction::InteractionMode)
Q_DECLARE_METATYPE(WallExtraction::SelectionMode)
Q_DECLARE_METATYPE(WallExtraction::MeasurementType)
Q_DECLARE_METATYPE(WallExtraction::SelectionResult)
Q_DECLARE_METATYPE(WallExtraction::MeasurementResult)

#endif // TOP_DOWN_INTERACTION_CONTROLLER_H
