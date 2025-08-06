#ifndef TOP_DOWN_VIEW_RENDERER_H
#define TOP_DOWN_VIEW_RENDERER_H

#include <QObject>
#include <QVector3D>
#include <QVector2D>
#include <QColor>
#include <QImage>
#include <QSize>
#include <QPointF>
#include <QRectF>
#include <vector>
#include <memory>
#include "las_reader.h"

namespace WallExtraction {

// 俯视图渲染模式
enum class TopDownRenderMode {
    Points,         // 点渲染
    Density,        // 密度图
    Contour,        // 等高线
    Heatmap         // 热力图
};

// 投影类型
enum class ProjectionType {
    Orthographic,   // 正交投影
    Perspective     // 透视投影（用于倾斜俯视图）
};

// 带颜色的点
struct ColoredPoint {
    QVector2D screenPosition;   // 屏幕坐标
    QColor color;              // 颜色
    float depth;               // 深度值
    size_t originalIndex;      // 原始点索引
    
    ColoredPoint() : depth(0.0f), originalIndex(0) {}
    ColoredPoint(const QVector2D& pos, const QColor& col, float d = 0.0f, size_t idx = 0)
        : screenPosition(pos), color(col), depth(d), originalIndex(idx) {}
};

// 投影结果
struct ProjectionResult {
    QVector2D screenPosition;
    float depth;
    bool isVisible;
    
    ProjectionResult() : depth(0.0f), isVisible(true) {}
    ProjectionResult(const QVector2D& pos, float d, bool visible = true)
        : screenPosition(pos), depth(d), isVisible(visible) {}
};

// 前向声明
class ColorMappingManager;
class ViewProjectionManager;
class TopDownInteractionController;

/**
 * @brief 俯视图渲染器
 * 
 * 专门用于渲染点云的2D俯视图，支持多种渲染模式和颜色映射，
 * 提供高性能的俯视图显示和交互功能。
 */
class TopDownViewRenderer : public QObject
{
    Q_OBJECT

public:
    explicit TopDownViewRenderer(QObject* parent = nullptr);
    ~TopDownViewRenderer();

    /**
     * @brief 检查渲染器是否已初始化
     * @return 初始化状态
     */
    bool isInitialized() const;

    /**
     * @brief 设置视口大小
     * @param size 视口大小
     */
    void setViewportSize(const QSize& size);

    /**
     * @brief 获取视口大小
     * @return 视口大小
     */
    QSize getViewportSize() const;

    /**
     * @brief 设置视图边界
     * @param minX 最小X坐标
     * @param maxX 最大X坐标
     * @param minY 最小Y坐标
     * @param maxY 最大Y坐标
     */
    void setViewBounds(float minX, float maxX, float minY, float maxY);

    /**
     * @brief 获取视图边界
     * @return 边界矩形
     */
    QRectF getViewBounds() const;

    /**
     * @brief 设置渲染模式
     * @param mode 渲染模式
     */
    void setRenderMode(TopDownRenderMode mode);

    /**
     * @brief 获取渲染模式
     * @return 当前渲染模式
     */
    TopDownRenderMode getRenderMode() const;

    /**
     * @brief 设置点大小
     * @param size 点大小（像素）
     */
    void setPointSize(float size);

    /**
     * @brief 获取点大小
     * @return 点大小
     */
    float getPointSize() const;

    /**
     * @brief 渲染俯视图
     * @param points 带属性的点云数据
     * @return 渲染是否成功
     */
    bool renderTopDownView(const std::vector<PointWithAttributes>& points);

    /**
     * @brief 渲染俯视图（仅坐标）
     * @param points 点云坐标数据
     * @return 渲染是否成功
     */
    bool renderTopDownView(const std::vector<QVector3D>& points);

    /**
     * @brief 获取渲染缓冲区
     * @return 渲染结果图像
     */
    QImage getRenderBuffer() const;

    /**
     * @brief 保存渲染结果
     * @param filename 文件名
     * @param format 图像格式
     * @return 保存是否成功
     */
    bool saveRenderResult(const QString& filename, const QString& format = "PNG");

    /**
     * @brief 获取交互控制器
     * @return 交互控制器指针
     */
    TopDownInteractionController* getInteractionController() const;

    /**
     * @brief 获取颜色映射管理器
     * @return 颜色映射管理器指针
     */
    ColorMappingManager* getColorMappingManager() const;

    /**
     * @brief 获取投影管理器
     * @return 投影管理器指针
     */
    ViewProjectionManager* getProjectionManager() const;

    /**
     * @brief 启用/禁用抗锯齿
     * @param enabled 是否启用
     */
    void setAntiAliasingEnabled(bool enabled);

    /**
     * @brief 检查抗锯齿是否启用
     * @return 是否启用
     */
    bool isAntiAliasingEnabled() const;

    /**
     * @brief 设置背景颜色
     * @param color 背景颜色
     */
    void setBackgroundColor(const QColor& color);

