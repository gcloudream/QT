#include "view_projection_manager.h"
#include <QDebug>
#include <QtMath>
#include <algorithm>

namespace WallExtraction {

ViewProjectionManager::ViewProjectionManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_projectionType(ProjectionType::Orthographic)
    , m_viewportSize(800, 600)
    , m_viewBounds(-100.0f, 100.0f, -100.0f, 100.0f)
    , m_depthTestingEnabled(true)
    , m_nearZ(0.1f)
    , m_farZ(1000.0f)
    , m_matricesValid(false)
    , m_lastZoom(1.0f)
    , m_transformCacheValid(false)
{
    // 注册元类型
    qRegisterMetaType<ViewParameters>("ViewParameters");
    qRegisterMetaType<ProjectionMatrices>("ProjectionMatrices");
    
    // 初始化视图参数
    m_viewParams.center = QVector3D(0, 0, 0);
    m_viewParams.zoom = 1.0f;
    m_viewParams.rotation = 0.0f;
    m_viewParams.bounds = m_viewBounds;
    
    m_initialized = true;
    qDebug() << "ViewProjectionManager created with orthographic projection";
}

ViewProjectionManager::~ViewProjectionManager()
{
    qDebug() << "ViewProjectionManager destroyed";
}

bool ViewProjectionManager::isInitialized() const
{
    return m_initialized;
}

void ViewProjectionManager::setProjectionType(ProjectionType type)
{
    if (m_projectionType != type) {
        m_projectionType = type;
        m_matricesValid = false;
        m_transformCacheValid = false;
        
        emit projectionParametersChanged();
        emit statusMessage(QString("Projection type changed to %1")
                          .arg(type == ProjectionType::Orthographic ? "Orthographic" : "Perspective"));
    }
}

ProjectionType ViewProjectionManager::getProjectionType() const
{
    return m_projectionType;
}

void ViewProjectionManager::setViewportSize(const QSize& size)
{
    if (m_viewportSize != size) {
        m_viewportSize = size;
        m_matricesValid = false;
        m_transformCacheValid = false;
        
        emit projectionParametersChanged();
        emit statusMessage(QString("Viewport size set to %1x%2").arg(size.width()).arg(size.height()));
    }
}

QSize ViewProjectionManager::getViewportSize() const
{
    return m_viewportSize;
}

void ViewProjectionManager::setViewBounds(float minX, float maxX, float minY, float maxY)
{
    setViewBounds(QRectF(minX, minY, maxX - minX, maxY - minY));
}

void ViewProjectionManager::setViewBounds(const QRectF& bounds)
{
    if (m_viewBounds != bounds) {
        m_viewBounds = bounds;
        m_viewParams.bounds = bounds;
        m_matricesValid = false;
        m_transformCacheValid = false;
        
        emit viewBoundsChanged(bounds);
        emit projectionParametersChanged();
        emit statusMessage(QString("View bounds set to [%1,%2,%3,%4]")
                          .arg(bounds.left()).arg(bounds.top())
                          .arg(bounds.right()).arg(bounds.bottom()));
    }
}

QRectF ViewProjectionManager::getViewBounds() const
{
    return m_viewBounds;
}

void ViewProjectionManager::setViewParameters(const ViewParameters& params)
{
    m_viewParams = params;
    m_viewBounds = params.bounds;
    m_matricesValid = false;
    m_transformCacheValid = false;
    
    emit projectionParametersChanged();
    emit viewBoundsChanged(m_viewBounds);
}

ViewParameters ViewProjectionManager::getViewParameters() const
{
    return m_viewParams;
}

std::vector<ProjectionResult> ViewProjectionManager::projectToTopDown(const std::vector<QVector3D>& points) const
{
    std::vector<ProjectionResult> results;
    results.reserve(points.size());
    
    // 确保投影矩阵是最新的
    if (!m_matricesValid) {
        const_cast<ViewProjectionManager*>(this)->updateProjectionMatrices();
    }
    
    for (const auto& point : points) {
        ProjectionResult result = projectPoint(point);
        results.push_back(result);
    }
    
    return results;
}

ProjectionResult ViewProjectionManager::projectPoint(const QVector3D& worldPoint) const
{
    ProjectionResult result;

    // 确保投影矩阵是最新的
    if (!m_matricesValid) {
        const_cast<ViewProjectionManager*>(this)->updateProjectionMatrices();
    }

    // 分步应用变换以便调试
    QVector4D homogeneousPoint(worldPoint, 1.0f);

    // 1. 视图变换
    QVector4D viewPoint = m_matrices.view * homogeneousPoint;

    // 2. 投影变换
    QVector4D projectedPoint = m_matrices.projection * viewPoint;

    // 3. 透视除法
    if (projectedPoint.w() != 0.0f && qAbs(projectedPoint.w()) > 0.0001f) {
        projectedPoint /= projectedPoint.w();
    }

    // 4. 视口变换
    QVector4D screenPoint = m_matrices.viewport * QVector4D(projectedPoint.x(), projectedPoint.y(), projectedPoint.z(), 1.0f);

    // 转换为屏幕坐标
    result.screenPosition = QVector2D(screenPoint.x(), screenPoint.y());
    result.depth = projectedPoint.z();

    // 检查可见性
    result.isVisible = isPointVisible(worldPoint) &&
                      result.screenPosition.x() >= 0 && result.screenPosition.x() < m_viewportSize.width() &&
                      result.screenPosition.y() >= 0 && result.screenPosition.y() < m_viewportSize.height();

    return result;
}

QVector2D ViewProjectionManager::worldToScreen(const QVector3D& worldPoint) const
{
    ProjectionResult result = projectPoint(worldPoint);
    return result.screenPosition;
}

QVector3D ViewProjectionManager::screenToWorld(const QVector2D& screenPoint, float worldZ) const
{
    // 屏幕坐标到标准化设备坐标
    QVector3D ndcPoint = screenToNdc(screenPoint, 0.0f);
    
    // 创建齐次坐标
    QVector4D homogeneousPoint(ndcPoint, 1.0f);
    
    // 应用逆变换
    QMatrix4x4 inverseMatrix = m_matrices.combined.inverted();
    QVector4D worldHomogeneous = inverseMatrix * homogeneousPoint;
    
    // 透视除法
    if (worldHomogeneous.w() != 0.0f) {
        worldHomogeneous /= worldHomogeneous.w();
    }
    
    // 设置指定的Z坐标
    return QVector3D(worldHomogeneous.x(), worldHomogeneous.y(), worldZ);
}

std::vector<ProjectionResult> ViewProjectionManager::projectWithDepth(const std::vector<QVector3D>& points) const
{
    auto results = projectToTopDown(points);
    
    if (m_depthTestingEnabled) {
        // 按深度排序
        std::sort(results.begin(), results.end(), 
                 [](const ProjectionResult& a, const ProjectionResult& b) {
                     return a.depth < b.depth;
                 });
    }
    
    return results;
}

void ViewProjectionManager::enableDepthTesting(bool enabled)
{
    m_depthTestingEnabled = enabled;
    emit statusMessage(QString("Depth testing %1").arg(enabled ? "enabled" : "disabled"));
}

bool ViewProjectionManager::isDepthTestingEnabled() const
{
    return m_depthTestingEnabled;
}

void ViewProjectionManager::setDepthRange(float nearZ, float farZ)
{
    if (nearZ >= farZ) {
        emit errorOccurred("Invalid depth range: near must be less than far");
        return;
    }
    
    m_nearZ = nearZ;
    m_farZ = farZ;
    m_matricesValid = false;
    
    emit projectionParametersChanged();
}

std::pair<float, float> ViewProjectionManager::getDepthRange() const
{
    return {m_nearZ, m_farZ};
}

QRectF ViewProjectionManager::calculateFitBounds(const std::vector<QVector3D>& points, float margin) const
{
    if (points.empty()) {
        return QRectF(-1, -1, 2, 2);
    }
    
    float minX = points[0].x();
    float maxX = points[0].x();
    float minY = points[0].y();
    float maxY = points[0].y();
    
    for (const auto& point : points) {
        minX = qMin(minX, point.x());
        maxX = qMax(maxX, point.x());
        minY = qMin(minY, point.y());
        maxY = qMax(maxY, point.y());
    }
    
    // 添加边距
    float rangeX = maxX - minX;
    float rangeY = maxY - minY;
    float marginX = rangeX * margin;
    float marginY = rangeY * margin;
    
    return QRectF(minX - marginX, minY - marginY, 
                  rangeX + 2 * marginX, rangeY + 2 * marginY);
}

void ViewProjectionManager::fitToPoints(const std::vector<QVector3D>& points, float margin)
{
    QRectF fitBounds = calculateFitBounds(points, margin);
    setViewBounds(fitBounds);
    
    // 重置视图参数
    m_viewParams.center = QVector3D(fitBounds.center().x(), fitBounds.center().y(), 0);
    m_viewParams.zoom = 1.0f;
    m_viewParams.rotation = 0.0f;
    
    emit statusMessage("View fitted to point cloud");
}

ProjectionMatrices ViewProjectionManager::getProjectionMatrices() const
{
    if (!m_matricesValid) {
        const_cast<ViewProjectionManager*>(this)->updateProjectionMatrices();
    }
    return m_matrices;
}

void ViewProjectionManager::updateProjectionMatrices()
{
    m_matrices.view = createViewMatrix();
    m_matrices.projection = (m_projectionType == ProjectionType::Orthographic) ? 
                           createOrthographicMatrix() : createPerspectiveMatrix();
    m_matrices.viewport = createViewportMatrix();
    m_matrices.updateCombined();
    
    m_matricesValid = true;
    m_transformCacheValid = false;
}

bool ViewProjectionManager::isPointVisible(const QVector3D& worldPoint) const
{
    // 简单的边界检查
    return worldPoint.x() >= m_viewBounds.left() && 
           worldPoint.x() <= m_viewBounds.right() &&
           worldPoint.y() >= m_viewBounds.top() && 
           worldPoint.y() <= m_viewBounds.bottom();
}

std::vector<size_t> ViewProjectionManager::performFrustumCulling(const std::vector<QVector3D>& points) const
{
    std::vector<size_t> visibleIndices;
    visibleIndices.reserve(points.size());
    
    for (size_t i = 0; i < points.size(); ++i) {
        if (isPointVisible(points[i])) {
            visibleIndices.push_back(i);
        }
    }
    
    return visibleIndices;
}

// 私有方法实现
QMatrix4x4 ViewProjectionManager::createOrthographicMatrix() const
{
    QMatrix4x4 matrix;

    float left = m_viewBounds.left();
    float right = m_viewBounds.right();
    float bottom = m_viewBounds.bottom();
    float top = m_viewBounds.top();

    // 应用缩放
    float scale = m_viewParams.zoom;
    float centerX = (left + right) * 0.5f;
    float centerY = (bottom + top) * 0.5f;
    float width = (right - left) / scale;
    float height = (top - bottom) / scale;

    left = centerX - width * 0.5f;
    right = centerX + width * 0.5f;
    bottom = centerY - height * 0.5f;
    top = centerY + height * 0.5f;

    matrix.ortho(left, right, bottom, top, m_nearZ, m_farZ);
    return matrix;
}

QMatrix4x4 ViewProjectionManager::createPerspectiveMatrix() const
{
    QMatrix4x4 matrix;

    float aspect = static_cast<float>(m_viewportSize.width()) / m_viewportSize.height();
    float fov = 45.0f; // 45度视野角

    matrix.perspective(fov, aspect, m_nearZ, m_farZ);
    return matrix;
}

QMatrix4x4 ViewProjectionManager::createViewMatrix() const
{
    QMatrix4x4 matrix;

    // 俯视图的视图矩阵
    QVector3D eye = m_viewParams.center + QVector3D(0, 0, 100); // 相机在上方
    QVector3D center = m_viewParams.center;
    QVector3D up = QVector3D(0, 1, 0);

    // 应用旋转
    if (m_viewParams.rotation != 0.0f) {
        matrix.rotate(m_viewParams.rotation, 0, 0, 1);
    }

    matrix.lookAt(eye, center, up);
    return matrix;
}

QMatrix4x4 ViewProjectionManager::createViewportMatrix() const
{
    QMatrix4x4 matrix;

    float width = m_viewportSize.width();
    float height = m_viewportSize.height();

    // 改进的视口变换矩阵
    // 将NDC坐标 [-1,1] 映射到屏幕坐标 [0, width] 和 [0, height]
    matrix.translate(width * 0.5f, height * 0.5f, 0.0f);
    matrix.scale(width * 0.5f, -height * 0.5f, 1.0f);

    return matrix;
}

QVector3D ViewProjectionManager::applyViewTransform(const QVector3D& point) const
{
    QVector4D homogeneous(point, 1.0f);
    QVector4D transformed = m_matrices.view * homogeneous;
    return transformed.toVector3D();
}

QVector3D ViewProjectionManager::applyProjectionTransform(const QVector3D& point) const
{
    QVector4D homogeneous(point, 1.0f);
    QVector4D transformed = m_matrices.projection * homogeneous;

    if (transformed.w() != 0.0f) {
        transformed /= transformed.w();
    }

    return transformed.toVector3D();
}

QVector2D ViewProjectionManager::applyViewportTransform(const QVector3D& point) const
{
    QVector4D homogeneous(point, 1.0f);
    QVector4D transformed = m_matrices.viewport * homogeneous;
    return QVector2D(transformed.x(), transformed.y());
}

QVector2D ViewProjectionManager::ndcToScreen(const QVector3D& ndcPoint) const
{
    float x = (ndcPoint.x() + 1.0f) * 0.5f * m_viewportSize.width();
    float y = (1.0f - ndcPoint.y()) * 0.5f * m_viewportSize.height();
    return QVector2D(x, y);
}

QVector3D ViewProjectionManager::screenToNdc(const QVector2D& screenPoint, float depth) const
{
    float x = (screenPoint.x() / m_viewportSize.width()) * 2.0f - 1.0f;
    float y = 1.0f - (screenPoint.y() / m_viewportSize.height()) * 2.0f;
    return QVector3D(x, y, depth);
}

bool ViewProjectionManager::validateProjectionParameters() const
{
    if (m_viewportSize.width() <= 0 || m_viewportSize.height() <= 0) {
        return false;
    }

    if (m_nearZ >= m_farZ) {
        return false;
    }

    if (m_viewBounds.width() <= 0 || m_viewBounds.height() <= 0) {
        return false;
    }

    return true;
}

} // namespace WallExtraction
