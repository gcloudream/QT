#include "point_cloud_lod_manager.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>
#include <unordered_map>

namespace WallExtraction {

PointCloudLODManager::PointCloudLODManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_strategy(LODStrategy::VoxelGridDownsampling)
    , m_levelCount(4)
    , m_adaptiveLODEnabled(true)
    , m_originalPointCount(0)
    , m_currentLODLevel(-1)
    , m_totalMemoryUsage(0)
    , m_lastGenerationTime(0)
{
    // 注册元类型
    qRegisterMetaType<LODStrategy>("LODStrategy");
    qRegisterMetaType<LODLevel>("LODLevel");
    
    // 设置默认距离阈值
    m_distanceThresholds = {10.0f, 50.0f, 200.0f, 1000.0f};
    
    m_initialized = true;
    qDebug() << "PointCloudLODManager created with" << m_levelCount << "LOD levels";
}

PointCloudLODManager::~PointCloudLODManager()
{
    clearLODData();
    qDebug() << "PointCloudLODManager destroyed";
}

bool PointCloudLODManager::isInitialized() const
{
    return m_initialized;
}

void PointCloudLODManager::setLODStrategy(LODStrategy strategy)
{
    if (m_strategy != strategy) {
        m_strategy = strategy;
        emit statusMessage(QString("LOD strategy changed to %1").arg(static_cast<int>(strategy)));
        
        // 如果已有LOD数据，需要重新生成
        if (!m_lodLevels.empty()) {
            emit statusMessage("LOD data will be regenerated with new strategy");
        }
    }
}

LODStrategy PointCloudLODManager::getLODStrategy() const
{
    return m_strategy;
}

void PointCloudLODManager::setLODLevelCount(int levelCount)
{
    if (levelCount > 0 && levelCount != m_levelCount) {
        m_levelCount = levelCount;
        
        // 重新计算距离阈值
        if (m_levelCount != m_distanceThresholds.size()) {
            m_distanceThresholds.clear();
            for (int i = 0; i < m_levelCount; ++i) {
                m_distanceThresholds.push_back(10.0f * qPow(5.0f, i));
            }
        }
        
        emit statusMessage(QString("LOD level count changed to %1").arg(levelCount));
    }
}

int PointCloudLODManager::getLODLevelCount() const
{
    return m_lodLevels.size();
}

bool PointCloudLODManager::generateLODLevels(const std::vector<QVector3D>& originalPoints)
{
    if (originalPoints.empty()) {
        emit errorOccurred("Cannot generate LOD levels from empty point cloud");
        return false;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    emit statusMessage(QString("Generating %1 LOD levels for %2 points...")
                      .arg(m_levelCount).arg(originalPoints.size()));
    
    // 清除现有LOD数据
    clearLODData();
    
    m_originalPointCount = originalPoints.size();
    m_lodLevels.reserve(m_levelCount);
    
    // 计算边界框用于自适应阈值
    auto boundingBox = computeBoundingBox(originalPoints);
    if (m_adaptiveLODEnabled) {
        m_distanceThresholds = computeDefaultDistanceThresholds(boundingBox);
    }
    
    try {
        // 生成各个LOD级别
        for (int level = 0; level < m_levelCount; ++level) {
            LODLevel lodLevel;
            lodLevel.level = level;
            
            // 计算缩减比例
            float reductionRatio = qPow(0.5f, level); // 每级减少50%
            
            // 根据策略生成LOD点云
            switch (m_strategy) {
                case LODStrategy::UniformDownsampling:
                    lodLevel.points = generateUniformDownsampling(originalPoints, reductionRatio);
                    break;
                    
                case LODStrategy::VoxelGridDownsampling: {
                    float voxelSize = 0.1f * qPow(2.0f, level); // 体素大小递增
                    lodLevel.points = generateVoxelGridDownsampling(originalPoints, voxelSize);
                    break;
                }
                
                case LODStrategy::RandomSampling:
                    lodLevel.points = generateRandomSampling(originalPoints, reductionRatio);
                    break;
                    
                case LODStrategy::ImportanceBasedSampling:
                    lodLevel.points = generateImportanceBasedSampling(originalPoints, reductionRatio);
                    break;
            }
            
            // 设置LOD级别属性
            lodLevel.reductionRatio = static_cast<float>(lodLevel.points.size()) / originalPoints.size();
            lodLevel.minDistance = (level > 0) ? m_distanceThresholds[level - 1] : 0.0f;
            lodLevel.maxDistance = (level < m_distanceThresholds.size()) ? 
                                  m_distanceThresholds[level] : std::numeric_limits<float>::max();
            lodLevel.memoryUsage = lodLevel.points.size() * sizeof(QVector3D);
            
            m_lodLevels.push_back(std::move(lodLevel));
            
            // 更新进度
            int progress = static_cast<int>(((level + 1) * 100) / m_levelCount);
            emit lodGenerationProgress(progress);
            
            qDebug() << "LOD level" << level << "generated:" << lodLevel.points.size() 
                     << "points (ratio:" << lodLevel.reductionRatio << ")";
        }
        
        // 计算总内存使用
        m_totalMemoryUsage = 0;
        for (const auto& level : m_lodLevels) {
            m_totalMemoryUsage += level.memoryUsage;
        }
        
        m_lastGenerationTime = timer.elapsed();
        
        emit statusMessage(QString("LOD generation completed in %1 ms, total memory: %2 MB")
                          .arg(m_lastGenerationTime)
                          .arg(m_totalMemoryUsage / (1024 * 1024)));
        
        return validateLODData();
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("LOD generation failed: %1").arg(e.what()));
        clearLODData();
        return false;
    }
}

int PointCloudLODManager::selectLODLevel(float distance) const
{
    if (m_lodLevels.empty()) {
        return -1;
    }
    
    // 根据距离选择合适的LOD级别
    for (int level = 0; level < m_lodLevels.size(); ++level) {
        if (distance <= m_distanceThresholds[level]) {
            return level;
        }
    }
    
    // 如果距离超过所有阈值，返回最低质量级别
    return m_lodLevels.size() - 1;
}

int PointCloudLODManager::selectLODLevel(const QVector3D& viewPosition, 
                                        const QVector3D& viewDirection, 
                                        float fov) const
{
    if (m_lodLevels.empty()) {
        return -1;
    }
    
    // 简化实现：计算视点到点云中心的距离
    auto boundingBox = computeBoundingBox(m_lodLevels[0].points);
    QVector3D center = (boundingBox.first + boundingBox.second) * 0.5f;
    float distance = (viewPosition - center).length();
    
    // 根据视野角度调整距离
    float adjustedDistance = distance / qTan(qDegreesToRadians(fov * 0.5f));
    
    return selectLODLevel(adjustedDistance);
}

std::vector<QVector3D> PointCloudLODManager::getLODPoints(int level) const
{
    if (level < 0 || level >= m_lodLevels.size()) {
        qWarning() << "Invalid LOD level:" << level;
        return {};
    }
    
    return m_lodLevels[level].points;
}

LODLevel PointCloudLODManager::getLODLevelInfo(int level) const
{
    if (level < 0 || level >= m_lodLevels.size()) {
        qWarning() << "Invalid LOD level:" << level;
        return LODLevel();
    }
    
    return m_lodLevels[level];
}

std::vector<LODLevel> PointCloudLODManager::getAllLODLevels() const
{
    return m_lodLevels;
}

void PointCloudLODManager::clearLODData()
{
    m_lodLevels.clear();
    m_originalPointCount = 0;
    m_totalMemoryUsage = 0;
    m_currentLODLevel = -1;
    
    emit statusMessage("LOD data cleared");
}

size_t PointCloudLODManager::getTotalMemoryUsage() const
{
    return m_totalMemoryUsage;
}

size_t PointCloudLODManager::getOriginalPointCount() const
{
    return m_originalPointCount;
}

void PointCloudLODManager::setDistanceThresholds(const std::vector<float>& thresholds)
{
    m_distanceThresholds = thresholds;
    
    // 更新现有LOD级别的距离范围
    for (size_t i = 0; i < m_lodLevels.size() && i < thresholds.size(); ++i) {
        m_lodLevels[i].minDistance = (i > 0) ? thresholds[i - 1] : 0.0f;
        m_lodLevels[i].maxDistance = thresholds[i];
    }
    
    emit statusMessage("Distance thresholds updated");
}

std::vector<float> PointCloudLODManager::getDistanceThresholds() const
{
    return m_distanceThresholds;
}

void PointCloudLODManager::setAdaptiveLODEnabled(bool enabled)
{
    m_adaptiveLODEnabled = enabled;
    emit statusMessage(QString("Adaptive LOD %1").arg(enabled ? "enabled" : "disabled"));
}

bool PointCloudLODManager::isAdaptiveLODEnabled() const
{
    return m_adaptiveLODEnabled;
}

// 私有方法实现
std::vector<QVector3D> PointCloudLODManager::generateUniformDownsampling(const std::vector<QVector3D>& points,
                                                                         float reductionRatio) const
{
    if (points.empty() || reductionRatio <= 0.0f || reductionRatio > 1.0f) {
        return {};
    }

    size_t targetCount = static_cast<size_t>(points.size() * reductionRatio);
    if (targetCount == 0) {
        targetCount = 1;
    }

    std::vector<QVector3D> downsampledPoints;
    downsampledPoints.reserve(targetCount);

    // 均匀采样
    float step = static_cast<float>(points.size()) / targetCount;
    for (size_t i = 0; i < targetCount; ++i) {
        size_t index = static_cast<size_t>(i * step);
        if (index < points.size()) {
            downsampledPoints.push_back(points[index]);
        }
    }

    return downsampledPoints;
}

std::vector<QVector3D> PointCloudLODManager::generateVoxelGridDownsampling(const std::vector<QVector3D>& points,
                                                                           float voxelSize) const
{
    if (points.empty() || voxelSize <= 0.0f) {
        return {};
    }

    // 使用体素网格进行下采样
    std::unordered_map<std::string, std::vector<QVector3D>> voxelMap;

    for (const auto& point : points) {
        // 计算体素索引
        int vx = static_cast<int>(std::floor(point.x() / voxelSize));
        int vy = static_cast<int>(std::floor(point.y() / voxelSize));
        int vz = static_cast<int>(std::floor(point.z() / voxelSize));

        // 创建体素键
        std::string voxelKey = std::to_string(vx) + "_" + std::to_string(vy) + "_" + std::to_string(vz);

        voxelMap[voxelKey].push_back(point);
    }

    // 计算每个体素的中心点
    std::vector<QVector3D> downsampledPoints;
    downsampledPoints.reserve(voxelMap.size());

    for (const auto& voxel : voxelMap) {
        QVector3D centroid(0, 0, 0);
        for (const auto& point : voxel.second) {
            centroid += point;
        }
        centroid /= voxel.second.size();
        downsampledPoints.push_back(centroid);
    }

    return downsampledPoints;
}

std::vector<QVector3D> PointCloudLODManager::generateRandomSampling(const std::vector<QVector3D>& points,
                                                                    float reductionRatio) const
{
    if (points.empty() || reductionRatio <= 0.0f || reductionRatio > 1.0f) {
        return {};
    }

    size_t targetCount = static_cast<size_t>(points.size() * reductionRatio);
    if (targetCount == 0) {
        targetCount = 1;
    }

    // 创建索引数组
    std::vector<size_t> indices(points.size());
    std::iota(indices.begin(), indices.end(), 0);

    // 随机打乱
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);

