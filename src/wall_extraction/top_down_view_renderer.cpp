#include "top_down_view_renderer.h"
#include "color_mapping_manager.h"
#include "view_projection_manager.h"
#include "top_down_interaction_controller.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QPainter>
#include <QtMath>

namespace WallExtraction {

TopDownViewRenderer::TopDownViewRenderer(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_viewportSize(800, 600)
    , m_viewBounds(-100.0f, 100.0f, -100.0f, 100.0f)
    , m_renderMode(TopDownRenderMode::Points)
    , m_pointSize(2.0f)
    , m_antiAliasingEnabled(true)
    , m_backgroundColor(Qt::black)
    , m_statisticsValid(false)
    , m_totalRenderCalls(0)
    , m_totalRenderTime(0)
    , m_lastPointCount(0)
{
    // 注册元类型
    qRegisterMetaType<TopDownRenderMode>("TopDownRenderMode");
    qRegisterMetaType<ProjectionType>("ProjectionType");
    qRegisterMetaType<ColoredPoint>("ColoredPoint");
    
    // 创建子组件
    m_colorMapper = std::make_unique<ColorMappingManager>(this);
    m_projectionManager = std::make_unique<ViewProjectionManager>(this);
    m_interactionController = std::make_unique<TopDownInteractionController>(this);
    
    // 设置子组件关联
    m_interactionController->setRenderer(this);
    m_interactionController->setProjectionManager(m_projectionManager.get());
    
    // 初始化渲染缓冲区
    m_renderBuffer = QImage(m_viewportSize, QImage::Format_ARGB32);
    m_renderBuffer.fill(m_backgroundColor);
    
    // 连接信号
    connect(m_projectionManager.get(), &ViewProjectionManager::projectionParametersChanged,
            this, &TopDownViewRenderer::viewParametersChanged);
    connect(m_colorMapper.get(), &ColorMappingManager::colorSchemeChanged,
            this, &TopDownViewRenderer::viewParametersChanged);
    
    m_initialized = true;
    qDebug() << "TopDownViewRenderer created with" << m_viewportSize << "viewport";
}

TopDownViewRenderer::~TopDownViewRenderer()
{
    qDebug() << "TopDownViewRenderer destroyed";
}

bool TopDownViewRenderer::isInitialized() const
{
    return m_initialized;
}

void TopDownViewRenderer::setViewportSize(const QSize& size)
{
    if (m_viewportSize != size) {
        m_viewportSize = size;
        
        // 更新投影管理器
        m_projectionManager->setViewportSize(size);
        
        // 重新创建渲染缓冲区
        m_renderBuffer = QImage(size, QImage::Format_ARGB32);
        m_renderBuffer.fill(m_backgroundColor);
        
        emit viewParametersChanged();
        emit statusMessage(QString("Viewport size changed to %1x%2").arg(size.width()).arg(size.height()));
    }
}

QSize TopDownViewRenderer::getViewportSize() const
{
    return m_viewportSize;
}

void TopDownViewRenderer::setViewBounds(float minX, float maxX, float minY, float maxY)
{
    QRectF bounds(minX, minY, maxX - minX, maxY - minY);
    if (m_viewBounds != bounds) {
        m_viewBounds = bounds;
        
        // 更新投影管理器
        m_projectionManager->setViewBounds(bounds);
        
        emit viewParametersChanged();
    }
}

QRectF TopDownViewRenderer::getViewBounds() const
{
    return m_viewBounds;
}

void TopDownViewRenderer::setRenderMode(TopDownRenderMode mode)
{
    if (m_renderMode != mode) {
        m_renderMode = mode;
        
        emit viewParametersChanged();
        emit statusMessage(QString("Render mode changed to %1").arg(static_cast<int>(mode)));
    }
}

TopDownRenderMode TopDownViewRenderer::getRenderMode() const
{
    return m_renderMode;
}

void TopDownViewRenderer::setPointSize(float size)
{
    if (size > 0.0f && m_pointSize != size) {
        m_pointSize = size;
        
        emit viewParametersChanged();
        emit statusMessage(QString("Point size set to %1").arg(size));
    }
}

float TopDownViewRenderer::getPointSize() const
{
    return m_pointSize;
}

bool TopDownViewRenderer::renderTopDownView(const std::vector<PointWithAttributes>& points)
{
    if (points.empty()) {
        emit errorOccurred("Cannot render empty point cloud");
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    emit statusMessage(QString("Rendering %1 points in top-down view...").arg(points.size()));
    qDebug() << "=== TopDownViewRenderer::renderTopDownView ===";
    qDebug() << "Input points:" << points.size();
    qDebug() << "Viewport size:" << m_viewportSize;
    qDebug() << "Render mode:" << static_cast<int>(m_renderMode);
    qDebug() << "Point size:" << m_pointSize;

    try {
        // 预处理点云数据
        qDebug() << "Starting preprocessing...";
        auto coloredPoints = preprocessPoints(points);
        qDebug() << "After preprocessing:" << coloredPoints.size() << "colored points";

        if (coloredPoints.empty()) {
            qDebug() << "ERROR: Preprocessing failed - analyzing input data...";

            // 分析输入数据
            if (!points.empty()) {
                const auto& firstPoint = points[0];
                qDebug() << "First input point position:" << firstPoint.position;
                qDebug() << "First input point attributes count:" << firstPoint.attributes.size();

                // 检查点云边界
                float minX = firstPoint.position.x(), maxX = minX;
                float minY = firstPoint.position.y(), maxY = minY;
                float minZ = firstPoint.position.z(), maxZ = minZ;

                for (const auto& point : points) {
                    minX = qMin(minX, point.position.x());
                    maxX = qMax(maxX, point.position.x());
                    minY = qMin(minY, point.position.y());
                    maxY = qMax(maxY, point.position.y());
                    minZ = qMin(minZ, point.position.z());
                    maxZ = qMax(maxZ, point.position.z());
                }

                qDebug() << "Input data bounds:";
                qDebug() << "  X:" << minX << "to" << maxX << "(range:" << (maxX - minX) << ")";
                qDebug() << "  Y:" << minY << "to" << maxY << "(range:" << (maxY - minY) << ")";
                qDebug() << "  Z:" << minZ << "to" << maxZ << "(range:" << (maxZ - minZ) << ")";
            }

            emit errorOccurred("Preprocessing failed: no colored points generated");
            return false;
        }

        // 检查前几个点的投影结果
        if (!coloredPoints.empty()) {
            for (int i = 0; i < qMin(5, static_cast<int>(coloredPoints.size())); ++i) {
                const auto& cp = coloredPoints[i];
                qDebug() << "Point" << i << "- Screen pos:" << cp.screenPosition
                        << "Color:" << cp.color << "Depth:" << cp.depth;
            }
        }

        // 应用视锥体剔除
        qDebug() << "Starting culling...";
        qDebug() << "Viewport size before culling:" << m_viewportSize;
        qDebug() << "View bounds:" << m_viewBounds.left() << m_viewBounds.top() << m_viewBounds.right() << m_viewBounds.bottom();

        auto originalColoredPoints = coloredPoints;
        coloredPoints = applyCulling(coloredPoints);
        qDebug() << "After culling:" << coloredPoints.size() << "visible points";

        if (coloredPoints.empty()) {
            qDebug() << "WARNING: All points culled! Attempting recovery...";

            // 尝试恢复：使用更宽松的剔除策略
            coloredPoints = applyLenientCulling(originalColoredPoints);
            qDebug() << "After lenient culling:" << coloredPoints.size() << "visible points";

            if (coloredPoints.empty()) {
                // 最后尝试：直接使用前1000个点进行调试
                qDebug() << "Using debug rendering with first 1000 points...";
                size_t debugCount = qMin(static_cast<size_t>(1000), originalColoredPoints.size());
                coloredPoints.assign(originalColoredPoints.begin(), originalColoredPoints.begin() + debugCount);
                qDebug() << "Debug rendering points:" << coloredPoints.size();

                if (coloredPoints.empty()) {
                    emit errorOccurred("Critical: No points available for rendering");
                    return false;
                }
            }
        }

        // 清除渲染缓冲区
        qDebug() << "Clearing render buffer...";
        clearRenderBuffer();

        // 验证渲染缓冲区状态
        if (m_renderBuffer.isNull() || m_renderBuffer.size().isEmpty()) {
            qDebug() << "ERROR: Render buffer is invalid";
            qDebug() << "Buffer null:" << m_renderBuffer.isNull();
            qDebug() << "Buffer size:" << m_renderBuffer.size();
            emit errorOccurred("Render buffer initialization failed");
            return false;
        }

        qDebug() << "Render buffer ready:" << m_renderBuffer.size();
        
        // 根据渲染模式进行渲染
        bool success = false;
        switch (m_renderMode) {
            case TopDownRenderMode::Points:
                success = renderPointMode(coloredPoints);
                break;
            case TopDownRenderMode::Density:
                success = renderDensityMode(coloredPoints);
                break;
            case TopDownRenderMode::Contour:
                success = renderContourMode(coloredPoints);
                break;
            case TopDownRenderMode::Heatmap:
                success = renderHeatmapMode(coloredPoints);
                break;
        }
        
        if (success) {
            qint64 renderTime = timer.elapsed();
            updateRenderStatistics(points.size(), renderTime);

            qDebug() << "Rendering successful!";
            qDebug() << "Render buffer size:" << m_renderBuffer.size();
            qDebug() << "Render buffer format:" << m_renderBuffer.format();
            qDebug() << "Render time:" << renderTime << "ms";

            emit renderingCompleted(renderTime);
            emit statusMessage(QString("Rendering completed in %1 ms").arg(renderTime));

            return true;
        } else {
            qDebug() << "Rendering failed in render mode switch";
            emit errorOccurred("Rendering failed");
            return false;
        }
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Exception during rendering: %1").arg(e.what()));
        return false;
    }
}

bool TopDownViewRenderer::renderTopDownView(const std::vector<QVector3D>& points)
{
    if (points.empty()) {
        emit errorOccurred("Cannot render empty point cloud");
        return false;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        // 预处理点云数据
        auto coloredPoints = preprocessPoints(points);
        
        // 应用视锥体剔除
        coloredPoints = applyCulling(coloredPoints);
        
        // 清除渲染缓冲区
        clearRenderBuffer();
        
        // 根据渲染模式进行渲染
        bool success = false;
        switch (m_renderMode) {
            case TopDownRenderMode::Points:
                success = renderPointMode(coloredPoints);
                break;
            case TopDownRenderMode::Density:
                success = renderDensityMode(coloredPoints);
                break;
            case TopDownRenderMode::Contour:
                success = renderContourMode(coloredPoints);
                break;
            case TopDownRenderMode::Heatmap:
                success = renderHeatmapMode(coloredPoints);
                break;
        }
        
        if (success) {
            qint64 renderTime = timer.elapsed();
            updateRenderStatistics(points.size(), renderTime);
            
            emit renderingCompleted(renderTime);
            return true;
        } else {
            emit errorOccurred("Rendering failed");
            return false;
        }
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Exception during rendering: %1").arg(e.what()));
        return false;
    }
}

QImage TopDownViewRenderer::getRenderBuffer() const
{
    return m_renderBuffer;
}

bool TopDownViewRenderer::saveRenderResult(const QString& filename, const QString& format)
{
    bool success = m_renderBuffer.save(filename, format.toLocal8Bit().constData());
    if (success) {
        emit statusMessage(QString("Render result saved to %1").arg(filename));
    } else {
        emit errorOccurred(QString("Failed to save render result to %1").arg(filename));
    }
    return success;
}

TopDownInteractionController* TopDownViewRenderer::getInteractionController() const
{
    return m_interactionController.get();
}

ColorMappingManager* TopDownViewRenderer::getColorMappingManager() const
{
    return m_colorMapper.get();
}

ViewProjectionManager* TopDownViewRenderer::getProjectionManager() const
{
    return m_projectionManager.get();
}

void TopDownViewRenderer::setAntiAliasingEnabled(bool enabled)
{
    if (m_antiAliasingEnabled != enabled) {
        m_antiAliasingEnabled = enabled;
        emit statusMessage(QString("Anti-aliasing %1").arg(enabled ? "enabled" : "disabled"));
    }
}

bool TopDownViewRenderer::isAntiAliasingEnabled() const
{
    return m_antiAliasingEnabled;
}

void TopDownViewRenderer::setBackgroundColor(const QColor& color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        m_renderBuffer.fill(color);
        emit viewParametersChanged();
    }
}

QColor TopDownViewRenderer::getBackgroundColor() const
{
    return m_backgroundColor;
}

void TopDownViewRenderer::clearRenderBuffer()
{
    m_renderBuffer.fill(m_backgroundColor);
}

QVariantMap TopDownViewRenderer::getRenderStatistics() const
{
    if (!m_statisticsValid) {
        const_cast<TopDownViewRenderer*>(this)->updateRenderStatistics(m_lastPointCount, 0);
    }
    return m_renderStatistics;
}

// 私有方法实现
bool TopDownViewRenderer::renderPointMode(const std::vector<ColoredPoint>& coloredPoints)
{
    qDebug() << "=== Rendering Point Mode ===";
    qDebug() << "Points to render:" << coloredPoints.size();
    qDebug() << "Point size:" << m_pointSize;
    qDebug() << "Render buffer size:" << m_renderBuffer.size();

    QPainter painter(&m_renderBuffer);

    if (m_antiAliasingEnabled) {
        painter.setRenderHint(QPainter::Antialiasing, true);
    }

    // 改进的点绘制方式
    int validPoints = 0;
    int processedCount = 0;

    for (const auto& point : coloredPoints) {
        QPointF screenPos(point.screenPosition.x(), point.screenPosition.y());

        // 检查点是否在有效范围内
        bool inBounds = (screenPos.x() >= 0 && screenPos.x() < m_renderBuffer.width() &&
                        screenPos.y() >= 0 && screenPos.y() < m_renderBuffer.height());

        if (inBounds) {
            // 使用更明显的绘制方式
            if (m_pointSize <= 1.0f) {
                // 小点：直接设置像素
                m_renderBuffer.setPixelColor(static_cast<int>(screenPos.x()),
                                           static_cast<int>(screenPos.y()),
                                           point.color);
            } else {
                // 大点：绘制圆形
                painter.setBrush(QBrush(point.color));
                painter.setPen(QPen(point.color, 1));
                float radius = m_pointSize * 0.5f;
                painter.drawEllipse(screenPos, radius, radius);
            }

            validPoints++;
        } else {
            // 记录边界外的点（仅前几个用于调试）
            if (processedCount < 5) {
                qDebug() << "Point" << processedCount << "out of bounds: ("
                        << screenPos.x() << "," << screenPos.y()
                        << ") buffer size:" << m_renderBuffer.width() << "x" << m_renderBuffer.height();
            }
        }

        // 更新进度和调试信息
        if (++processedCount % 10000 == 0) {
            int progress = static_cast<int>((processedCount * 100) / coloredPoints.size());
            emit renderingProgress(progress);
            qDebug() << "Progress:" << progress << "% - Valid points so far:" << validPoints;
        }
    }

    qDebug() << "Point mode rendering completed";
    qDebug() << "Total processed:" << processedCount;
    qDebug() << "Valid points rendered:" << validPoints;

    // 详细的渲染结果分析
    double renderRatio = coloredPoints.empty() ? 0.0 : (static_cast<double>(validPoints) / coloredPoints.size() * 100.0);
    qDebug() << "Render success ratio:" << renderRatio << "%";

    if (validPoints == 0) {
        qDebug() << "ERROR: No points were successfully rendered";
        qDebug() << "Possible causes:";
        qDebug() << "  - All points outside viewport bounds";
        qDebug() << "  - Invalid screen coordinates";
        qDebug() << "  - Projection transformation failed";
        emit renderingProgress(100);
        return false;
    } else if (renderRatio < 1.0) {
        qDebug() << "WARNING: Low render success ratio (" << renderRatio << "%)";
    }

    emit renderingProgress(100);
    return validPoints > 0; // 只有在至少渲染了一个点时才返回成功
}

bool TopDownViewRenderer::renderDensityMode(const std::vector<ColoredPoint>& coloredPoints)
{
    // 创建密度网格
    int gridWidth = m_viewportSize.width() / 4;  // 降低分辨率提高性能
    int gridHeight = m_viewportSize.height() / 4;

    auto densityData = calculateDensityGrid(coloredPoints, gridWidth, gridHeight);
    drawDensityGrid(densityData, gridWidth, gridHeight);

    return true;
}

bool TopDownViewRenderer::renderContourMode(const std::vector<ColoredPoint>& coloredPoints)
{
    // 简化的等高线实现
    QPainter painter(&m_renderBuffer);

    if (m_antiAliasingEnabled) {
        painter.setRenderHint(QPainter::Antialiasing, true);
    }

    // 按深度分组绘制等高线
    std::map<int, std::vector<ColoredPoint>> depthGroups;

    for (const auto& point : coloredPoints) {
        int depthLevel = static_cast<int>(point.depth * 10); // 0.1米间隔
        depthGroups[depthLevel].push_back(point);
    }

    // 绘制每个深度级别
    for (const auto& group : depthGroups) {
        if (group.second.size() < 3) continue; // 需要至少3个点形成轮廓

        QPen pen(QColor::fromHsv((group.first * 30) % 360, 255, 255), 1.0f);
        painter.setPen(pen);

        // 简化：直接连接点形成轮廓
        for (size_t i = 1; i < group.second.size(); ++i) {
            QPointF p1(group.second[i-1].screenPosition.x(), group.second[i-1].screenPosition.y());
            QPointF p2(group.second[i].screenPosition.x(), group.second[i].screenPosition.y());
            painter.drawLine(p1, p2);
        }
    }

    return true;
}

bool TopDownViewRenderer::renderHeatmapMode(const std::vector<ColoredPoint>& coloredPoints)
{
    // 创建热力图
    int gridWidth = m_viewportSize.width() / 2;
    int gridHeight = m_viewportSize.height() / 2;

    std::vector<float> heatData(gridWidth * gridHeight, 0.0f);

    // 计算热力值
    for (const auto& point : coloredPoints) {
        int x = static_cast<int>(point.screenPosition.x() * gridWidth / m_viewportSize.width());
        int y = static_cast<int>(point.screenPosition.y() * gridHeight / m_viewportSize.height());

        if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
            int index = y * gridWidth + x;
            heatData[index] += 1.0f;
        }
    }

    // 应用高斯模糊效果
    std::vector<float> blurredData = heatData;
    int radius = 3;
    for (int y = radius; y < gridHeight - radius; ++y) {
        for (int x = radius; x < gridWidth - radius; ++x) {
            float sum = 0.0f;
            int count = 0;

            for (int dy = -radius; dy <= radius; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight) {
                        sum += heatData[ny * gridWidth + nx];
                        count++;
                    }
                }
            }

            blurredData[y * gridWidth + x] = sum / count;
        }
    }

    // 绘制热力图
    QPainter painter(&m_renderBuffer);

    float maxHeat = *std::max_element(blurredData.begin(), blurredData.end());
    if (maxHeat > 0.0f) {
        for (int y = 0; y < gridHeight; ++y) {
            for (int x = 0; x < gridWidth; ++x) {
                float heat = blurredData[y * gridWidth + x] / maxHeat;

                if (heat > 0.01f) { // 只绘制有热力的区域
                    // 热力图颜色：蓝色->绿色->黄色->红色
                    QColor heatColor;
                    if (heat < 0.25f) {
                        heatColor = QColor::fromRgbF(0, 0, heat * 4);
                    } else if (heat < 0.5f) {
                        heatColor = QColor::fromRgbF(0, (heat - 0.25f) * 4, 1);
                    } else if (heat < 0.75f) {
                        heatColor = QColor::fromRgbF((heat - 0.5f) * 4, 1, 1 - (heat - 0.5f) * 4);
                    } else {
                        heatColor = QColor::fromRgbF(1, 1 - (heat - 0.75f) * 4, 0);
                    }

                    heatColor.setAlphaF(heat * 0.8f);

                    QRectF rect(x * m_viewportSize.width() / gridWidth,
                               y * m_viewportSize.height() / gridHeight,
                               m_viewportSize.width() / gridWidth,
                               m_viewportSize.height() / gridHeight);

                    painter.fillRect(rect, heatColor);
                }
            }
        }
    }

    return true;
}

