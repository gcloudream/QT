#ifndef WALL_FITTING_ALGORITHM_H
#define WALL_FITTING_ALGORITHM_H

#include <QObject>
#include <QVector3D>
#include <QVector2D>
#include <QDateTime>
#include <vector>
#include <memory>
#include <functional>

namespace WallExtraction {

// 前向声明
struct LineSegment;

// 平面数据结构
struct Plane3D {
    QVector3D point;        // 平面上的一点
    QVector3D normal;       // 平面法向量
    float distance;         // 到原点的距离
    std::vector<int> inlierIndices;  // 内点索引
    float confidence;       // 置信度

    Plane3D() : distance(0.0f), confidence(0.0f) {}

    // 计算点到平面的距离
    float distanceToPoint(const QVector3D& point) const;

    // 检查点是否在平面上（在容差范围内）
    bool containsPoint(const QVector3D& point, float tolerance = 0.05f) const;
};

// 墙面段数据结构
struct WallSegment {
    int id;                             // 唯一标识符
    QVector3D startPoint;               // 起点
    QVector3D endPoint;                 // 终点
    QVector3D normal;                   // 墙面法向量
    float thickness;                    // 墙面厚度
    float height;                       // 墙面高度
    std::vector<QVector3D> supportingPoints;  // 支撑点云
    std::vector<int> sourceLineIds;     // 来源线段ID
    float confidence;                   // 置信度
    QDateTime createdTime;              // 创建时间

    WallSegment() : id(-1), thickness(0.0f), height(0.0f), confidence(0.0f) {
        createdTime = QDateTime::currentDateTime();
    }

    // 计算墙面长度
    float length() const;

    // 计算墙面面积
    float area() const;
};

// RANSAC算法参数
struct RANSACParameters {
    float probability;          // 成功概率 (0.99)
    int minPoints;             // 最小点数 (100)
    float epsilon;             // 距离阈值 (0.05m)
    float clusterEpsilon;      // 聚类阈值 (0.1m)
    float normalThreshold;     // 法向量阈值 (0.9)
    float cosAngle;           // 角度余弦值 (0.1 for ~84度)
    int maxIterations;        // 最大迭代次数 (1000)
    float minWallLength;      // 最小墙面长度 (1.0m)
    float maxWallThickness;   // 最大墙面厚度 (0.5m)

    RANSACParameters()
        : probability(0.99f)
        , minPoints(100)
        , epsilon(0.05f)
        , clusterEpsilon(0.1f)
        , normalThreshold(0.9f)
        , cosAngle(0.1f)
        , maxIterations(1000)
        , minWallLength(1.0f)
        , maxWallThickness(0.5f)
    {}
};

// 墙面拟合结果
struct WallFittingResult {
    std::vector<WallSegment> walls;     // 提取的墙面
    std::vector<Plane3D> planes;        // 检测到的平面
    int totalPoints;                    // 总点数
    int processedPoints;                // 处理的点数
    int unassignedPoints;               // 未分配的点数
    float processingTime;               // 处理时间（秒）
    bool success;                       // 是否成功
    QString errorMessage;               // 错误信息

    WallFittingResult()
        : totalPoints(0)
        , processedPoints(0)
        , unassignedPoints(0)
        , processingTime(0.0f)
        , success(false)
    {}
};

/**
 * @brief 基于RANSAC的墙面拟合算法类
 *
 * 实现基于RANSAC算法的墙面检测和拟合功能，支持：
 * - 从点云中自动检测垂直平面
 * - 基于用户绘制线段的半自动墙面提取
 * - 平面合并和优化
 * - 墙面几何参数计算
 */
class WallFittingAlgorithm : public QObject
{
    Q_OBJECT

public:
    explicit WallFittingAlgorithm(QObject* parent = nullptr);
    ~WallFittingAlgorithm();

    // 初始化和状态
    bool initialize();
    bool isInitialized() const;
    void reset();

    // 参数设置
    void setRANSACParameters(const RANSACParameters& params);
    RANSACParameters getRANSACParameters() const;

    // 进度回调设置
    void setProgressCallback(std::function<void(int, const QString&)> callback);

    // 主要算法接口
    WallFittingResult fitWallsFromPointCloud(const std::vector<QVector3D>& points);
    WallFittingResult fitWallsFromLines(const std::vector<QVector3D>& points,
                                       const std::vector<LineSegment>& userLines);

