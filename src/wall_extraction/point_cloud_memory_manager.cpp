#include "point_cloud_memory_manager.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

namespace WallExtraction {

PointCloudMemoryManager::PointCloudMemoryManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_strategy(MemoryStrategy::LRU)
    , m_maxMemoryUsage(1024 * 1024 * 1024) // 默认1GB
    , m_autoMemoryManagement(true)
    , m_currentMemoryUsage(0)
    , m_statisticsValid(false)
    , m_totalLoadOperations(0)
    , m_totalUnloadOperations(0)
    , m_totalLoadTime(0)
    , m_totalUnloadTime(0)
{
    // 注册元类型
    qRegisterMetaType<MemoryStrategy>("MemoryStrategy");
    qRegisterMetaType<PointCloudChunk>("PointCloudChunk");
    
    m_initialized = true;
    qDebug() << "PointCloudMemoryManager created with max memory:" << (m_maxMemoryUsage / (1024*1024)) << "MB";
}

PointCloudMemoryManager::~PointCloudMemoryManager()
{
    clearAllData();
    qDebug() << "PointCloudMemoryManager destroyed";
}

bool PointCloudMemoryManager::isInitialized() const
{
    return m_initialized;
}

void PointCloudMemoryManager::setMemoryStrategy(MemoryStrategy strategy)
{
    if (m_strategy != strategy) {
        m_strategy = strategy;
        emit statusMessage(QString("Memory strategy changed to %1").arg(static_cast<int>(strategy)));
    }
}

MemoryStrategy PointCloudMemoryManager::getMemoryStrategy() const
{
    return m_strategy;
}

void PointCloudMemoryManager::setMaxMemoryUsage(size_t maxMemoryMB)
{
    size_t newMaxMemory = maxMemoryMB * 1024 * 1024;
    if (newMaxMemory != m_maxMemoryUsage) {
        m_maxMemoryUsage = newMaxMemory;
        
        emit statusMessage(QString("Max memory usage set to %1 MB").arg(maxMemoryMB));
        emit memoryUsageChanged(m_currentMemoryUsage, m_maxMemoryUsage);
        
        // 如果当前使用量超过新限制，执行清理
        if (m_currentMemoryUsage > m_maxMemoryUsage && m_autoMemoryManagement) {
            performMemoryCleanup(m_maxMemoryUsage * 0.8); // 清理到80%
        }
    }
}

size_t PointCloudMemoryManager::getMaxMemoryUsage() const
{
    return m_maxMemoryUsage / (1024 * 1024); // 返回MB
}

bool PointCloudMemoryManager::loadPointCloudChunked(const std::vector<QVector3D>& points, size_t chunkSize)
{
    if (points.empty() || chunkSize == 0) {
        emit errorOccurred("Invalid parameters for chunked loading");
        return false;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    emit statusMessage(QString("Loading %1 points in chunks of %2...")
                      .arg(points.size()).arg(chunkSize));
    
    // 清除现有数据
    clearAllData();
    
    try {
        // 计算需要的块数量
        size_t totalChunks = (points.size() + chunkSize - 1) / chunkSize;
        m_chunks.reserve(totalChunks);
        
        // 创建块
        for (size_t i = 0; i < points.size(); i += chunkSize) {
            size_t endIndex = qMin(i + chunkSize, points.size());
            
            auto chunk = createChunk(points, i, endIndex);
            if (chunk) {
                m_chunks.push_back(std::move(chunk));
            }
        }
        
        // 初始加载一些块
        size_t initialLoadCount = qMin(static_cast<size_t>(4), m_chunks.size());
        for (size_t i = 0; i < initialLoadCount; ++i) {
            if (m_chunks[i] && !m_chunks[i]->isLoaded) {
                m_chunks[i]->isLoaded = true;
                m_chunks[i]->lastAccessTime = QDateTime::currentMSecsSinceEpoch();
                m_loadedChunks.push_back(i);
                m_currentMemoryUsage += m_chunks[i]->memoryUsage;
                
                emit chunkLoaded(i);
            }
        }
        
        m_totalLoadOperations++;
        m_totalLoadTime += timer.elapsed();
        
        updateMemoryStatistics();
        
        emit statusMessage(QString("Loaded %1 chunks (%2 initially loaded) in %3 ms")
                          .arg(m_chunks.size()).arg(initialLoadCount).arg(timer.elapsed()));
        
        emit memoryUsageChanged(m_currentMemoryUsage, m_maxMemoryUsage);
        
        return true;
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Exception during chunked loading: %1").arg(e.what()));
        clearAllData();
        return false;
    }
}

size_t PointCloudMemoryManager::getLoadedChunkCount() const
{
    return m_loadedChunks.size();
}

size_t PointCloudMemoryManager::getTotalChunkCount() const
{
    return m_chunks.size();
}

std::vector<QVector3D> PointCloudMemoryManager::getChunkPoints(size_t chunkIndex) const
{
    if (chunkIndex >= m_chunks.size()) {
        qWarning() << "Invalid chunk index:" << chunkIndex;
        return {};
    }
    
    auto& chunk = m_chunks[chunkIndex];
    if (!chunk) {
        return {};
    }
    
    // 更新访问时间
    const_cast<PointCloudMemoryManager*>(this)->updateChunkAccessTime(chunkIndex);
    
    // 如果块未加载，尝试加载
    if (!chunk->isLoaded) {
        // 检查内存限制
        if (m_currentMemoryUsage + chunk->memoryUsage > m_maxMemoryUsage) {
            if (m_autoMemoryManagement) {
                const_cast<PointCloudMemoryManager*>(this)->performMemoryCleanup(
                    m_maxMemoryUsage - chunk->memoryUsage);
            } else {
                const_cast<PointCloudMemoryManager*>(this)->emitMemoryWarning("Memory limit would be exceeded");
                return {};
            }
        }
        
        // 加载块
        chunk->isLoaded = true;
        chunk->lastAccessTime = QDateTime::currentMSecsSinceEpoch();
        const_cast<PointCloudMemoryManager*>(this)->m_loadedChunks.push_back(chunkIndex);
        const_cast<PointCloudMemoryManager*>(this)->m_currentMemoryUsage += chunk->memoryUsage;

        const_cast<PointCloudMemoryManager*>(this)->emitChunkLoaded(chunkIndex);
        const_cast<PointCloudMemoryManager*>(this)->emitMemoryUsageChanged(m_currentMemoryUsage, m_maxMemoryUsage);
    }
    
    return chunk->points;
}

std::vector<QVector3D> PointCloudMemoryManager::getPointsForRendering(int lodLevel) const
{
    std::vector<QVector3D> renderPoints;
    
    // 根据LOD级别决定加载哪些块
    size_t maxChunks = qMax(1, static_cast<int>(m_chunks.size()) >> lodLevel);
    
    for (size_t i = 0; i < qMin(maxChunks, m_chunks.size()); ++i) {
        auto chunkPoints = getChunkPoints(i);
        
        // 根据LOD级别进行下采样
        size_t step = 1 << lodLevel; // 2^lodLevel
        for (size_t j = 0; j < chunkPoints.size(); j += step) {
            renderPoints.push_back(chunkPoints[j]);
        }
    }
    
    return renderPoints;
}

std::vector<QVector3D> PointCloudMemoryManager::getVisiblePoints(const QVector3D& viewPosition,
                                                                const QVector3D& viewDirection,
                                                                float fov,
                                                                float nearPlane,
                                                                float farPlane) const
{
    std::vector<QVector3D> visiblePoints;
    
    for (size_t i = 0; i < m_chunks.size(); ++i) {
        if (m_chunks[i] && isChunkVisible(*m_chunks[i], viewPosition, viewDirection, fov, nearPlane, farPlane)) {
            auto chunkPoints = getChunkPoints(i);
            visiblePoints.insert(visiblePoints.end(), chunkPoints.begin(), chunkPoints.end());
        }
    }
    
    return visiblePoints;
}

bool PointCloudMemoryManager::preloadRegion(const QVector3D& center, float radius)
{
    QElapsedTimer timer;
    timer.start();
    
    size_t loadedCount = 0;
    
    for (size_t i = 0; i < m_chunks.size(); ++i) {
        if (m_chunks[i] && !m_chunks[i]->isLoaded) {
            // 检查块是否在指定区域内
            QVector3D chunkCenter = (m_chunks[i]->boundingBoxMin + m_chunks[i]->boundingBoxMax) * 0.5f;
            float distance = (chunkCenter - center).length();
            
            if (distance <= radius) {
                // 预加载这个块
                getChunkPoints(i); // 这会触发加载
                loadedCount++;
            }
        }
    }
    
    emit statusMessage(QString("Preloaded %1 chunks in region (center: %2, radius: %3) in %4 ms")
                      .arg(loadedCount)
                      .arg(QString("(%1,%2,%3)").arg(center.x()).arg(center.y()).arg(center.z()))
                      .arg(radius)
                      .arg(timer.elapsed()));
    
    return loadedCount > 0;
}

bool PointCloudMemoryManager::unloadChunk(size_t chunkIndex)
{
    if (chunkIndex >= m_chunks.size() || !m_chunks[chunkIndex] || !m_chunks[chunkIndex]->isLoaded) {
        return false;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    auto& chunk = m_chunks[chunkIndex];
    
    // 从加载队列中移除
    auto it = std::find(m_loadedChunks.begin(), m_loadedChunks.end(), chunkIndex);
    if (it != m_loadedChunks.end()) {
        m_loadedChunks.erase(it);
    }
    
    // 更新内存使用量
    m_currentMemoryUsage -= chunk->memoryUsage;
    
    // 标记为未加载
    chunk->isLoaded = false;
    
    m_totalUnloadOperations++;
    m_totalUnloadTime += timer.elapsed();
    
    emit chunkUnloaded(chunkIndex);
    emit memoryUsageChanged(m_currentMemoryUsage, m_maxMemoryUsage);
    
    return true;
}

bool PointCloudMemoryManager::optimizeMemoryUsage()
{
    if (!m_autoMemoryManagement) {
        return false;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    size_t initialMemory = m_currentMemoryUsage;
    size_t targetMemory = m_maxMemoryUsage * 0.7; // 目标使用70%
    
    bool success = performMemoryCleanup(targetMemory);
    
    size_t savedMemory = initialMemory - m_currentMemoryUsage;
    
    emit statusMessage(QString("Memory optimization completed in %1 ms, saved %2 MB")
                      .arg(timer.elapsed())
                      .arg(savedMemory / (1024 * 1024)));
    
    return success;
}

size_t PointCloudMemoryManager::getCurrentMemoryUsage() const
{
    return m_currentMemoryUsage;
}

QVariantMap PointCloudMemoryManager::getMemoryStatistics() const
{
    if (!m_statisticsValid) {
        const_cast<PointCloudMemoryManager*>(this)->updateMemoryStatistics();
    }
    
    return m_memoryStatistics;
}

void PointCloudMemoryManager::clearAllData()
{
    m_chunks.clear();
    m_loadedChunks.clear();
    m_currentMemoryUsage = 0;
    m_statisticsValid = false;
    
    emit statusMessage("All data cleared");
    emit memoryUsageChanged(0, m_maxMemoryUsage);
}

void PointCloudMemoryManager::setChunkPriority(size_t chunkIndex, int priority)
{
    if (chunkIndex < m_chunks.size() && m_chunks[chunkIndex]) {
        m_chunks[chunkIndex]->priority = priority;
    }
}

void PointCloudMemoryManager::setAutoMemoryManagementEnabled(bool enabled)
{
    m_autoMemoryManagement = enabled;
    emit statusMessage(QString("Auto memory management %1").arg(enabled ? "enabled" : "disabled"));
}

bool PointCloudMemoryManager::isAutoMemoryManagementEnabled() const
{
    return m_autoMemoryManagement;
}

// 私有方法实现
std::unique_ptr<PointCloudChunk> PointCloudMemoryManager::createChunk(const std::vector<QVector3D>& points,
                                                                      size_t startIndex,
                                                                      size_t endIndex)
{
    if (startIndex >= endIndex || endIndex > points.size()) {
        return nullptr;
    }

    auto chunk = std::make_unique<PointCloudChunk>();

    // 复制点数据
    chunk->points.reserve(endIndex - startIndex);
    for (size_t i = startIndex; i < endIndex; ++i) {
        chunk->points.push_back(points[i]);
    }

    // 计算边界框
    auto boundingBox = computeChunkBoundingBox(chunk->points);
    chunk->boundingBoxMin = boundingBox.first;
    chunk->boundingBoxMax = boundingBox.second;

    // 计算内存使用量
    chunk->memoryUsage = chunk->points.size() * sizeof(QVector3D);

    // 设置默认属性
    chunk->priority = 0;
    chunk->isLoaded = false;
    chunk->lastAccessTime = 0;

    return chunk;
}

std::pair<QVector3D, QVector3D> PointCloudMemoryManager::computeChunkBoundingBox(const std::vector<QVector3D>& points) const
{
    if (points.empty()) {
        return {QVector3D(), QVector3D()};
    }

    QVector3D minPoint = points[0];
    QVector3D maxPoint = points[0];

    for (const auto& point : points) {
        minPoint.setX(qMin(minPoint.x(), point.x()));
        minPoint.setY(qMin(minPoint.y(), point.y()));
        minPoint.setZ(qMin(minPoint.z(), point.z()));

        maxPoint.setX(qMax(maxPoint.x(), point.x()));
        maxPoint.setY(qMax(maxPoint.y(), point.y()));
        maxPoint.setZ(qMax(maxPoint.z(), point.z()));
    }

    return {minPoint, maxPoint};
}

bool PointCloudMemoryManager::isChunkVisible(const PointCloudChunk& chunk,
                                            const QVector3D& viewPosition,
                                            const QVector3D& viewDirection,
                                            float fov,
                                            float nearPlane,
                                            float farPlane) const
{
    // 简化的视锥体剔除算法
    QVector3D chunkCenter = (chunk.boundingBoxMin + chunk.boundingBoxMax) * 0.5f;
    QVector3D toChunk = chunkCenter - viewPosition;

    // 距离检查
    float distance = toChunk.length();
    if (distance < nearPlane || distance > farPlane) {
        return false;
    }

    // 视野角度检查
    QVector3D normalizedToChunk = toChunk.normalized();
    QVector3D normalizedViewDir = viewDirection.normalized();

    float dotProduct = QVector3D::dotProduct(normalizedToChunk, normalizedViewDir);
    float angle = qAcos(qBound(-1.0f, dotProduct, 1.0f));

    return angle <= qDegreesToRadians(fov * 0.5f);
}

std::vector<size_t> PointCloudMemoryManager::selectChunksToUnload(size_t requiredMemory)
{
    std::vector<size_t> chunksToUnload;
    size_t freedMemory = 0;

    // 创建候选列表
    std::vector<std::pair<size_t, qint64>> candidates; // (index, score)

    for (size_t i : m_loadedChunks) {
        if (i < m_chunks.size() && m_chunks[i] && m_chunks[i]->isLoaded) {
            qint64 score = 0;

            switch (m_strategy) {
                case MemoryStrategy::LRU:
                    score = -m_chunks[i]->lastAccessTime; // 越久未访问，分数越高
                    break;

                case MemoryStrategy::LFU:
                    // 简化实现，使用访问时间作为频率指标
                    score = -m_chunks[i]->lastAccessTime;
                    break;

                case MemoryStrategy::FIFO:
                    score = i; // 索引越小，越早创建
                    break;

                case MemoryStrategy::Priority:
                    score = -m_chunks[i]->priority; // 优先级越低，越容易被卸载
                    break;
            }

            candidates.emplace_back(i, score);
        }
    }

    // 按分数排序
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // 选择要卸载的块
    for (const auto& candidate : candidates) {
        size_t chunkIndex = candidate.first;
        if (chunkIndex < m_chunks.size() && m_chunks[chunkIndex]) {
            chunksToUnload.push_back(chunkIndex);
            freedMemory += m_chunks[chunkIndex]->memoryUsage;

            if (freedMemory >= requiredMemory) {
                break;
            }
        }
    }

    return chunksToUnload;
}

void PointCloudMemoryManager::updateChunkAccessTime(size_t chunkIndex)
{
    if (chunkIndex < m_chunks.size() && m_chunks[chunkIndex]) {
        m_chunks[chunkIndex]->lastAccessTime = QDateTime::currentMSecsSinceEpoch();
    }
}

bool PointCloudMemoryManager::isMemoryLimitExceeded() const
{
    return m_currentMemoryUsage > m_maxMemoryUsage;
}

bool PointCloudMemoryManager::performMemoryCleanup(size_t targetMemory)
{
    if (m_currentMemoryUsage <= targetMemory) {
        return true; // 已经在目标范围内
    }

    size_t requiredMemory = m_currentMemoryUsage - targetMemory;
    auto chunksToUnload = selectChunksToUnload(requiredMemory);

    size_t unloadedCount = 0;
    for (size_t chunkIndex : chunksToUnload) {
        if (unloadChunk(chunkIndex)) {
            unloadedCount++;
        }
    }

    emit statusMessage(QString("Memory cleanup: unloaded %1 chunks, freed %2 MB")
                      .arg(unloadedCount)
                      .arg(requiredMemory / (1024 * 1024)));

    return m_currentMemoryUsage <= targetMemory;
}

void PointCloudMemoryManager::updateMemoryStatistics()
{
    m_memoryStatistics.clear();

    m_memoryStatistics["strategy"] = static_cast<int>(m_strategy);
    m_memoryStatistics["max_memory_mb"] = static_cast<qulonglong>(m_maxMemoryUsage / (1024 * 1024));
    m_memoryStatistics["current_memory_mb"] = static_cast<qulonglong>(m_currentMemoryUsage / (1024 * 1024));
    m_memoryStatistics["memory_usage_percent"] = static_cast<double>(m_currentMemoryUsage) / m_maxMemoryUsage * 100.0;

    m_memoryStatistics["total_chunks"] = static_cast<qulonglong>(m_chunks.size());
    m_memoryStatistics["loaded_chunks"] = static_cast<qulonglong>(m_loadedChunks.size());
    m_memoryStatistics["auto_management"] = m_autoMemoryManagement;

    m_memoryStatistics["total_load_operations"] = static_cast<qulonglong>(m_totalLoadOperations);
    m_memoryStatistics["total_unload_operations"] = static_cast<qulonglong>(m_totalUnloadOperations);
    m_memoryStatistics["avg_load_time_ms"] = m_totalLoadOperations > 0 ?
        static_cast<double>(m_totalLoadTime) / m_totalLoadOperations : 0.0;
    m_memoryStatistics["avg_unload_time_ms"] = m_totalUnloadOperations > 0 ?
        static_cast<double>(m_totalUnloadTime) / m_totalUnloadOperations : 0.0;

    m_statisticsValid = true;
}

// 辅助方法实现
void PointCloudMemoryManager::emitMemoryWarning(const QString& message)
{
    emit memoryWarning(message);
}

void PointCloudMemoryManager::emitChunkLoaded(size_t chunkIndex)
{
    emit chunkLoaded(chunkIndex);
}

void PointCloudMemoryManager::emitMemoryUsageChanged(size_t currentUsage, size_t maxUsage)
{
    emit memoryUsageChanged(currentUsage, maxUsage);
}

} // namespace WallExtraction
