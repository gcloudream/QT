#ifndef VIEW_PROJECTION_MANAGER_H
#define VIEW_PROJECTION_MANAGER_H

#include <QObject>
#include <QVector3D>
#include <QVector2D>
#include <QMatrix4x4>
#include <QSize>
#include <QRectF>
#include <vector>
#include "top_down_view_renderer.h"

namespace WallExtraction {

// 视图参数
struct ViewParameters {
    QVector3D center;           // 视图中心
    float zoom;                 // 缩放级别
    float rotation;             // 旋转角度（度）
    QRectF bounds;             // 世界坐标边界
    
    ViewParameters() : center(0, 0, 0), zoom(1.0f), rotation(0.0f) {}
};

// 投影矩阵
struct ProjectionMatrices {
    QMatrix4x4 view;            // 视图矩阵
    QMatrix4x4 projection;      // 投影矩阵
    QMatrix4x4 viewport;        // 视口矩阵
    QMatrix4x4 combined;        // 组合矩阵
    
    void updateCombined() {
        combined = viewport * projection * view;
    }
};

/**
 * @brief 视图投影管理器
 * 
 * 负责管理2D俯视图的投影变换，包括正交投影、视口变换、
 * 坐标系转换等功能。
 */
class ViewProjectionManager : public QObject
{
    Q_OBJECT

public:
    explicit ViewProjectionManager(QObject* parent = nullptr);
    ~ViewProjectionManager();

    /**
     * @brief 检查管理器是否已初始化
     * @return 初始化状态
     */
    bool isInitialized() const;

    /**
     * @brief 设置投影类型
     * @param type 投影类型
     */
    void setProjectionType(ProjectionType type);

    /**
     * @brief 获取投影类型
     * @return 当前投影类型
     */
    ProjectionType getProjectionType() const;

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
     * @brief 设置视图边界
     * @param bounds 边界矩形
     */
    void setViewBounds(const QRectF& bounds);

    /**
     * @brief 获取视图边界
     * @return 边界矩形
     */
    QRectF getViewBounds() const;

    /**
     * @brief 设置视图参数
     * @param params 视图参数
     */
    void setViewParameters(const ViewParameters& params);

    /**
     * @brief 获取视图参数
     * @return 当前视图参数
     */
    ViewParameters getViewParameters() const;

    /**
     * @brief 投影到俯视图
     * @param points 3D点云数据
     * @return 投影结果
     */
    std::vector<ProjectionResult> projectToTopDown(const std::vector<QVector3D>& points) const;

    /**
     * @brief 投影单个点
     * @param worldPoint 世界坐标点
     * @return 投影结果
     */
    ProjectionResult projectPoint(const QVector3D& worldPoint) const;

    /**
     * @brief 世界坐标到屏幕坐标
     * @param worldPoint 世界坐标点
     * @return 屏幕坐标
     */
    QVector2D worldToScreen(const QVector3D& worldPoint) const;

    /**
     * @brief 屏幕坐标到世界坐标
     * @param screenPoint 屏幕坐标点
     * @param worldZ 世界Z坐标（高度）
     * @return 世界坐标
     */
    QVector3D screenToWorld(const QVector2D& screenPoint, float worldZ = 0.0f) const;

    /**
     * @brief 带深度的投影
     * @param points 3D点云数据
     * @return 带深度的投影结果
     */
    std::vector<ProjectionResult> projectWithDepth(const std::vector<QVector3D>& points) const;

    /**
     * @brief 启用/禁用深度测试
     * @param enabled 是否启用
     */
    void enableDepthTesting(bool enabled);

    /**
     * @brief 检查深度测试是否启用
     * @return 是否启用
     */
    bool isDepthTestingEnabled() const;

    /**
     * @brief 设置深度范围
     * @param nearZ 近平面Z值
     * @param farZ 远平面Z值
     */
    void setDepthRange(float nearZ, float farZ);

    /**
     * @brief 获取深度范围
     * @return 深度范围（近平面，远平面）
     */
    std::pair<float, float> getDepthRange() const;

