#ifndef WALL_EXTRACTION_MANAGER_H
#define WALL_EXTRACTION_MANAGER_H

#include <QObject>
#include <QWidget>
#include <QVector3D>
#include <memory>
#include <stdexcept>
#include <vector>

// 前向声明
class LineDrawingTool;
class WallFittingAlgorithm;
class WireframeGenerator;

// 包含必要的结构定义
#include "wall_fitting_algorithm.h"

namespace WallExtraction {

// 交互模式枚举
enum class InteractionMode {
    PointCloudView,      // 原有点云查看模式
    LineDrawing,         // 线段绘制模式
    WallExtraction,      // 墙面提取模式
    WireframeView        // 线框图查看模式
};

// 自定义异常类
class WallExtractionException : public std::exception {
public:
    explicit WallExtractionException(const QString& message);
    const char* what() const noexcept override;
    QString getDetailedMessage() const;

private:
    QString m_message;
    QString m_detailedMessage;
};

// 前向声明内部组件
class LineDrawingTool;
class WallFittingAlgorithm;
class WireframeGenerator;

/**
 * @brief 墙面提取模块的主管理器类
 * 
 * 负责协调各个子模块的工作流程，管理交互模式切换，
 * 处理与主系统的集成通信。
 */
class WallExtractionManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口对象
     */
    explicit WallExtractionManager(QWidget* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~WallExtractionManager();

    /**
     * @brief 初始化管理器和所有子组件
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 检查管理器是否已初始化
     * @return 初始化状态
     */
    bool isInitialized() const;

    /**
     * @brief 激活墙面提取模块
     * @return 激活是否成功
     */
    bool activateModule();

    /**
     * @brief 停用墙面提取模块
     * @return 停用是否成功
     */
    bool deactivateModule();

    /**
     * @brief 检查模块是否处于激活状态
     * @return 激活状态
     */
    bool isActive() const;

    /**
     * @brief 设置交互模式
     * @param mode 新的交互模式
     * @return 设置是否成功
     */
    bool setInteractionMode(InteractionMode mode);

    /**
     * @brief 获取当前交互模式
     * @return 当前交互模式
     */
    InteractionMode getCurrentMode() const;

    /**
     * @brief 设置状态消息
     * @param message 状态消息
     */
    void setStatusMessage(const QString& message);

    /**
     * @brief 获取线段绘制工具
     * @return 线段绘制工具指针
     */
    LineDrawingTool* getLineDrawingTool() const;

    /**
     * @brief 获取墙面拟合算法
     * @return 墙面拟合算法指针
     */
    WallFittingAlgorithm* getWallFittingAlgorithm() const;

    /**
     * @brief 获取线框图生成器
     * @return 线框图生成器指针
     */
    WireframeGenerator* getWireframeGenerator() const;

    /**
     * @brief 处理无效操作（用于测试异常处理）
     * @throws WallExtractionException
     */
    void processInvalidOperation();

    /**
     * @brief 基于用户绘制线段进行墙面拟合
     * @param pointCloud 点云数据
     * @return 拟合是否成功
     */
    bool performLineBasedWallFitting(const std::vector<QVector3D>& pointCloud);

    /**
     * @brief 自动墙面拟合（仅基于点云）
     * @param pointCloud 点云数据
     * @return 拟合是否成功
     */
    bool performAutoWallFitting(const std::vector<QVector3D>& pointCloud);

    /**
     * @brief 获取最近的墙面拟合结果
     * @return 墙面拟合结果
     */
    WallFittingResult getLastWallFittingResult() const;

    /**
     * @brief 清除所有线段和墙面数据
     */
    void clearAllData();

    /**
     * @brief 导出墙面数据
     * @param filename 文件名
     * @return 导出是否成功
     */
    bool exportWallData(const QString& filename) const;

    /**
     * @brief 导入墙面数据
     * @param filename 文件名
     * @return 导入是否成功
     */
    bool importWallData(const QString& filename);

signals:
    /**
     * @brief 交互模式改变信号
     * @param mode 新的交互模式
     */
    void interactionModeChanged(InteractionMode mode);

    /**
     * @brief 状态消息改变信号
     * @param message 状态消息
     */
    void statusMessageChanged(const QString& message);

    /**
     * @brief 模块激活状态改变信号
     * @param active 激活状态
     */
    void moduleActivationChanged(bool active);

    /**
     * @brief 错误发生信号
     * @param error 错误消息
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 墙面拟合开始信号
     */
    void wallFittingStarted();

    /**
     * @brief 墙面拟合完成信号
     * @param result 拟合结果
     */
    void wallFittingCompleted(const WallFittingResult& result);

    /**
     * @brief 墙面拟合失败信号
     * @param error 错误消息
     */
    void wallFittingFailed(const QString& error);

    /**
     * @brief 墙面拟合进度信号
     * @param percentage 进度百分比
     * @param status 状态描述
     */
    void wallFittingProgress(int percentage, const QString& status);

private slots:
    /**
     * @brief 处理子组件错误
     * @param error 错误消息
     */
    void handleComponentError(const QString& error);

private:
    /**
     * @brief 初始化子组件
     * @return 初始化是否成功
     */
    bool initializeComponents();

    /**
     * @brief 设置信号槽连接
     */
    void setupConnections();

    /**
     * @brief 清理资源
     */
    void cleanup();

    /**
     * @brief 验证状态
     * @return 状态是否有效
     */
    bool validateState() const;

private:
    // 状态变量
    bool m_initialized;
    bool m_active;
    InteractionMode m_currentMode;
    QString m_statusMessage;

    // 父窗口
    QWidget* m_parentWidget;

    // 子组件（使用智能指针管理）
    std::unique_ptr<LineDrawingTool> m_lineDrawingTool;
    std::unique_ptr<WallFittingAlgorithm> m_wallFittingAlgorithm;
    std::unique_ptr<WireframeGenerator> m_wireframeGenerator;

    // 数据缓存
    std::vector<QVector3D> m_currentPointCloud;
    WallFittingResult m_lastWallFittingResult;

    // 处理状态
    bool m_isProcessing;
};

} // namespace WallExtraction

// 注册元类型以支持信号槽
Q_DECLARE_METATYPE(WallExtraction::InteractionMode)

#endif // WALL_EXTRACTION_MANAGER_H