std::vector<ColoredPoint> TopDownViewRenderer::preprocessPoints(const std::vector<PointWithAttributes>& points)
{
    // 应用颜色映射
    auto coloredPoints = m_colorMapper->applyColorMapping(points);

    // 应用投影变换
    std::vector<QVector3D> positions;
    positions.reserve(points.size());
    for (const auto& point : points) {
        positions.push_back(point.position);
    }

    auto projectionResults = m_projectionManager->projectToTopDown(positions);

    // 合并结果
    for (size_t i = 0; i < coloredPoints.size() && i < projectionResults.size(); ++i) {
        coloredPoints[i].screenPosition = projectionResults[i].screenPosition;
        coloredPoints[i].depth = projectionResults[i].depth;
    }

    return coloredPoints;
}

std::vector<ColoredPoint> TopDownViewRenderer::preprocessPoints(const std::vector<QVector3D>& points)
{
    qDebug() << "=== TopDownViewRenderer::preprocessPoints (QVector3D) ===";
    qDebug() << "Input points:" << points.size();

    // 应用颜色映射
    auto coloredPoints = m_colorMapper->applyColorMapping(points);
    qDebug() << "After color mapping:" << coloredPoints.size() << "colored points";

    // 应用投影变换
    auto projectionResults = m_projectionManager->projectToTopDown(points);
    qDebug() << "After projection:" << projectionResults.size() << "projection results";

    // 合并结果
    size_t validProjections = 0;
    for (size_t i = 0; i < coloredPoints.size() && i < projectionResults.size(); ++i) {
        coloredPoints[i].screenPosition = projectionResults[i].screenPosition;
        coloredPoints[i].depth = projectionResults[i].depth;

        // 检查投影是否有效
        if (projectionResults[i].isVisible) {
            validProjections++;
        }

        // 调试前几个点的投影结果
        if (i < 5) {
            qDebug() << "Point" << i << "- World:" << points[i]
                    << "Screen:" << projectionResults[i].screenPosition
                    << "Visible:" << projectionResults[i].isVisible
                    << "Color:" << coloredPoints[i].color;
        }
    }

    qDebug() << "Valid projections:" << validProjections << "out of" << projectionResults.size();
    qDebug() << "Preprocessing completed";

    return coloredPoints;
}

