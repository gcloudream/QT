#include "top_down_interaction_controller.h"
#include "top_down_view_renderer.h"
#include "view_projection_manager.h"
#include <QDebug>
#include <QtMath>
#include <algorithm>

namespace WallExtraction {

TopDownInteractionController::TopDownInteractionController(QObject* parent)
    : QObject(parent)
    , m_renderer(nullptr)
    , m_projectionManager(nullptr)
    , m_interactionMode(InteractionMode::Pan)
    , m_selectionMode(SelectionMode::Rectangle)
    , m_measurementType(MeasurementType::Distance)
    , m_panSensitivity(1.0f)
    , m_zoomSensitivity(1.2f)
    , m_minZoom(0.1f)
    , m_maxZoom(100.0f)
    , m_inertialScrollingEnabled(true)
{
    // 注册元类型
    qRegisterMetaType<InteractionMode>("InteractionMode");
    qRegisterMetaType<SelectionMode>("SelectionMode");
    qRegisterMetaType<MeasurementType>("MeasurementType");
    qRegisterMetaType<SelectionResult>("SelectionResult");
    qRegisterMetaType<MeasurementResult>("MeasurementResult");
    
    qDebug() << "TopDownInteractionController created";
}

TopDownInteractionController::~TopDownInteractionController()
{
    qDebug() << "TopDownInteractionController destroyed";
}

void TopDownInteractionController::setRenderer(TopDownViewRenderer* renderer)
{
    m_renderer = renderer;
}

void TopDownInteractionController::setProjectionManager(ViewProjectionManager* projectionManager)
{
    m_projectionManager = projectionManager;
}

void TopDownInteractionController::setInteractionMode(InteractionMode mode)
{
    if (m_interactionMode != mode) {
        m_interactionMode = mode;
        
        // 重置交互状态
        m_state.isActive = false;
        m_state.mode = InteractionMode::None;
        
        emit interactionModeChanged(mode);
        emit statusMessage(QString("Interaction mode changed to %1").arg(static_cast<int>(mode)));
    }
}

InteractionMode TopDownInteractionController::getInteractionMode() const
{
    return m_interactionMode;
}

bool TopDownInteractionController::handleMousePress(QMouseEvent* event)
{
    if (!event) return false;
    
    QPointF pos = event->position();
    
    switch (m_interactionMode) {
        case InteractionMode::Pan:
            if (event->button() == Qt::LeftButton) {
                startPan(pos);
                return true;
            }
            break;
            
        case InteractionMode::Select:
            if (event->button() == Qt::LeftButton) {
                m_state.isActive = true;
                m_state.mode = InteractionMode::Select;
                m_state.startPoint = pos;
                m_state.currentPoint = pos;
                return true;
            }
            break;
            
        case InteractionMode::Measure:
            if (event->button() == Qt::LeftButton) {
                m_tempMeasurementPoints.push_back(pos);
                
                if (m_measurementType == MeasurementType::Distance && m_tempMeasurementPoints.size() == 2) {
                    // 完成距离测量
                    float distance = measureWorldDistance(m_tempMeasurementPoints[0], m_tempMeasurementPoints[1]);
                    
                    MeasurementResult result;
                    result.type = MeasurementType::Distance;
                    result.value = distance;
                    result.unit = "m";
                    result.points = m_tempMeasurementPoints;
                    result.isValid = true;
                    
                    m_measurementResults.push_back(result);
                    m_tempMeasurementPoints.clear();
                    
                    emit measurementCompleted(result);
                }
                return true;
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

bool TopDownInteractionController::handleMouseMove(QMouseEvent* event)
{
    if (!event) return false;
    
    QPointF pos = event->position();
    
    if (m_state.isActive) {
        switch (m_state.mode) {
            case InteractionMode::Pan:
                updatePan(pos);
                return true;
                
            case InteractionMode::Select:
                m_state.currentPoint = pos;
                // 这里可以添加选择框的实时显示
                return true;
                
            default:
                break;
        }
    }
    
    return false;
}

bool TopDownInteractionController::handleMouseRelease(QMouseEvent* event)
{
    if (!event) return false;
    
    QPointF pos = event->position();
    
    if (m_state.isActive) {
        switch (m_state.mode) {
            case InteractionMode::Pan:
                if (event->button() == Qt::LeftButton) {
                    endPan();
                    return true;
                }
                break;
                
            case InteractionMode::Select:
                if (event->button() == Qt::LeftButton) {
                    // 完成选择
                    QRectF selectionRect(m_state.startPoint, pos);
                    selectionRect = selectionRect.normalized();
                    
                    SelectionResult result = selectPointsInRect(selectionRect);
                    m_currentSelection = result;
                    
                    m_state.isActive = false;
                    emit selectionChanged(result);
                    return true;
                }
                break;
                
            default:
                break;
        }
    }
    
    return false;
}

bool TopDownInteractionController::handleWheelEvent(QWheelEvent* event)
{
    if (!event) return false;
    
    if (m_interactionMode == InteractionMode::Zoom || 
        (m_interactionMode == InteractionMode::Pan && event->modifiers() & Qt::ControlModifier)) {
        
        QPointF center = event->position();
        int delta = event->angleDelta().y();
        
        wheelZoom(center, delta);
        return true;
    }
    
    return false;
}

bool TopDownInteractionController::handleKeyEvent(QKeyEvent* event)
{
    if (!event) return false;
    
    switch (event->key()) {
        case Qt::Key_Escape:
            // 取消当前操作
            m_state.isActive = false;
            m_tempMeasurementPoints.clear();
            clearSelection();
            return true;
            
        case Qt::Key_Delete:
            // 清除选择或测量
            clearSelection();
            clearMeasurements();
            return true;
            
        default:
            break;
    }
    
    return false;
}

// 平移控制实现
void TopDownInteractionController::startPan(const QPointF& startPoint)
{
    m_state.isActive = true;
    m_state.mode = InteractionMode::Pan;
    m_state.startPoint = startPoint;
    m_state.lastPoint = startPoint;
}

void TopDownInteractionController::updatePan(const QPointF& currentPoint)
{
    if (!m_state.isActive || m_state.mode != InteractionMode::Pan) {
        return;
    }
    
    QPointF delta = (currentPoint - m_state.lastPoint) * m_panSensitivity;
    pan(delta);
    
    m_state.lastPoint = currentPoint;
}

void TopDownInteractionController::endPan()
{
    m_state.isActive = false;
    m_state.mode = InteractionMode::None;
}

void TopDownInteractionController::pan(const QPointF& delta)
{
    if (!m_projectionManager) return;
    
    // 获取当前视图参数
    ViewParameters params = m_projectionManager->getViewParameters();
    
    // 将屏幕坐标的平移转换为世界坐标的平移
    QRectF bounds = m_projectionManager->getViewBounds();
    QSize viewport = m_projectionManager->getViewportSize();
    
    float worldDeltaX = -delta.x() * bounds.width() / viewport.width();
    float worldDeltaY = delta.y() * bounds.height() / viewport.height();
    
    // 更新视图中心
    params.center += QVector3D(worldDeltaX, worldDeltaY, 0);
    
    // 更新边界
    params.bounds.translate(worldDeltaX, worldDeltaY);
    
    m_projectionManager->setViewParameters(params);
    emit viewChanged();
}

QPointF TopDownInteractionController::getViewCenter() const
{
    if (!m_projectionManager) return QPointF();
    
    ViewParameters params = m_projectionManager->getViewParameters();
    return QPointF(params.center.x(), params.center.y());
}

void TopDownInteractionController::setViewCenter(const QPointF& center)
{
    if (!m_projectionManager) return;
    
    ViewParameters params = m_projectionManager->getViewParameters();
    QPointF oldCenter(params.center.x(), params.center.y());
    QPointF delta = center - oldCenter;
    
    params.center = QVector3D(center.x(), center.y(), params.center.z());
    params.bounds.translate(delta.x(), delta.y());
    
    m_projectionManager->setViewParameters(params);
    emit viewChanged();
}

// 缩放控制实现
void TopDownInteractionController::zoom(float factor, const QPointF& center)
{
    if (!m_projectionManager) return;
    
    ViewParameters params = m_projectionManager->getViewParameters();
    
    float newZoom = clampZoom(params.zoom * factor);
    if (newZoom == params.zoom) return;
    
    params.zoom = newZoom;
    
    // 如果指定了缩放中心，调整视图中心以保持该点在屏幕上的位置不变
    if (!center.isNull()) {
        // 这里需要复杂的计算来保持缩放中心不变
        // 简化实现：直接应用缩放
    }
    
    m_projectionManager->setViewParameters(params);
    emit viewChanged();
}

void TopDownInteractionController::wheelZoom(const QPointF& center, int delta)
{
    float factor = (delta > 0) ? m_zoomSensitivity : (1.0f / m_zoomSensitivity);
    zoom(factor, center);
}

float TopDownInteractionController::getZoomLevel() const
{
    if (!m_projectionManager) return 1.0f;
    
    ViewParameters params = m_projectionManager->getViewParameters();
    return params.zoom;
}

void TopDownInteractionController::setZoomLevel(float zoom)
{
    if (!m_projectionManager) return;
    
    ViewParameters params = m_projectionManager->getViewParameters();
    params.zoom = clampZoom(zoom);
    
    m_projectionManager->setViewParameters(params);
    emit viewChanged();
}

void TopDownInteractionController::zoomToFit(float margin)
{
    if (!m_projectionManager) return;
    
    // 这需要知道当前显示的点云数据
    // 简化实现：重置到默认视图
    ViewParameters params = m_projectionManager->getViewParameters();
    params.zoom = 1.0f;
    params.center = QVector3D(0, 0, 0);
    
    m_projectionManager->setViewParameters(params);
    emit viewChanged();
}

// 选择控制实现
void TopDownInteractionController::setSelectionMode(SelectionMode mode)
{
    if (m_selectionMode != mode) {
        m_selectionMode = mode;
        clearSelection();
        emit statusMessage(QString("Selection mode changed to %1").arg(static_cast<int>(mode)));
    }
}

SelectionMode TopDownInteractionController::getSelectionMode() const
{
    return m_selectionMode;
}

SelectionResult TopDownInteractionController::selectPointsInRect(const QRectF& rect)
{
    SelectionResult result;
    result.boundingRect = rect;

    // 这里需要访问当前渲染的点云数据
    // 简化实现：返回模拟结果
    if (!rect.isEmpty()) {
        // 模拟选择了一些点
        for (int i = 0; i < 10; ++i) {
            result.pointIndices.push_back(i);
        }
        result.selectionCount = result.pointIndices.size();
    }

    return result;
}

SelectionResult TopDownInteractionController::selectPointsInCircle(const QPointF& center, float radius)
{
    SelectionResult result;

    // 将圆形转换为边界矩形进行简化处理
    QRectF boundingRect(center.x() - radius, center.y() - radius, 2 * radius, 2 * radius);
    result.boundingRect = boundingRect;

    // 简化实现：使用矩形选择
    return selectPointsInRect(boundingRect);
}

SelectionResult TopDownInteractionController::selectPointsInPolygon(const std::vector<QPointF>& polygon)
{
    SelectionResult result;

    if (polygon.size() < 3) {
        return result;
    }

    // 计算多边形边界框
    float minX = polygon[0].x(), maxX = polygon[0].x();
    float minY = polygon[0].y(), maxY = polygon[0].y();

    for (const auto& point : polygon) {
        minX = qMin(minX, static_cast<float>(point.x()));
        maxX = qMax(maxX, static_cast<float>(point.x()));
        minY = qMin(minY, static_cast<float>(point.y()));
        maxY = qMax(maxY, static_cast<float>(point.y()));
    }

    result.boundingRect = QRectF(minX, minY, maxX - minX, maxY - minY);

    // 简化实现：使用边界框选择
    return selectPointsInRect(result.boundingRect);
}

void TopDownInteractionController::clearSelection()
{
    m_currentSelection = SelectionResult();
    emit selectionChanged(m_currentSelection);
}

SelectionResult TopDownInteractionController::getCurrentSelection() const
{
    return m_currentSelection;
}

// 测量控制实现
void TopDownInteractionController::setMeasurementType(MeasurementType type)
{
    if (m_measurementType != type) {
        m_measurementType = type;
        m_tempMeasurementPoints.clear();
        emit statusMessage(QString("Measurement type changed to %1").arg(static_cast<int>(type)));
    }
}

MeasurementType TopDownInteractionController::getMeasurementType() const
{
    return m_measurementType;
}

float TopDownInteractionController::measureScreenDistance(const QPointF& point1, const QPointF& point2)
{
    QPointF delta = point2 - point1;
    return qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
}

float TopDownInteractionController::measureWorldDistance(const QPointF& point1, const QPointF& point2)
{
    if (!m_projectionManager) {
        return measureScreenDistance(point1, point2);
    }

    // 将屏幕坐标转换为世界坐标
    QVector3D world1 = m_projectionManager->screenToWorld(QVector2D(point1), 0.0f);
    QVector3D world2 = m_projectionManager->screenToWorld(QVector2D(point2), 0.0f);

    // 计算世界坐标距离
    QVector3D delta = world2 - world1;
    return delta.length();
}

float TopDownInteractionController::measureArea(const std::vector<QPointF>& points)
{
    if (points.size() < 3) {
        return 0.0f;
    }

    if (!m_projectionManager) {
        return 0.0f;
    }

    // 将屏幕坐标转换为世界坐标
    std::vector<QVector3D> worldPoints;
    worldPoints.reserve(points.size());

    for (const auto& point : points) {
        QVector3D worldPoint = m_projectionManager->screenToWorld(QVector2D(point), 0.0f);
        worldPoints.push_back(worldPoint);
    }

    // 使用鞋带公式计算多边形面积
    float area = 0.0f;
    for (size_t i = 0; i < worldPoints.size(); ++i) {
        size_t j = (i + 1) % worldPoints.size();
        area += worldPoints[i].x() * worldPoints[j].y();
        area -= worldPoints[j].x() * worldPoints[i].y();
    }

    return qAbs(area) * 0.5f;
}

float TopDownInteractionController::measureAngle(const QPointF& center, const QPointF& point1, const QPointF& point2)
{
    QPointF vec1 = point1 - center;
    QPointF vec2 = point2 - center;

    float dot = vec1.x() * vec2.x() + vec1.y() * vec2.y();
    float len1 = qSqrt(vec1.x() * vec1.x() + vec1.y() * vec1.y());
    float len2 = qSqrt(vec2.x() * vec2.x() + vec2.y() * vec2.y());

    if (len1 == 0.0f || len2 == 0.0f) {
        return 0.0f;
    }

    float cosAngle = dot / (len1 * len2);
    cosAngle = qBound(-1.0f, cosAngle, 1.0f);

    return qRadiansToDegrees(qAcos(cosAngle));
}

std::vector<MeasurementResult> TopDownInteractionController::getMeasurementResults() const
{
    return m_measurementResults;
}

void TopDownInteractionController::clearMeasurements()
{
    m_measurementResults.clear();
    m_tempMeasurementPoints.clear();
    emit statusMessage("Measurements cleared");
}

// 设置和配置
void TopDownInteractionController::setPanSensitivity(float sensitivity)
{
    m_panSensitivity = qMax(0.1f, sensitivity);
}

void TopDownInteractionController::setZoomSensitivity(float sensitivity)
{
    m_zoomSensitivity = qMax(1.01f, sensitivity);
}

void TopDownInteractionController::setMinZoom(float minZoom)
{
    m_minZoom = qMax(0.01f, minZoom);
}

void TopDownInteractionController::setMaxZoom(float maxZoom)
{
    m_maxZoom = qMax(m_minZoom, maxZoom);
}

void TopDownInteractionController::setInertialScrollingEnabled(bool enabled)
{
    m_inertialScrollingEnabled = enabled;
}

// 私有辅助方法
void TopDownInteractionController::updateInteractionState()
{
    // 更新交互状态的内部逻辑
}

bool TopDownInteractionController::isPointInRect(const QPointF& point, const QRectF& rect) const
{
    return rect.contains(point);
}

bool TopDownInteractionController::isPointInCircle(const QPointF& point, const QPointF& center, float radius) const
{
    QPointF delta = point - center;
    float distance = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
    return distance <= radius;
}

bool TopDownInteractionController::isPointInPolygon(const QPointF& point, const std::vector<QPointF>& polygon) const
{
    if (polygon.size() < 3) {
        return false;
    }

    // 射线投射算法
    bool inside = false;
    for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        if (((polygon[i].y() > point.y()) != (polygon[j].y() > point.y())) &&
            (point.x() < (polygon[j].x() - polygon[i].x()) * (point.y() - polygon[i].y()) /
             (polygon[j].y() - polygon[i].y()) + polygon[i].x())) {
            inside = !inside;
        }
    }

    return inside;
}

float TopDownInteractionController::clampZoom(float zoom) const
{
    return qBound(m_minZoom, zoom, m_maxZoom);
}

} // namespace WallExtraction