    // 选择前targetCount个点
    std::vector<QVector3D> sampledPoints;
    sampledPoints.reserve(targetCount);

    for (size_t i = 0; i < targetCount && i < indices.size(); ++i) {
        sampledPoints.push_back(points[indices[i]]);
    }

    return sampledPoints;
}

std::vector<QVector3D> PointCloudLODManager::generateImportanceBasedSampling(const std::vector<QVector3D>& points,
                                                                             float reductionRatio) const
{
    if (points.empty() || reductionRatio <= 0.0f || reductionRatio > 1.0f) {
        return {};
    }

    size_t targetCount = static_cast<size_t>(points.size() * reductionRatio);
    if (targetCount == 0) {
        targetCount = 1;
    }

    // 计算每个点的重要性分数
    std::vector<std::pair<float, size_t>> importanceScores;
    importanceScores.reserve(points.size());

    for (size_t i = 0; i < points.size(); ++i) {
        float score = calculateImportanceScore(points, i);
        importanceScores.emplace_back(score, i);
    }

    // 按重要性分数排序
    std::sort(importanceScores.begin(), importanceScores.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // 选择重要性最高的点
    std::vector<QVector3D> sampledPoints;
    sampledPoints.reserve(targetCount);

    for (size_t i = 0; i < targetCount && i < importanceScores.size(); ++i) {
        size_t pointIndex = importanceScores[i].second;
        sampledPoints.push_back(points[pointIndex]);
    }

    return sampledPoints;
}

float PointCloudLODManager::calculateImportanceScore(const std::vector<QVector3D>& points, size_t index) const
{
    if (index >= points.size()) {
        return 0.0f;
    }

    const QVector3D& point = points[index];
    float score = 0.0f;

    // 基于局部密度的重要性评分
    float radius = 1.0f;
    int neighborCount = 0;

    for (size_t i = 0; i < points.size(); ++i) {
        if (i != index) {
            float distance = (points[i] - point).length();
            if (distance <= radius) {
                neighborCount++;
            }
        }
    }

    // 密度越高，重要性越高
    score += neighborCount * 0.1f;

    // 基于高度的重要性（假设建筑物的高点更重要）
    score += point.z() * 0.01f;

    return score;
}

std::pair<QVector3D, QVector3D> PointCloudLODManager::computeBoundingBox(const std::vector<QVector3D>& points) const
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

std::vector<float> PointCloudLODManager::computeDefaultDistanceThresholds(const std::pair<QVector3D, QVector3D>& boundingBox) const
{
    QVector3D size = boundingBox.second - boundingBox.first;
    float maxDimension = qMax(qMax(size.x(), size.y()), size.z());

    std::vector<float> thresholds;
    thresholds.reserve(m_levelCount);

    for (int i = 0; i < m_levelCount; ++i) {
        float threshold = maxDimension * qPow(2.0f, i);
        thresholds.push_back(threshold);
    }

    return thresholds;
}

bool PointCloudLODManager::validateLODData() const
{
    if (m_lodLevels.empty()) {
        return false;
    }

    // 检查LOD级别的完整性
    for (size_t i = 0; i < m_lodLevels.size(); ++i) {
        const auto& level = m_lodLevels[i];

        if (!level.isValid()) {
            qWarning() << "Invalid LOD level" << i;
            return false;
        }

        if (level.level != static_cast<int>(i)) {
            qWarning() << "LOD level index mismatch at" << i;
            return false;
        }

        // 检查点数量递减
        if (i > 0 && level.points.size() > m_lodLevels[i-1].points.size()) {
            qWarning() << "LOD level" << i << "has more points than level" << (i-1);
            return false;
        }
    }

    return true;
}

} // namespace WallExtraction