    /**
     * @brief 计算适合的视图边界
     * @param points 点云数据
     * @param margin 边距比例
     * @return 计算的边界
     */
    QRectF calculateFitBounds(const std::vector<QVector3D>& points, float margin = 0.1f) const;

    /**
     * @brief 自动适配视图
     * @param points 点云数据
     * @param margin 边距比例
     */
    void fitToPoints(const std::vector<QVector3D>& points, float margin = 0.1f);

    /**
     * @brief 获取投影矩阵
     * @return 投影矩阵组合
     */
    ProjectionMatrices getProjectionMatrices() const;

    /**
     * @brief 更新投影矩阵
     */
    void updateProjectionMatrices();

    /**
     * @brief 检查点是否在视图范围内
     * @param worldPoint 世界坐标点
     * @return 是否可见
     */
    bool isPointVisible(const QVector3D& worldPoint) const;

    /**
     * @brief 视锥体剔除
     * @param points 点云数据
     * @return 可见点的索引
     */
    std::vector<size_t> performFrustumCulling(const std::vector<QVector3D>& points) const;

signals:
    /**
     * @brief 投影参数改变信号
     */
    void projectionParametersChanged();

    /**
     * @brief 视图边界改变信号
     * @param bounds 新的边界
     */
    void viewBoundsChanged(const QRectF& bounds);

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
     * @brief 创建正交投影矩阵
     * @return 投影矩阵
     */
    QMatrix4x4 createOrthographicMatrix() const;

    /**
     * @brief 创建透视投影矩阵
     * @return 投影矩阵
     */
    QMatrix4x4 createPerspectiveMatrix() const;

    /**
     * @brief 创建视图矩阵
     * @return 视图矩阵
     */
    QMatrix4x4 createViewMatrix() const;

    /**
     * @brief 创建视口矩阵
     * @return 视口矩阵
     */
    QMatrix4x4 createViewportMatrix() const;

    /**
     * @brief 应用视图变换
     * @param point 世界坐标点
     * @return 视图坐标点
     */
    QVector3D applyViewTransform(const QVector3D& point) const;

    /**
     * @brief 应用投影变换
     * @param point 视图坐标点
     * @return 投影坐标点
     */
    QVector3D applyProjectionTransform(const QVector3D& point) const;

    /**
     * @brief 应用视口变换
     * @param point 投影坐标点
     * @return 屏幕坐标点
     */
    QVector2D applyViewportTransform(const QVector3D& point) const;

    /**
     * @brief 标准化设备坐标到屏幕坐标
     * @param ndcPoint 标准化设备坐标
     * @return 屏幕坐标
     */
    QVector2D ndcToScreen(const QVector3D& ndcPoint) const;

    /**
     * @brief 屏幕坐标到标准化设备坐标
     * @param screenPoint 屏幕坐标
     * @param depth 深度值
     * @return 标准化设备坐标
     */
    QVector3D screenToNdc(const QVector2D& screenPoint, float depth = 0.0f) const;

    /**
     * @brief 验证投影参数
     * @return 参数是否有效
     */
    bool validateProjectionParameters() const;

private:
    bool m_initialized;
    ProjectionType m_projectionType;
    QSize m_viewportSize;
    QRectF m_viewBounds;
    ViewParameters m_viewParams;
    bool m_depthTestingEnabled;
    float m_nearZ;
    float m_farZ;
    
    // 投影矩阵
    mutable ProjectionMatrices m_matrices;
    mutable bool m_matricesValid;
    
    // 性能优化
    mutable QVector3D m_lastViewCenter;
    mutable float m_lastZoom;
    mutable bool m_transformCacheValid;
};

} // namespace WallExtraction

// 注册元类型
Q_DECLARE_METATYPE(WallExtraction::ViewParameters)
Q_DECLARE_METATYPE(WallExtraction::ProjectionMatrices)

#endif // VIEW_PROJECTION_MANAGER_H