std::vector<ColoredPoint> TopDownViewRenderer::applyCulling(const std::vector<ColoredPoint>& coloredPoints)
{
    qDebug() << "=== Applying Culling ===";
    qDebug() << "Input points:" << coloredPoints.size();
    qDebug() << "Viewport size:" << m_viewportSize;

    std::vector<ColoredPoint> culledPoints;
    culledPoints.reserve(coloredPoints.size());

    int insideCount = 0;
    int outsideCount = 0;

    for (const auto& point : coloredPoints) {
        // 检查点是否在视口范围内
        bool inside = (point.screenPosition.x() >= 0 && point.screenPosition.x() < m_viewportSize.width() &&
                      point.screenPosition.y() >= 0 && point.screenPosition.y() < m_viewportSize.height());

        if (inside) {
            culledPoints.push_back(point);
            insideCount++;
        } else {
            outsideCount++;
        }

        // 调试前几个点的剔除结果
        if (culledPoints.size() < 5 && inside) {
            qDebug() << "Culled point" << culledPoints.size()-1 << "- Screen:" << point.screenPosition
                    << "Color:" << point.color;
        }
    }

    qDebug() << "Culling results: Inside=" << insideCount << "Outside=" << outsideCount;
    qDebug() << "Culling completed, returning" << culledPoints.size() << "points";

    return culledPoints;
}