    // 平面检测
    std::vector<Plane3D> detectPlanes(const std::vector<QVector3D>& points);

    // 墙面提取
    std::vector<WallSegment> extractWallsFromPlanes(const std::vector<Plane3D>& planes,
                                                    const std::vector<QVector3D>& points);

    // 基于用户线段的墙面拟合
    std::vector<WallSegment> fitWallsAlongLines(const std::vector<QVector3D>& points,
                                               const std::vector<LineSegment>& userLines);

    // 几何优化
    void optimizeWallGeometry(std::vector<WallSegment>& walls);
    void mergeParallelWalls(std::vector<WallSegment>& walls, float angleThreshold = 5.0f);
    void regularizeWallIntersections(std::vector<WallSegment>& walls);

    // 工具方法
    static bool isVerticalPlane(const Plane3D& plane, float angleThreshold = 10.0f);
    static float calculatePlaneDistance(const Plane3D& plane1, const Plane3D& plane2);
    static bool arePlanesParallel(const Plane3D& plane1, const Plane3D& plane2,
                                 float angleThreshold = 5.0f);

signals:
    // 进度信号
    void progressChanged(int percentage, const QString& status);

    // 结果信号
    void wallsDetected(const std::vector<WallSegment>& walls);
    void planesDetected(const std::vector<Plane3D>& planes);

    // 状态信号
    void processingStarted();
    void processingCompleted(const WallFittingResult& result);
    void processingFailed(const QString& error);

    // 错误信号
    void errorOccurred(const QString& error);
    void warningOccurred(const QString& warning);

private slots:
    void onInternalProgress(int percentage, const QString& status);

private:
    // 初始化方法
    void initializeParameters();
    void setupCallbacks();

    // RANSAC核心算法
    Plane3D fitPlaneRANSAC(const std::vector<QVector3D>& points,
                           const std::vector<int>& indices);
    std::vector<int> findPlaneInliers(const std::vector<QVector3D>& points,
                                     const Plane3D& plane, float threshold);

    // 平面处理
    Plane3D refinePlane(const std::vector<QVector3D>& points,
                       const std::vector<int>& inliers);
    void filterVerticalPlanes(std::vector<Plane3D>& planes);
    void clusterPlanes(std::vector<Plane3D>& planes);

    // 墙面构建
    WallSegment buildWallFromPlane(const Plane3D& plane,
                                  const std::vector<QVector3D>& points);
    void calculateWallBoundaries(WallSegment& wall,
                                const std::vector<QVector3D>& points);
    void estimateWallThickness(WallSegment& wall,
                              const std::vector<QVector3D>& points);

    // 基于线段的拟合
    std::vector<QVector3D> findPointsNearLine(const std::vector<QVector3D>& points,
                                             const LineSegment& line,
                                             float searchRadius);
    Plane3D fitPlaneToLineAndPoints(const LineSegment& line,
                                   const std::vector<QVector3D>& nearbyPoints);

    // 几何计算辅助方法
    QVector3D calculateCentroid(const std::vector<QVector3D>& points);
    QVector3D calculateNormal(const std::vector<QVector3D>& points);
    float calculateVariance(const std::vector<QVector3D>& points, const Plane3D& plane);

    // 数据验证
    bool validatePointCloud(const std::vector<QVector3D>& points);
    bool validateUserLines(const std::vector<LineSegment>& lines);
    bool validateResult(const WallFittingResult& result);

    // 进度报告
    void reportProgress(int percentage, const QString& status);
    void updateProgress(int currentStep, int totalSteps, const QString& operation);

private:
    // 基本状态
    bool m_initialized;

    // 算法参数
    RANSACParameters m_parameters;

    // 回调函数
    std::function<void(int, const QString&)> m_progressCallback;

    // 处理状态
    bool m_isProcessing;
    QDateTime m_processingStartTime;

    // 统计信息
    int m_totalIterations;
    int m_successfulFits;

    // 缓存数据
    std::vector<Plane3D> m_lastDetectedPlanes;
    std::vector<WallSegment> m_lastExtractedWalls;
};

} // namespace WallExtraction

// 注册元类型以支持信号槽
Q_DECLARE_METATYPE(WallExtraction::Plane3D)
Q_DECLARE_METATYPE(WallExtraction::WallSegment)
Q_DECLARE_METATYPE(WallExtraction::WallFittingResult)

#endif // WALL_FITTING_ALGORITHM_H