    /**
     * @brief 获取背景颜色
     * @return 背景颜色
     */
    QColor getBackgroundColor() const;

    /**
     * @brief 清除渲染缓冲区
     */
    void clearRenderBuffer();

    /**
     * @brief 获取渲染统计信息
     * @return 统计信息映射
     */
    QVariantMap getRenderStatistics() const;

signals:
    /**
     * @brief 渲染完成信号
     * @param renderTime 渲染时间（毫秒）
     */
    void renderingCompleted(qint64 renderTime);

    /**
     * @brief 渲染进度信号
     * @param percentage 进度百分比 (0-100)
     */
    void renderingProgress(int percentage);

    /**
     * @brief 视图参数改变信号
     */
    void viewParametersChanged();

    /**
     * @brief 状态消息信号
     * @param message 状态消息
     */
    void statusMessage(const QString& message);

    /**
     * @brief 错误发生信号
     * @param error 错误消息
     */
    void errorOccurred(const QString& error);

private:
    /**
     * @brief 渲染点模式
     * @param coloredPoints 带颜色的点数据
     * @return 渲染是否成功
     */
    bool renderPointMode(const std::vector<ColoredPoint>& coloredPoints);

    /**
     * @brief 渲染密度图模式
     * @param coloredPoints 带颜色的点数据
     * @return 渲染是否成功
     */
    bool renderDensityMode(const std::vector<ColoredPoint>& coloredPoints);

    /**
     * @brief 渲染等高线模式
     * @param coloredPoints 带颜色的点数据
     * @return 渲染是否成功
     */
    bool renderContourMode(const std::vector<ColoredPoint>& coloredPoints);

    /**
     * @brief 渲染热力图模式
     * @param coloredPoints 带颜色的点数据
     * @return 渲染是否成功
     */
    bool renderHeatmapMode(const std::vector<ColoredPoint>& coloredPoints);

    /**
     * @brief 预处理点云数据
     * @param points 原始点云数据
     * @return 处理后的带颜色点数据
     */
    std::vector<ColoredPoint> preprocessPoints(const std::vector<PointWithAttributes>& points);

    /**
     * @brief 预处理点云数据（仅坐标）
     * @param points 原始点云坐标
     * @return 处理后的带颜色点数据
     */
    std::vector<ColoredPoint> preprocessPoints(const std::vector<QVector3D>& points);

    /**
     * @brief 应用视锥体剔除
     * @param coloredPoints 带颜色的点数据
     * @return 剔除后的点数据
     */
    std::vector<ColoredPoint> applyCulling(const std::vector<ColoredPoint>& coloredPoints);

    /**
     * @brief 应用宽松的视锥体剔除（用于恢复渲染）
     * @param coloredPoints 带颜色的点数据
     * @return 剔除后的点数据
     */
    std::vector<ColoredPoint> applyLenientCulling(const std::vector<ColoredPoint>& coloredPoints);

    /**
     * @brief 绘制单个点
     * @param point 点数据
     */
    void drawPoint(const ColoredPoint& point);

    /**
     * @brief 绘制密度网格
     * @param gridData 网格密度数据
     * @param gridWidth 网格宽度
     * @param gridHeight 网格高度
     */
    void drawDensityGrid(const std::vector<float>& gridData, int gridWidth, int gridHeight);

    /**
     * @brief 计算密度网格
     * @param points 点数据
     * @param gridWidth 网格宽度
     * @param gridHeight 网格高度
     * @return 密度数据
     */
    std::vector<float> calculateDensityGrid(const std::vector<ColoredPoint>& points, 
                                           int gridWidth, int gridHeight);

    /**
     * @brief 更新渲染统计
     * @param pointCount 点数量
     * @param renderTime 渲染时间
     */
    void updateRenderStatistics(size_t pointCount, qint64 renderTime);

private:
    bool m_initialized;
    QSize m_viewportSize;
    QRectF m_viewBounds;
    TopDownRenderMode m_renderMode;
    float m_pointSize;
    bool m_antiAliasingEnabled;
    QColor m_backgroundColor;
    
    // 渲染缓冲区
    QImage m_renderBuffer;
    
    // 子组件
    std::unique_ptr<ColorMappingManager> m_colorMapper;
    std::unique_ptr<ViewProjectionManager> m_projectionManager;
    std::unique_ptr<TopDownInteractionController> m_interactionController;
    
    // 渲染统计
    mutable QVariantMap m_renderStatistics;
    mutable bool m_statisticsValid;
    
    // 性能计数器
    size_t m_totalRenderCalls;
    qint64 m_totalRenderTime;
    size_t m_lastPointCount;
};

} // namespace WallExtraction

// 注册元类型
Q_DECLARE_METATYPE(WallExtraction::TopDownRenderMode)
Q_DECLARE_METATYPE(WallExtraction::ProjectionType)
Q_DECLARE_METATYPE(WallExtraction::ColoredPoint)

#endif // TOP_DOWN_VIEW_RENDERER_H