std::vector<WallExtraction::ColoredPoint> TopDownViewRenderer::applyLenientCulling(const std::vector<WallExtraction::ColoredPoint>& coloredPoints)
{
    qDebug() << "=== Applying Lenient Culling ===";
    qDebug() << "Input points:" << coloredPoints.size();

    std::vector<WallExtraction::ColoredPoint> culledPoints;
    culledPoints.reserve(coloredPoints.size());

    // 使用更宽松的边界（扩大50%）
    float margin = 0.5f;
    float left = m_viewBounds.left() - m_viewBounds.width() * margin;
    float right = m_viewBounds.right() + m_viewBounds.width() * margin;
    float top = m_viewBounds.top() - m_viewBounds.height() * margin;
    float bottom = m_viewBounds.bottom() + m_viewBounds.height() * margin;

    qDebug() << "Lenient bounds:" << left << top << right << bottom;

    int culledCount = 0;
    for (const auto& point : coloredPoints) {
        float x = point.screenPosition.x();
        float y = point.screenPosition.y();

        // 宽松的边界检查
        if (x >= left && x <= right && y >= top && y <= bottom) {
            culledPoints.push_back(point);
        } else {
            culledCount++;
        }
    }

    qDebug() << "Lenient culling result:" << culledPoints.size() << "kept," << culledCount << "culled";

    // 如果仍然没有点，尝试保留所有点
    if (culledPoints.empty() && !coloredPoints.empty()) {
        qDebug() << "Lenient culling still failed, keeping all points for debugging";
        culledPoints = coloredPoints;
    }

    return culledPoints;
}

void TopDownViewRenderer::drawPoint(const ColoredPoint& point)
{
    QPainter painter(&m_renderBuffer);

    QPen pen(point.color, m_pointSize, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pen);

    QPointF screenPos(point.screenPosition.x(), point.screenPosition.y());
    painter.drawPoint(screenPos);
}

std::vector<float> TopDownViewRenderer::calculateDensityGrid(const std::vector<ColoredPoint>& points,
                                                            int gridWidth, int gridHeight)
{
    std::vector<float> densityData(gridWidth * gridHeight, 0.0f);

    for (const auto& point : points) {
        int x = static_cast<int>(point.screenPosition.x() * gridWidth / m_viewportSize.width());
        int y = static_cast<int>(point.screenPosition.y() * gridHeight / m_viewportSize.height());

        if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
            int index = y * gridWidth + x;
            densityData[index] += 1.0f;
        }
    }

    return densityData;
}

void TopDownViewRenderer::drawDensityGrid(const std::vector<float>& gridData, int gridWidth, int gridHeight)
{
    QPainter painter(&m_renderBuffer);

    float maxDensity = *std::max_element(gridData.begin(), gridData.end());
    if (maxDensity <= 0.0f) return;

    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            float density = gridData[y * gridWidth + x] / maxDensity;

            if (density > 0.01f) {
                QColor densityColor = QColor::fromRgbF(density, density, density);

                QRectF rect(x * m_viewportSize.width() / gridWidth,
                           y * m_viewportSize.height() / gridHeight,
                           m_viewportSize.width() / gridWidth,
                           m_viewportSize.height() / gridHeight);

                painter.fillRect(rect, densityColor);
            }
        }
    }
}

void TopDownViewRenderer::updateRenderStatistics(size_t pointCount, qint64 renderTime)
{
    m_totalRenderCalls++;
    m_totalRenderTime += renderTime;
    m_lastPointCount = pointCount;

    m_renderStatistics.clear();
    m_renderStatistics["render_mode"] = static_cast<int>(m_renderMode);
    m_renderStatistics["viewport_width"] = m_viewportSize.width();
    m_renderStatistics["viewport_height"] = m_viewportSize.height();
    m_renderStatistics["point_size"] = m_pointSize;
    m_renderStatistics["anti_aliasing"] = m_antiAliasingEnabled;
    m_renderStatistics["last_point_count"] = static_cast<qulonglong>(pointCount);
    m_renderStatistics["last_render_time_ms"] = renderTime;
    m_renderStatistics["total_render_calls"] = static_cast<qulonglong>(m_totalRenderCalls);
    m_renderStatistics["total_render_time_ms"] = m_totalRenderTime;
    m_renderStatistics["avg_render_time_ms"] = m_totalRenderCalls > 0 ?
        static_cast<double>(m_totalRenderTime) / m_totalRenderCalls : 0.0;

    m_statisticsValid = true;
}

} // namespace WallExtraction
